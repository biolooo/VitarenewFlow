using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Web.Script.Serialization;
using Drawing = System.Drawing;

namespace LibraryDesktop {
    internal static class Tests {
        static readonly List<object> Results = new List<object>();
        static void Assert(bool test, string name) { if (!test) throw new Exception("FAIL: " + name); Results.Add(new { test = name, passed = true }); }
        static void Reject(Action action, string name) { bool rejected = false; try { action(); } catch { rejected = true; } Assert(rejected, name); }
        static void Write(string file, byte[] bytes) { Directory.CreateDirectory(Path.GetDirectoryName(file)); File.WriteAllBytes(file, bytes); }
        static byte[] Content(int length) { byte[] bytes = new byte[length]; for (int i = 0; i < length; i++) bytes[i] = (byte)((i * 31 + 17) % 251); return bytes; }
        static byte[] SampleCia() {
            byte[] data = new byte[0x5d40];
            Array.Copy(BitConverter.GetBytes(0x2020U),0,data,0,4);Array.Copy(BitConverter.GetBytes(0x200U),0,data,0x10,4);
            Array.Copy(BitConverter.GetBytes(0x3ac0U),0,data,0x14,4);Array.Copy(BitConverter.GetBytes(64UL),0,data,0x18,8);
            data[0x2041]=1;data[0x2043]=4;
            byte[] id = {0,4,0,0,0x12,0x34,0x56,0};Array.Copy(id,0,data,0x2040+0x140+0x4c,8);
            Array.Copy(System.Text.Encoding.ASCII.GetBytes("SMDH"),0,data,0x2280+0x400,4);
            Array.Copy(System.Text.Encoding.Unicode.GetBytes("Test CIA Game"),0,data,0x2280+0x400+8+0x200,26);
            return data;
        }
        public static int Run(string report) {
            string scratch = Path.Combine(Path.GetTempPath(), "3ds-library-desktop-tests-" + Guid.NewGuid().ToString("N"));
            Directory.CreateDirectory(scratch);
            string root = Path.Combine(scratch, "card"); string input = Path.Combine(scratch, "input");
            Directory.CreateDirectory(Path.Combine(root, "Nintendo 3DS"));
            Write(Path.Combine(root, "3ds/3ds-library/3ds-library.3dsx"), Content(123));
            try {
                var card = new Card(root, true);
                Assert(Card.Views.Length == 8 && new Game { System = "3ds" }.Category == "3ds" && new Game { System = "3ds", Homebrew = true }.Category == "homebrew", "Separate retail and homebrew categories");
                Assert(new Game { System = "gba" }.Category == "gameboy" && new Game { System = "gb" }.Category == "gameboy", "GB GBC and GBA share view and retain backend");
                Assert(Card.IsHomebrew("0004000000384A00") && !Card.IsHomebrew("0004000000053F00"), "Homebrew fixture and retail title classified correctly");
                string[] extensions = { ".nds", ".gba", ".z64", ".n64", ".v64", ".sfc", ".smc", ".nes", ".vb", ".gb", ".gbc" };
                string[] systems = { "ds", "gba", "n64", "n64", "n64", "snes", "snes", "nes", "vb", "gb", "gb" };
                for (int i = 0; i < extensions.Length; i++) Assert(Card.Detect("Example" + extensions[i].ToUpperInvariant()) == systems[i], "Extension mapping " + extensions[i]);
                Assert(Card.Detect("game.zip") == null && Card.Detect("game.3ds") == null, "Archive and 3DS dump not mistaken for launchable ROM");
                Reject(() => card.PathOnCard("../outside"), "Traversal rejected"); Reject(() => card.PathOnCard("C:\\outside"), "Absolute path rejected");
                Reject(() => card.CoverFor("gba", "../game.gba"), "Cover key traversal rejected"); Reject(() => card.CoverFor("bad", "game.gba"), "Unknown system rejected");
                Assert(Card.CleanTitle("Zelda (Europe) (En,Fr,De).gba") == "Zelda", "Known release tags cleaned");
                Assert(Card.CleanTitle("Game (The Journey).gba") == "Game (The Journey)", "Subtitle preserved");
                string rom = Path.Combine(input, "Sample (Europe).GBA"); Write(rom, Content(2200100));
                var plan = card.Plan(new[] { rom })[0];
                Assert(!plan.Skip && plan.Target == Path.GetFullPath(Path.Combine(root, "roms/gba/Sample (Europe).GBA")), "Automatic GBA destination");
                int progress = 0; string copyHash = card.Import(plan, p => { progress++; Assert(p.Done <= p.Total, "Bounded progress " + progress); }, () => false);
                Assert(File.Exists(plan.Target) && Card.Hash(rom) == copyHash && Card.Hash(plan.Target) == copyHash && progress > 1, "Verified chunked ROM import");
                Assert(card.Plan(new[] { rom })[0].Skip, "Existing ROM never overwritten");
                var game = card.Scan().Single();
                Assert(game.Key == "Sample (Europe).GBA" && game.CoverPath.EndsWith("gba\\Sample (Europe).GBA.png"), "Launcher cover key includes full ROM filename");
                string save = Path.ChangeExtension(plan.Target, ".sav"); Write(save, Content(4096)); string saveHash = Card.Hash(save);
                Assert(card.SaveCandidates(game).Contains(save), "Adjacent battery save discovered read-only");
                string jpg = Path.Combine(input, "large.jpg");
                using (var bitmap = new Drawing.Bitmap(1600, 1000)) { using (var g = Drawing.Graphics.FromImage(bitmap)) g.Clear(Drawing.Color.Coral); bitmap.Save(jpg, Drawing.Imaging.ImageFormat.Jpeg); }
                card.AssignCover(game, jpg);
                using (var image = Drawing.Image.FromFile(game.CoverPath)) Assert(image.Width == 256 && image.Height == 160, "JPG converted to aspect-preserving bounded PNG");
                byte[] pngMagic = File.ReadAllBytes(game.CoverPath).Take(8).ToArray();
                Assert(pngMagic.SequenceEqual(new byte[] {137,80,78,71,13,10,26,10}), "Output is real PNG");
                string firstCover = Card.Hash(game.CoverPath);
                string png = Path.Combine(input, "alpha.png");
                using (var bitmap = new Drawing.Bitmap(80, 120, Drawing.Imaging.PixelFormat.Format32bppArgb)) { bitmap.SetPixel(40, 60, Drawing.Color.Aqua); bitmap.Save(png, Drawing.Imaging.ImageFormat.Png); }
                card.AssignCover(game, png);
                using (var bitmap = new Drawing.Bitmap(game.CoverPath)) Assert(bitmap.Width == 80 && bitmap.Height == 120 && bitmap.GetPixel(0, 0).A == 0, "Small covers not upscaled and alpha retained");
                string[] backups = Directory.GetFiles(Path.Combine(root, "3ds/3ds-library/pc-backups"), "*.png", SearchOption.AllDirectories);
                Assert(backups.Length == 1 && Card.Hash(backups[0]) == firstCover, "Previous cover backup hash verified");
                string coverHash = Card.Hash(game.CoverPath);
                string corrupt = Path.Combine(input, "broken.png"); Write(corrupt, Content(30));
                Reject(() => card.AssignCover(game, corrupt), "Corrupt cover rejected");
                Assert(Card.Hash(game.CoverPath) == coverHash && Card.Hash(save) == saveHash, "Failed cover import preserves cover and save");
                string alternate = Path.Combine(root, "3ds/DaedalusX64/Roms/Already.z64"); Write(alternate, Content(40));
                string same = Path.Combine(input, "Already.z64"); Write(same, Content(50));
                Assert(card.Plan(new[] { same })[0].Skip, "Existing N64 ROM in legacy scan folder detected");
                string ds = Path.Combine(input, "New.nds"); Write(ds, Content(4000000));
                var twins = card.Plan(new[] { ds, ds }); Assert(!twins[0].Skip && twins[1].Skip, "Duplicate batch entries skipped");
                int chunks = 0;
                Reject(() => card.Import(twins[0], p => chunks++, () => chunks >= 1), "Cancellation interrupts import");
                Assert(!File.Exists(twins[0].Target) && !Directory.GetFiles(root, "*.partial", SearchOption.AllDirectories).Any(), "Cancelled partial copy cleaned");
                var stale = card.Plan(new[] { ds })[0]; Write(stale.Target, Content(70)); string existing = Card.Hash(stale.Target);
                Reject(() => card.Import(stale, null, null), "Stale plan cannot overwrite a new destination");
                Assert(Card.Hash(stale.Target) == existing, "Intervening destination unchanged");
                string cia = Path.Combine(input, "Application.cia"); Write(cia, SampleCia());
                var ciaplan = card.Plan(new[] { cia })[0]; card.Import(ciaplan, null, null);
                var pending = card.Scan().Single(g => g.Pending);
                Assert(ciaplan.Target == Path.GetFullPath(Path.Combine(root, "cias/Application.cia")) && pending.Key == "0004000012345600" && pending.Title == "Test CIA Game", "CIA metadata visible as pending native title");
                Assert(card.Plan(new[]{cia})[0].Skip, "CIA title ID duplicate rejected");
                card.AssignCover(pending,png);Assert(File.Exists(card.CoverFor("3ds",pending.Key)), "Pending CIA cover uses native title ID");
                string brokenCia = Path.Combine(input,"Broken.cia");Write(brokenCia,Content(100));Assert(card.Plan(new[]{brokenCia})[0].Skip,"Malformed CIA rejected before import");
                string archive = Path.Combine(input, "archive.zip"); Write(archive, Content(50));
                Assert(card.Plan(new[] { archive })[0].Skip, "Unsupported archive shown as skipped");
                string zero = Path.Combine(input, "empty.nes"); Write(zero, new byte[0]); Assert(card.Plan(new[] { zero })[0].Skip, "Empty ROM rejected");
                string nativeFolder = Path.Combine(root, "Nintendo 3DS", new string('a',32), new string('b',32), "title/00040000/0f800100"); Directory.CreateDirectory(nativeFolder);
                var native = card.Scan().Single(x => x.Native && !x.Pending);
                Assert(native.Key == "000400000F800100" && native.Title == "FBI", "Installed SD title ID discovered without decrypting contents");
                Assert(card.CoverFor(native.System, native.Key).EndsWith("3ds\\000400000F800100.png"), "Native title cover uses 16-digit title ID");
                for (int i = 0; i < extensions.Length; i++) {
                    string sample = Path.Combine(input, "Platform " + i + extensions[i]); Write(sample, Content(40 + i));
                    var mapping = card.Plan(new[] { sample })[0];
                    card.Import(mapping, null, null);
                    Assert(File.Exists(Path.Combine(root, "roms", Card.RomFolder(systems[i]), Path.GetFileName(sample))) && card.Scan().Any(g => g.Key == Path.GetFileName(sample) && g.System == systems[i]), "Imported ROM visible to library scan " + extensions[i]);
                }
                var before = Directory.GetFiles(root, "*", SearchOption.AllDirectories).ToDictionary(x => x, Card.Hash);
                card.Scan(); card.SaveCandidates(game);
                Assert(before.All(p => File.Exists(p.Key) && Card.Hash(p.Key) == p.Value) && Directory.GetFiles(root, "*", SearchOption.AllDirectories).Length == before.Count, "Library scan and save discovery never write");
                string marker = Path.Combine(root, "3ds/3ds-library/3ds-library.3dsx"); File.Move(marker, marker + ".test-backup");
                Reject(() => card.AssignCover(game, jpg), "Removed card marker blocks mutation");
                Assert(Card.Hash(save) == saveHash && Card.Hash(game.CoverPath) == coverHash, "Marker rejection preserves data");
                File.Move(marker + ".test-backup", marker);
                Reject(() => new Card(Path.Combine(scratch, "missing"), true), "Wrong directory rejected");
                foreach (string uiCheck in new Desktop(card).CheckUI()) Assert(true, uiCheck);
                foreach (string autoCheck in AutoCoverTests.Run()) Assert(true,autoCheck);
                Assert(true,new Desktop(card).CheckAutoCoverButton());
                File.WriteAllText(report, new JavaScriptSerializer().Serialize(new { version = "0.3.1", passed = true, checks = Results.Count, scratch = scratch, results = Results }), System.Text.Encoding.UTF8);
                return 0;
            } catch (Exception e) {
                File.WriteAllText(report, new JavaScriptSerializer().Serialize(new { version = "0.3.1", passed = false, error = e.ToString(), scratch = scratch, results = Results }), System.Text.Encoding.UTF8); return 1;
            }
        }
    }
}



