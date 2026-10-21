// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 VitarenewFlow contributors
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Security.Cryptography;
using System.Text;
using System.Text.RegularExpressions;
using Drawing = System.Drawing;

namespace LibraryDesktop {
    public sealed class Game {
        public string System { get; set; }
        public string Key { get; set; }
        public string FilePath { get; set; }
        public string CoverPath { get; set; }
        public string Title { get; set; }
        public long Size { get; set; }
        public bool Pending { get; set; }
        public bool Homebrew { get; set; }
        public string Category { get { return Native ? (Homebrew ? "homebrew" : "3ds") : System == "gba" || System == "gb" ? "gameboy" : System; } }
        public double ArtOpacity { get { return Pending ? 0.45 : 1; } }
        public bool Native { get { return System == "3ds"; } }
        public string Platform { get { return Card.Platform(System); } }
        public string Caption { get { return Platform + (Pending ? " · Installation ausstehend" : Native ? " · installiert" : " · " + Card.Bytes(Size)); } }
        public System.Windows.Media.ImageSource Cover { get; set; }
        public string CoverLabel { get { return File.Exists(CoverPath) ? "Cover vorhanden" : "Cover hinzufügen"; } }
    }
    public sealed class ImportPlan {
        public string Source, Target, System, Note;
        public bool Skip;
        public string Name { get { return Path.GetFileName(Source); } }
        public string Destination { get { return Skip ? Note : (System == "cia" ? "cias/ · auf dem 3DS in der Library installieren" : "roms/" + Card.RomFolder(System) + "/"); } }
    }
    public sealed class CopyProgress { public long Done, Total; public string Name; }

    public sealed class Card {
        public readonly string Root;
        readonly bool sandbox;
        readonly uint serial;
        public static readonly string[] Systems = { "3ds", "ds", "gba", "n64", "snes", "nes", "vb", "gb" };
        public static readonly string[] Views = { "3ds", "homebrew", "ds", "gameboy", "n64", "snes", "nes", "vb" };
        public static bool IsHomebrew(string id) {
            return NativeNames.ContainsKey(id) || new[] { "0004000000DF1000", "0004000003DF1000", "000400000EB00000", "000400000BF74D00", "000400000EC10000", "0004000000DF1100", "0004000000308200", "000400000FF9FF00" }.Contains(id,StringComparer.OrdinalIgnoreCase);
        }
        public static readonly string[] ScanDirectories = { "roms/nds", "roms/gba", "roms/n64", "3ds/DaedalusX64/Roms", "roms/snes", "roms/nes", "roms/vb", "roms/gb" };
        static readonly Dictionary<string,string> NativeNames = new Dictionary<string,string>(StringComparer.OrdinalIgnoreCase) {
            {"00040000001A1E00", "mGBA"}, {"0004000000384900", "Snes9x"}, {"0004000004391700", "Universal-Updater"},
            {"0004000000384A00", "VirtuaNES"}, {"000400000DAED300", "DaedalusX64"},
            {"000400000BAFE000", "Anemone3DS"}, {"000400000BCFFF00", "Checkpoint"}, {"000400000BEEF500", "ftpd"},
            {"000400000F104200", "VitarenewFlow"}, {"000400000D40D200", "Nimbus"}, {"000400000D921E00", "Homebrew Launcher"}, {"000400000DEAD100", "RGB"},
            {"000400000F800100", "FBI"}, {"000400000FE7CB00", "Red Viper"}
        };
        [DllImport("kernel32.dll", CharSet = CharSet.Unicode, SetLastError = true)]
        static extern bool GetVolumeInformation(string root, StringBuilder name, int nameSize, out uint serialNumber, out uint maxName, out uint flags, StringBuilder fileSystem, int fsSize);

