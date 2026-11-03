using System.Diagnostics;
using System.Xml.Linq;
namespace LibraryDesktop;
internal sealed record VolumeSnapshot(string Mount, string Identity, string Format, bool Writable, string Device = "");
internal static class PortableVolume {
    static string Clean(string path) => Path.GetFullPath(path).TrimEnd(Path.DirectorySeparatorChar) + Path.DirectorySeparatorChar;
    public static void RequireMountRoot(string root) { Snapshot(root); }
    public static string Identity(string root, bool sandbox) => sandbox ? "sandbox:" + Clean(root) : Snapshot(root).Identity;
    public static long FreeBytes(string root) => new DriveInfo(root).AvailableFreeSpace;
    public static VolumeSnapshot Snapshot(string root) {
        root = Clean(root);
        VolumeSnapshot volume;
        if (OperatingSystem.IsMacOS()) {
            var start = new ProcessStartInfo("/usr/sbin/diskutil") { RedirectStandardOutput = true, RedirectStandardError = true, UseShellExecute = false };
            start.ArgumentList.Add("info"); start.ArgumentList.Add("-plist"); start.ArgumentList.Add(root);
            using var process = Process.Start(start) ?? throw new IOException("Datenträger konnte nicht geprüft werden.");
            var read = process.StandardOutput.ReadToEndAsync();
            if (!process.WaitForExit(5000)) { process.Kill(); throw new IOException("Datenträgerprüfung hat zu lange gedauert."); }
            if (process.ExitCode != 0) throw new IOException("SD-Karte ist nicht verfügbar.");
            volume = ParseMac(read.GetAwaiter().GetResult());
        } else if (OperatingSystem.IsLinux()) {
            volume = ParseLinux(File.ReadAllText("/proc/self/mountinfo"), root);
            string tool = File.Exists("/usr/bin/lsblk") ? "/usr/bin/lsblk" : "/bin/lsblk";
            var start = new ProcessStartInfo(tool) { RedirectStandardOutput = true, RedirectStandardError = true, UseShellExecute = false };
            foreach (string arg in new[] { "--json", "--list", "--output", "MAJ:MIN,FSTYPE,FSVER,UUID" }) start.ArgumentList.Add(arg);
            using var process = Process.Start(start) ?? throw new IOException("FAT32-Prüfung benötigt lsblk (util-linux).");
            var read = process.StandardOutput.ReadToEndAsync();
            if (!process.WaitForExit(5000)) { process.Kill(); throw new IOException("FAT32-Prüfung hat zu lange gedauert."); }
            if (process.ExitCode != 0) throw new IOException("FAT32 konnte nicht bestätigt werden.");
            volume = ApplyLinuxMetadata(volume, read.GetAwaiter().GetResult());
        }
        else throw new IOException("Diese Ausgabe unterstützt macOS und Linux.");
        if (string.IsNullOrWhiteSpace(volume.Mount) || !Path.IsPathFullyQualified(volume.Mount) || Clean(volume.Mount) != root || !volume.Writable || volume.Format != "FAT32" || string.IsNullOrWhiteSpace(volume.Identity))
            throw new IOException("Bitte das Hauptverzeichnis einer beschreibbaren FAT32-3DS-SD-Karte auswählen.");
        return volume;
    }
    internal static VolumeSnapshot ParseMac(string xml) {
        var dictionary = XDocument.Parse(xml).Root?.Element("dict") ?? throw new IOException("Ungültige Datenträgerantwort.");
        var nodes = dictionary.Elements().ToArray(); var values = new Dictionary<string,string>();
        for (int i = 0; i + 1 < nodes.Length; i += 2) values[nodes[i].Value] = nodes[i+1].Name.LocalName is "true" or "false" ? nodes[i+1].Name.LocalName : nodes[i+1].Value;
        string Get(string name) => values.TryGetValue(name, out var value) ? value : "";
        string personality = Get("FileSystemPersonality");
        if (personality.Length == 0) personality = Get("FilesystemName");
        if (personality.Length == 0) personality = Get("FilesystemUserVisibleName");
        var format = Get("FilesystemType") == "msdos" && personality.Contains("FAT32", StringComparison.OrdinalIgnoreCase) ? "FAT32" : personality;
        string uuid = Get("VolumeUUID");
        if (Get("MountPoint").Length == 0 || !Path.IsPathFullyQualified(Get("MountPoint")) || Get("DeviceIdentifier").Length == 0 || uuid.Length == 0) throw new IOException("Unvollständige SD-Datenträgeridentität.");
        return new(Get("MountPoint"), uuid.Length == 0 ? "" : Get("DeviceIdentifier") + ":" + uuid + ":" + Get("MediaUUID"), format, Get("WritableVolume") == "true");
    }
    internal static VolumeSnapshot ApplyLinuxMetadata(VolumeSnapshot volume, string json) {
        using var document = System.Text.Json.JsonDocument.Parse(json);
        foreach (var item in document.RootElement.GetProperty("blockdevices").EnumerateArray()) {
            string Get(string name) => item.TryGetProperty(name, out var value) ? value.GetString() ?? "" : "";
            if (Get("maj:min") != volume.Device) continue;
            string uuid = Get("uuid");
            if (Get("fstype") != "vfat" || !Get("fsver").Equals("FAT32", StringComparison.OrdinalIgnoreCase) || uuid.Length == 0)
                throw new IOException("Keine bestätigte FAT32-SD-Karte.");
            return volume with { Format = "FAT32", Identity = volume.Identity + ":" + uuid };
        }
        throw new IOException("Die FAT32-Datenträgeridentität konnte nicht geprüft werden.");
    }
    static string Unescape(string value) => value.Replace("\\040", " ").Replace("\\011", "\t").Replace("\\134", "\\");
    internal static VolumeSnapshot ParseLinux(string text, string root) {
        foreach (string line in text.Split('\n')) {
            string[] halves = line.Split(" - "); if (halves.Length != 2) continue;
            string[] left = halves[0].Split(' '), right = halves[1].Split(' ');
            if (left.Length < 6 || right.Length < 3 || Clean(Unescape(left[4])) != Clean(root)) continue;
            if (left[3] != "/") throw new IOException("Bitte das vollständige SD-Volume auswählen, keinen eingebundenen Unterordner.");
            return new(Unescape(left[4]), left[0] + ":" + left[2] + ":" + right[1], right[0], left[5].Split(',').Contains("rw") && right[2].Split(',').Contains("rw"), left[2]);
        }
        throw new IOException("Der ausgewählte Ordner ist kein eingebundener SD-Datenträger.");
    }
}
