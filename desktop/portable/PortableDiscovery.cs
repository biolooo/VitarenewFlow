namespace LibraryDesktop;
internal static class PortableDiscovery {
    internal static readonly string[] RootNames = { "roms", "games", "nds", "ds", "gba", "gb", "gbc", "gameboy", "game boy", "game boy advance", "gameboy advance", "game boy color", "n64", "nintendo 64", "nes", "snes", "super nintendo", "vb", "virtual boy", "virtualboy" };
    static readonly string[] Emulators = { "DaedalusX64", "mGBA", "snes9x_3ds", "virtuanes_3ds", "red-viper" };
    internal static bool Link(string path) => (File.GetAttributes(path) & FileAttributes.ReparsePoint) != 0;
    public static IReadOnlyList<string> Scan(Card card) {
        var files = new List<string>(); var seenFiles = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
        var visited = new Dictionary<string,int>(StringComparer.OrdinalIgnoreCase); int entries = 0, directories = 0; bool limited = false, stopped = false;
        void Walk(string root, int depth, bool dsOnly) {
            if (stopped) return;
            if (depth > 8) { limited = true; return; }
            if (System.Text.Encoding.UTF8.GetByteCount("sdmc:/"+Path.GetRelativePath(card.Root,root).Replace('\\','/')) >= 512) { limited = true; return; }
            string key = root + (dsOnly ? "|ds" : "|all");
            if (visited.TryGetValue(key, out int oldDepth) && oldDepth <= depth) return;
            visited[key] = depth;
            try {
                card.PathOnCard(Path.GetRelativePath(card.Root, root));
                if (!Directory.Exists(root)) { limited = true; return; }
                if (Link(root)) return;
                if (++directories > 256) { limited = stopped = true; return; }
                foreach (string path in Directory.EnumerateFileSystemEntries(root)) {
                    if (++entries > 12000) { limited = stopped = true; return; }
                    if (Path.GetFileName(path).StartsWith('.') || Link(path)) continue;
                    if (System.Text.Encoding.UTF8.GetByteCount("sdmc:/"+Path.GetRelativePath(card.Root,path).Replace('\\','/')) >= 512) { limited = true; continue; }
                    if (Directory.Exists(path)) { if (depth == 8) limited = true; else Walk(path, depth+1, dsOnly); }
                    else if (Card.Detect(path) is string system && (!dsOnly || system == "ds") && seenFiles.Add(path)) files.Add(path);
                    if (limited && entries > 12000) return;
                }
            } catch (IOException) { limited = true; } catch (UnauthorizedAccessException) { limited = true; }
        }
        // Explicit DS roots keep priority and are .nds-only, matching the console.
        string config = card.PathOnCard("3ds/3ds-library/ds-folders.txt");
        if (!File.Exists(config)) config += ".bak";
        if (File.Exists(config) && !Link(config) && new FileInfo(config).Length <= 8 * 514) {
            var roots = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
            foreach (string line in File.ReadLines(config)) {
                if (roots.Count == 8) break;
                string relative = line.Trim();
                if (!relative.StartsWith("sdmc:/", StringComparison.OrdinalIgnoreCase) || relative.Contains('\\') || relative.Substring(6).Contains(':')) continue;
                relative = relative.Substring(6).TrimStart('/');
                if (relative.Length == 0) { limited = true; continue; } // Explicit whole-SD DS root needs a narrower folder on desktop.
                try { if (relative.Length > 0) { string resolved=card.PathOnCard(relative); if(roots.Add(resolved)) Walk(resolved, 0, true); } } catch (IOException) { limited = true; }
            }
        }
        foreach (string directory in Directory.EnumerateFileSystemEntries(card.Root)) {
            if (++entries > 12000) { limited = stopped = true; break; }
            if (stopped) break;
            if (Link(directory) || !Directory.Exists(directory)) continue;
            string name = Path.GetFileName(directory);
            if (RootNames.Contains(name, StringComparer.OrdinalIgnoreCase)) Walk(directory, 0, false);
            if (!name.Equals("3ds", StringComparison.OrdinalIgnoreCase)) continue;
            foreach (string app in Directory.EnumerateFileSystemEntries(directory)) {
                if (++entries > 12000) { limited = stopped = true; break; }
                if (!Directory.Exists(app) || Link(app) || !Emulators.Contains(Path.GetFileName(app), StringComparer.OrdinalIgnoreCase)) continue;
                foreach (string folder in Directory.EnumerateFileSystemEntries(app)) {
                    if (++entries > 12000) { limited = stopped = true; break; }
                    if (Directory.Exists(folder) && !Link(folder) && new[] { "roms", "games" }.Contains(Path.GetFileName(folder), StringComparer.OrdinalIgnoreCase)) Walk(folder, 0, false);
                }
            }
        }
        card.DiscoveryNotice = limited ? "Die Suche hat eine Grenze oder einen nicht lesbaren Ordner erreicht; die Liste kann unvollständig sein." : "";
        return files;
    }
}