        public Card(string root) : this(root, false) { }
        internal Card(string root, bool testSandbox) {
            Root = Path.GetFullPath(root).TrimEnd(Path.DirectorySeparatorChar) + Path.DirectorySeparatorChar;
            sandbox = testSandbox;
            if (!sandbox && !Root.Equals(Path.GetPathRoot(Root), StringComparison.OrdinalIgnoreCase))
                throw new IOException("Die App bitte direkt im Hauptverzeichnis der 3DS-SD-Karte öffnen.");
            serial = VolumeSerial();
            Validate();
        }
        uint VolumeSerial() {
            uint n, max, flags; var fs = new StringBuilder(64);
            if (!GetVolumeInformation(Path.GetPathRoot(Root), null, 0, out n, out max, out flags, fs, fs.Capacity))
                throw new IOException("Das Laufwerk ist nicht verfügbar. SD-Karte erneut einstecken.");
            if (!sandbox && fs.ToString() != "FAT32") throw new IOException("Hier wurde keine FAT32-SD-Karte erkannt.");
            return n;
        }
        public void Validate() {
            if (VolumeSerial() != serial || !Directory.Exists(Path.Combine(Root, "Nintendo 3DS")) || !File.Exists(Path.Combine(Root, "3ds/3ds-library/3ds-library.3dsx")))
                throw new IOException("Die 3DS-Library-SD-Karte fehlt oder wurde ausgetauscht. App schließen und von der richtigen Karte erneut öffnen.");
            RejectLinks(Root);
        }
        void RejectLinks(string path) {
            string current = Path.GetFullPath(path);
            while (current != null && current.StartsWith(Root, StringComparison.OrdinalIgnoreCase)) {
                if ((File.Exists(current) || Directory.Exists(current)) && (File.GetAttributes(current) & FileAttributes.ReparsePoint) != 0)
                    throw new IOException("Verknüpfte Ordner werden aus Sicherheitsgründen nicht beschrieben.");
                var p = Directory.GetParent(current.TrimEnd(Path.DirectorySeparatorChar));
                current = p == null ? null : p.FullName + (p.FullName.EndsWith("\\") ? "" : "\\");
            }
        }
        public string PathOnCard(string relative) {
            if (Path.IsPathRooted(relative) || relative.Split('/', '\\').Any(x => x == "..")) throw new IOException("Ungültiger SD-Pfad.");
            string path = Path.GetFullPath(Path.Combine(Root, relative));
            if (!path.StartsWith(Root, StringComparison.OrdinalIgnoreCase)) throw new IOException("Pfad liegt außerhalb der SD-Karte.");
            RejectLinks(path);
            return path;
        }
        static void CheckKey(string key) {
            if (String.IsNullOrWhiteSpace(key) || key == "." || key == ".." || key.IndexOfAny(Path.GetInvalidFileNameChars()) >= 0 || key.EndsWith(".") || key.EndsWith(" ") || Encoding.UTF8.GetByteCount(key) > 255)
                throw new IOException("Dateiname ist für die 3DS-Library ungeeignet oder zu lang.");
        }
        public static string Platform(string sys) {
            switch (sys) { case "homebrew": return "Homebrew Apps"; case "gameboy": return "Game Boy"; case "3ds": return "Nintendo 3DS"; case "ds": return "Nintendo DS"; case "gba": return "Game Boy Advance"; case "n64": return "Nintendo 64"; case "snes": return "Super Nintendo"; case "nes": return "Nintendo NES"; case "vb": return "Virtual Boy"; case "gb": return "Game Boy / Color"; default: return "Unbekannt"; }
        }
        public static string RomFolder(string sys) { return sys == "ds" ? "nds" : sys; }
        public static string Detect(string file) {
            switch (Path.GetExtension(file).ToLowerInvariant()) {
                case ".nds": return "ds"; case ".gba": return "gba"; case ".z64": case ".v64": case ".n64": return "n64";
                case ".sfc": case ".smc": return "snes"; case ".nes": return "nes"; case ".vb": return "vb"; case ".gb": case ".gbc": return "gb";
                default: return null;
            }
        }
        public static bool IsImage(string file) { return new[] { ".png", ".jpg", ".jpeg" }.Contains(Path.GetExtension(file).ToLowerInvariant()); }
        public static string Bytes(long bytes) { return bytes >= 1073741824 ? (bytes / 1073741824.0).ToString("0.0") + " GB" : bytes >= 1048576 ? (bytes / 1048576.0).ToString("0.#") + " MB" : (bytes / 1024.0).ToString("0.#") + " KB"; }
        public static string CleanTitle(string filename) {
            string name = Path.GetFileNameWithoutExtension(filename);
            // Remove only known release tags. A legitimate parenthesized subtitle stays intact.
            return Regex.Replace(name, @"\s*\((?:Europe|USA|Japan|World|En(?:,[A-Za-z]{2})*|Rev \d+|Wii U Virtual Console|E|U|J)\)", "", RegexOptions.IgnoreCase).Trim();
        }
        public string CoverFor(string system, string key) {
            CheckKey(key);
            if (!Systems.Contains(system)) throw new IOException("Unbekannte Plattform.");
            string path = PathOnCard("3ds/3ds-library/covers/" + system + "/" + key + ".png");
            if (key.Length + 4 > 255 || path.Length > 247) throw new IOException("Dateiname ist für die Cover-Verwaltung zu lang. Vor dem Import einen kürzeren Namen verwenden.");
            return path;
        }
        public List<Game> Scan() {
            Validate(); var games = new List<Game>();
            foreach (string rel in ScanDirectories) {
                string dir = PathOnCard(rel); if (!Directory.Exists(dir)) continue;
                foreach (string f in Directory.GetFiles(dir)) {
                    string sys = Detect(f); if (sys == null || (File.GetAttributes(f) & FileAttributes.ReparsePoint) != 0) continue;
                    string key = Path.GetFileName(f);
                    try { games.Add(new Game { System = sys, Key = key, FilePath = f, CoverPath = CoverFor(sys, key), Title = CleanTitle(key), Size = new FileInfo(f).Length }); }
                    catch (IOException) { /* Unrepresentable filename: never silently rename a ROM/save pair. */ }
                }
            }
            string native = PathOnCard("Nintendo 3DS"); var seen = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
            foreach (string id0 in Directory.GetDirectories(native).Where(x => Regex.IsMatch(Path.GetFileName(x), "^[0-9a-fA-F]{32}$")))
            foreach (string id1 in Directory.GetDirectories(id0).Where(x => Regex.IsMatch(Path.GetFileName(x), "^[0-9a-fA-F]{32}$")))
            foreach (string group in new[] { "00040000", "00040002" }) {
                string folder = Path.Combine(id1, "title", group); if (!Directory.Exists(folder)) continue;
                foreach (string title in Directory.GetDirectories(folder)) {
                    string low = Path.GetFileName(title); if (!Regex.IsMatch(low, "^[0-9a-fA-F]{8}$")) continue;
                    string id = (group + low).ToUpperInvariant(), label;
                    if (!seen.Add(id)) continue;
                    if (!NativeNames.TryGetValue(id, out label)) label = "3DS-Titel · " + id;
                    games.Add(new Game { System = "3ds", Key = id, Title = label, CoverPath = CoverFor("3ds", id), FilePath = null, Homebrew = IsHomebrew(id) });
                }
            }
            string ciaFolder = PathOnCard("cias");
            if (Directory.Exists(ciaFolder)) foreach (string file in Directory.GetFiles(ciaFolder).Where(f => Path.GetExtension(f).Equals(".cia", StringComparison.OrdinalIgnoreCase))) {
                try {
                    string id, name;
                    if (!CiaMetadata.Read(file, out id, out name)) continue;
                    Game installed = games.FirstOrDefault(g => g.Native && g.Key == id);
                    if (installed != null) { if (installed.Title.StartsWith("3DS-Titel · ",StringComparison.Ordinal)) installed.Title = name; continue; }
                    if (!seen.Add(id)) continue;
                    games.Add(new Game { System = "3ds", Key = id, Title = name, CoverPath = CoverFor("3ds", id), FilePath = file, Pending = true, Size = new FileInfo(file).Length, Homebrew = IsHomebrew(id) });
                } catch (IOException) { } catch (UnauthorizedAccessException) { }
            }
            return games.OrderBy(x => Array.IndexOf(Systems, x.System)).ThenBy(x => x.Title, StringComparer.CurrentCultureIgnoreCase).ToList();
        }
        public List<ImportPlan> Plan(IEnumerable<string> files) {
            Validate(); var current = Scan(); var used = new HashSet<string>(StringComparer.OrdinalIgnoreCase); var result = new List<ImportPlan>();
            foreach (string file in files) {
                var plan = new ImportPlan { Source = Path.GetFullPath(file) }; result.Add(plan);
                string name = Path.GetFileName(file), sys = Detect(file);
                if (sys == null && Path.GetExtension(file).Equals(".cia", StringComparison.OrdinalIgnoreCase)) sys = "cia";
                plan.System = sys;
                try {
                    if (!File.Exists(file)) throw new IOException("Keine lesbare Datei (Ordner bitte zuerst öffnen).");
                    if (sys == null) throw new IOException(Path.GetExtension(file).Equals(".3ds", StringComparison.OrdinalIgnoreCase) ? ".3ds wird nicht direkt gestartet. Installierte 3DS-Titel verwenden." : "Format nicht unterstützt. ZIP/7z bitte vorher entpacken.");
                    if (sys == "cia") {
                        string titleId, titleName;
                        if (!CiaMetadata.Read(file, out titleId, out titleName)) throw new IOException("Keine lesbare 3DS-Spiel-CIA. Updates, DLC und Systemtitel werden hier nicht importiert.");
                        if (current.Any(g => g.Native && g.Key == titleId)) throw new IOException("Dieser Titel ist bereits installiert oder zur Installation bereitgestellt.");
                        if (!used.Add("cia:" + titleId)) throw new IOException("Dieser CIA-Titel wurde im Stapel bereits ausgewählt.");
                    }
                    CheckKey(name);
                    if (sys != "cia") CoverFor(sys, name);
                    if (new FileInfo(file).Length == 0) throw new IOException("Datei ist leer.");
                    if (new FileInfo(file).Length >= 4294967296L) throw new IOException("Zu groß für FAT32 (maximal 4 GB pro Datei).");
                    plan.Target = PathOnCard(sys == "cia" ? "cias/" + name : "roms/" + RomFolder(sys) + "/" + name);
                    if (File.Exists(plan.Target) || current.Any(x => x.System == sys && String.Equals(x.Key, name, StringComparison.OrdinalIgnoreCase))) throw new IOException("Bereits auf der Karte · bleibt unverändert.");
                    if (!used.Add(plan.Target)) throw new IOException("Doppelt ausgewählt · wird einmal importiert.");
                } catch (IOException e) { plan.Skip = true; plan.Note = e.Message; }
            }
            return result;
        }
        public static string Hash(string file) { using (var sha = SHA256.Create()) using (var s = File.OpenRead(file)) return BitConverter.ToString(sha.ComputeHash(s)).Replace("-", ""); }
        public string Import(ImportPlan item, Action<CopyProgress> progress, Func<bool> cancel) {
            Validate(); if (item.Skip) throw new IOException("Dieser Eintrag kann nicht importiert werden.");
            // Replan immediately before writing: never trust stale UI paths or overwrite an intervening import.
            ImportPlan fresh = Plan(new[] { item.Source })[0];
            if (fresh.Skip || !String.Equals(fresh.Target, item.Target, StringComparison.OrdinalIgnoreCase)) throw new IOException(fresh.Note ?? "Importziel hat sich geändert.");
            string parent = Path.GetDirectoryName(fresh.Target);
            Directory.CreateDirectory(parent); PathOnCard(parent.Substring(Root.Length));
            string temp = Path.Combine(parent, ".library-" + Guid.NewGuid().ToString("N") + ".partial");
            try {
                long length = new FileInfo(fresh.Source).Length;
                var drive = new DriveInfo(Path.GetPathRoot(Root));
                if (drive.AvailableFreeSpace < length + 1048576) throw new IOException("Nicht genügend freier Speicher auf der SD-Karte.");
                string hash;
                using (var input = new FileStream(fresh.Source, FileMode.Open, FileAccess.Read, FileShare.Read))
                using (var output = new FileStream(temp, FileMode.CreateNew, FileAccess.Write, FileShare.None))
                using (var sha = SHA256.Create()) {
                    byte[] buffer = new byte[1048576]; int n; long done = 0;
                    while ((n = input.Read(buffer, 0, buffer.Length)) > 0) {
                        if (cancel != null && cancel()) throw new OperationCanceledException("Import abgebrochen. Keine neue Spieldatei übernommen.");
                        output.Write(buffer, 0, n); sha.TransformBlock(buffer, 0, n, null, 0); done += n;
                        if (progress != null) progress(new CopyProgress { Done = done, Total = length, Name = fresh.Name });
                    }
                    sha.TransformFinalBlock(new byte[0], 0, 0); hash = BitConverter.ToString(sha.Hash).Replace("-", ""); output.Flush(true);
                }
                if (cancel != null && cancel()) throw new OperationCanceledException("Import abgebrochen.");
                if (Hash(temp) != hash) throw new IOException("Die Kopie konnte nicht verifiziert werden.");
                Validate(); PathOnCard(fresh.Target.Substring(Root.Length));
                File.Move(temp, fresh.Target); // Fails if an existing destination appeared. No overwrite flag.
                return hash;
            } finally { if (File.Exists(temp)) File.Delete(temp); }
        }
        public string AssignCover(Game game, string imageFile, bool onlyMissing = false) {
            Validate();
            if (!Scan().Any(x => x.System == game.System && x.Key == game.Key && x.FilePath == game.FilePath)) throw new IOException("Spiel ist nicht mehr auf dieser Karte vorhanden.");
            if (!IsImage(imageFile) || !File.Exists(imageFile)) throw new IOException("Bitte ein PNG- oder JPG-Bild wählen.");
            if (new FileInfo(imageFile).Length > 32 * 1024 * 1024) throw new IOException("Bitte ein Bild unter 32 MB verwenden.");
            string target = CoverFor(game.System, game.Key);
            if (onlyMissing && File.Exists(target)) throw new CoverExistsException();
            if (Path.GetFullPath(imageFile).Equals(target, StringComparison.OrdinalIgnoreCase)) return "Dieses Cover ist bereits zugeordnet.";
            string token = Guid.NewGuid().ToString("N");
            Directory.CreateDirectory(Path.GetDirectoryName(target));
            string temp = Path.Combine(Path.GetDirectoryName(target), ".library-" + token + ".partial"), rollback = Path.Combine(Path.GetDirectoryName(target), ".library-" + token + ".rollback");
            string backup = null;
            try {
                using (var stream = File.OpenRead(imageFile)) using (var image = Drawing.Image.FromStream(stream, true, true)) {
                    if ((long)image.Width * image.Height > 40000000) throw new IOException("Bitte ein Bild mit höchstens 40 Megapixeln verwenden.");
                    if (image.PropertyIdList.Contains(0x112)) {
                        int orientation = BitConverter.ToUInt16(image.GetPropertyItem(0x112).Value, 0);
                        Drawing.RotateFlipType[] transforms = { Drawing.RotateFlipType.RotateNoneFlipNone, Drawing.RotateFlipType.RotateNoneFlipNone, Drawing.RotateFlipType.RotateNoneFlipX, Drawing.RotateFlipType.Rotate180FlipNone, Drawing.RotateFlipType.Rotate180FlipX, Drawing.RotateFlipType.Rotate90FlipX, Drawing.RotateFlipType.Rotate90FlipNone, Drawing.RotateFlipType.Rotate270FlipX, Drawing.RotateFlipType.Rotate270FlipNone };
                        if (orientation >= 1 && orientation <= 8) image.RotateFlip(transforms[orientation]);
                    }
                    double scale = Math.Min(1.0, 256.0 / Math.Max(image.Width, image.Height));
                    int width = Math.Max(1, (int)Math.Round(image.Width * scale)), height = Math.Max(1, (int)Math.Round(image.Height * scale));
                    using (var bitmap = new Drawing.Bitmap(width, height, Drawing.Imaging.PixelFormat.Format32bppArgb)) {
                        using (var graphics = Drawing.Graphics.FromImage(bitmap)) {
                            graphics.Clear(Drawing.Color.Transparent); graphics.CompositingMode = Drawing.Drawing2D.CompositingMode.SourceCopy;
                            graphics.InterpolationMode = Drawing.Drawing2D.InterpolationMode.HighQualityBicubic;
                            graphics.PixelOffsetMode = Drawing.Drawing2D.PixelOffsetMode.HighQuality;
                            graphics.DrawImage(image, new Drawing.Rectangle(0, 0, width, height));
                        }
                        using (var output = new FileStream(temp, FileMode.CreateNew, FileAccess.Write, FileShare.None)) { CoverCompression.Save(bitmap,output); output.Flush(true); }
                    }
                }
                using (var verify = Drawing.Image.FromFile(temp)) { if (verify.Width > 256 || verify.Height > 256) throw new IOException("Cover-Konvertierung fehlgeschlagen."); }
                string newHash = Hash(temp); Validate(); PathOnCard(target.Substring(Root.Length));
                if (onlyMissing && File.Exists(target)) throw new CoverExistsException();
                if (File.Exists(target)) {
                    // Short token filename keeps backups usable even for a long ROM filename.
                    backup = PathOnCard("3ds/3ds-library/pc-backups/" + DateTime.Now.ToString("yyyyMMdd-HHmmss") + "-" + token.Substring(0, 8) + "/cover.png");
                    Directory.CreateDirectory(Path.GetDirectoryName(backup)); File.Copy(target, backup, false);
                    if (Hash(backup) != Hash(target)) throw new IOException("Cover-Backup konnte nicht verifiziert werden.");
                    File.WriteAllText(Path.Combine(Path.GetDirectoryName(backup), "ZIEL.txt"), "Original: " + target.Substring(Root.Length) + "\r\nSHA256: " + Hash(backup) + "\r\n", Encoding.UTF8);
                    File.Move(target, rollback);
                }
                try { File.Move(temp, target); if (Hash(target) != newHash) throw new IOException("Cover-Prüfung fehlgeschlagen."); }
                catch { if (File.Exists(rollback)) { if (File.Exists(target)) File.Delete(target); File.Move(rollback, target); } throw; }
                if (File.Exists(rollback)) File.Delete(rollback);
                // These are derived loader caches, never the customer's original image.
                foreach(string suffix in new[]{".thumb.png",".thumb.size"}){try{if(File.Exists(target+suffix))File.Delete(target+suffix);}catch(IOException){/* source timestamp/size checks still invalidate stale cache */}}
                return backup == null ? "Cover gespeichert · auf dem 3DS neu einlesen." : "Cover gespeichert · vorheriges Cover gesichert.";
            } finally { if (File.Exists(temp)) File.Delete(temp); }
        }
        public List<string> SaveCandidates(Game game) {
            var paths = new List<string>(); if (game.Native) return paths;
            string stem = Path.GetFileNameWithoutExtension(game.Key), dir = Path.GetDirectoryName(game.FilePath);
            // Only conventional locations used by this installation. This is discovery, not save compatibility/sync.
            foreach (string ext in new[] { ".sav", ".srm", ".ram", ".mpk", ".eep", ".fla", ".sra" }) {
                paths.Add(Path.Combine(dir, stem + ext)); paths.Add(Path.Combine(dir, "saves", stem + ext));
                if (game.System == "n64") paths.Add(PathOnCard("3ds/DaedalusX64/SaveGames/" + stem + ext));
                if (game.System == "snes") paths.Add(PathOnCard("3ds/snes9x_3ds/saves/" + stem + ext));
            }
            return paths.Where(File.Exists).Distinct(StringComparer.OrdinalIgnoreCase).ToList();
        }
    }
}



