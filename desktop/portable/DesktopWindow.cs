// SPDX-License-Identifier: GPL-3.0-only
// Native Avalonia interface; the shared Card engine owns every SD write.
using System.Diagnostics;
using Avalonia;
using Avalonia.Controls;
using Avalonia.Controls.Templates;
using Avalonia.Input;
using Avalonia.Interactivity;
using Avalonia.Layout;
using Avalonia.Media;
using Avalonia.Media.Imaging;
using Avalonia.Platform.Storage;
using Avalonia.Styling;
using Avalonia.Threading;

namespace LibraryDesktop;

public sealed class DesktopWindow : Window {
    static readonly IBrush Ink = Brush("#EDF3F7"), Muted = Brush("#93A9B9"), Accent = Brush("#64E8C4"), Panel = Brush("#101923");
    readonly Dictionary<string, Button> navigation = new();
    readonly List<Game> games = new();
    readonly List<Bitmap> images = new();
    readonly List<Button> writeActions = new();
    readonly ListBox library = new() { Background = Brushes.Transparent, BorderThickness = new Thickness(0), SelectionMode = SelectionMode.Single };
    readonly TextBox search = new() { PlaceholderText = "Spiele suchen …", MinWidth = 180 };
    readonly TextBlock heading = Text("Deine Spiele.", 27), summary = Text("Wähle deine 3DS-SD-Karte aus.", 12, Muted);
    readonly TextBlock cardLabel = Text("Keine SD-Karte ausgewählt", 12, Muted), status = Text("Bereit. Die Karte bleibt an ihrem bisherigen Speicherort.", 12, Muted);
    readonly TextBlock empty = Text("SD-Karte auswählen, um deine Sammlung zu öffnen.", 16, Muted);
    readonly TextBlock detailTitle = Text("Deine nächste Session.", 21), detailPlatform = Text("", 12, Muted), detailFile = Text("", 11, Muted), detailNote = Text("Spiele und Cover verwalten, ohne vorhandene Spielstände zu verschieben.", 12, Muted), saveInfo = Text("Bekannte Speicherorte werden nur gelesen.", 12, Muted);
    readonly TextBlock artPlaceholder = Text("Wähle ein Spiel", 16, Muted);
    readonly Image detailCover = new() { Stretch = Stretch.Uniform, MaxHeight = 235 };
    readonly Button chooseCard = ActionButton("SD-Karte wählen", true), addGames = ActionButton("Spiele hinzufügen", true), autoCover = ActionButton("Cover automatisch"), refresh = ActionButton("Neu einlesen"), changeCover = ActionButton("Cover hinzufügen"), openGame = ActionButton("Spielordner öffnen"), openSave = ActionButton("Spielstandordner öffnen"), cancel = ActionButton("Abbrechen");
    readonly ProgressBar progress = new() { Minimum = 0, Maximum = 100, Height = 5, IsVisible = false, Foreground = Accent };
    Card card;
    string category = "all";
    bool busy, closeRequested;
    int operationGeneration;
    CancellationTokenSource operation;
    List<string> saves = new();

    public DesktopWindow() {
        Title = "VitarenewFlow · Desktop"; Width = 1320; Height = 860; MinWidth = 1060; MinHeight = 720;
        Background = Brush("#0B1016"); Foreground = Ink; RequestedThemeVariant = ThemeVariant.Dark;
        WindowStartupLocation = WindowStartupLocation.CenterScreen;
        Content = BuildShell();
        library.ItemTemplate = new FuncDataTemplate<Game>((game, _) => GameRow(game));
        library.SelectionChanged += (_, _) => ShowDetails();
        search.TextChanged += (_, _) => Filter();
        chooseCard.Click += async (_, _) => await ChooseCardAsync();
        refresh.Click += async (_, _) => { if (card != null) await LoadCardAsync(card); };
        addGames.Click += async (_, _) => await ChooseGamesAsync();
        changeCover.Click += async (_, _) => await ChooseCoverAsync();
        autoCover.Click += async (_, _) => await DownloadCoversAsync();
        openGame.Click += (_, _) => { if (Selected?.FilePath != null) Explore(Selected.FilePath, true); };
        openSave.Click += (_, _) => { if (saves.Count > 0) Explore(saves[0], true); };
        cancel.Click += (_, _) => CancelOperation();
        KeyDown += async (_, e) => { if (e.Key == Key.F5 && !busy && card != null) { e.Handled = true; await LoadCardAsync(card); } };
        DragDrop.SetAllowDrop(this, true);
        AddHandler(DragDrop.DragOverEvent, OnDragOver);
        AddHandler(DragDrop.DropEvent, OnDrop);
        Closing += (_, e) => { if (busy) { closeRequested = true; e.Cancel = true; CancelOperation(); } };
        Closed += (_, _) => DisposeImages();
        Filter(); SetBusy(false);
    }

    static IBrush Brush(string color) => new SolidColorBrush(Color.Parse(color));
    static TextBlock Text(string value, double size = 14, IBrush color = null) => new() { Text = value, FontSize = size, Foreground = color ?? Ink, TextWrapping = TextWrapping.Wrap };
    static Button ActionButton(string text, bool primary = false) => new() { Content = text, Padding = new Thickness(13, 10), Background = primary ? Accent : Brush("#1B2935"), Foreground = primary ? Brush("#0C2922") : Ink, BorderBrush = primary ? Accent : Brush("#2C3C4B"), BorderThickness = new Thickness(1), HorizontalContentAlignment = HorizontalAlignment.Center };
    static Border Surface(Control child, Thickness padding) => new() { Child = child, Background = Panel, CornerRadius = new CornerRadius(12), Padding = padding };
    static void At(Grid grid, Control child, int row = 0, int column = 0, int columnSpan = 1) { Grid.SetRow(child, row); Grid.SetColumn(child, column); Grid.SetColumnSpan(child, columnSpan); grid.Children.Add(child); }
    static StackPanel Stack(params Control[] children) { var panel = new StackPanel { Spacing = 12 }; foreach (var child in children) panel.Children.Add(child); return panel; }
    Game Selected => library.SelectedItem as Game;

    Control BuildShell() {
        var shell = new Grid { RowDefinitions = new RowDefinitions("Auto,*,Auto"), ColumnDefinitions = new ColumnDefinitions("210,*,310"), Margin = new Thickness(24), ColumnSpacing = 18, RowSpacing = 20 };
        var header = new Grid { ColumnDefinitions = new ColumnDefinitions("*,Auto") };
        At(header, Stack(Text("VitaRenewFlow", 25), Text("DESKTOP · DEINE SAMMLUNG", 10, Accent)), 0, 0);
        var mount = Stack(chooseCard, cardLabel); mount.HorizontalAlignment = HorizontalAlignment.Right; At(header, mount, 0, 1);
        At(shell, header, 0, 0, 3);
        var nav = new StackPanel { Spacing = 5 };
        nav.Children.Add(Text("BIBLIOTHEK", 10, Muted));
        foreach (string system in new[] { "all" }.Concat(Card.Views)) {
            string key = system;
            var button = ActionButton(system == "all" ? "Alle Spiele" : Card.Platform(system));
            button.HorizontalContentAlignment = HorizontalAlignment.Stretch; button.FontSize = 12;
            button.Click += (_, _) => { category = key; search.Text = ""; Filter(); };
            navigation.Add(system, button); nav.Children.Add(button);
        }
        var folders = Stack(FolderButton("SD-Karte öffnen", ""), FolderButton("Cover-Backups", "3ds/3ds-library/pc-backups")); folders.Margin = new Thickness(0, 24, 0, 0);
        nav.Children.Add(folders);
        nav.Children.Add(Text("Spiele bleiben in ihren Ordnern. Importiert wird erst nach deiner Bestätigung.", 11, Muted));
        At(shell, Surface(new ScrollViewer { Content = nav }, new Thickness(14)), 1, 0);
        var center = new Grid { RowDefinitions = new RowDefinitions("Auto,Auto,*,Auto"), RowSpacing = 15 };
        var title = new Grid { ColumnDefinitions = new ColumnDefinitions("*,Auto") };
        At(title, Stack(heading, summary)); search.Width = 230; search.VerticalAlignment = VerticalAlignment.Top; At(title, search, 0, 1);
        At(center, title);
        var actions = new WrapPanel { Orientation = Orientation.Horizontal }; foreach (var button in new[] { addGames, autoCover, refresh }) { button.Margin = new Thickness(0, 0, 8, 6); actions.Children.Add(button); }
        At(center, actions, 1);
        var listArea = new Grid(); listArea.Children.Add(library); empty.HorizontalAlignment = HorizontalAlignment.Center; empty.VerticalAlignment = VerticalAlignment.Center; empty.Margin = new Thickness(20); empty.TextAlignment = TextAlignment.Center; listArea.Children.Add(empty);
        At(center, Surface(listArea, new Thickness(8)), 2);
        At(center, Text("Ziehe Spiele hierher. Ein einzelnes PNG/JPG wird dem ausgewählten Spiel als Cover zugeordnet.", 11, Muted), 3);
        At(shell, center, 1, 1);
        var art = new Grid { Height = 240 }; art.Children.Add(detailCover); artPlaceholder.HorizontalAlignment = HorizontalAlignment.Center; artPlaceholder.VerticalAlignment = VerticalAlignment.Center; art.Children.Add(artPlaceholder);
        var details = Stack(Surface(art, new Thickness(12)), detailTitle, detailPlatform, detailFile, changeCover, openGame, detailNote, Text("SPIELSTÄNDE", 10, Accent), saveInfo, openSave);
        At(shell, Surface(new ScrollViewer { Content = details }, new Thickness(17)), 1, 2);
        var footer = new Grid { ColumnDefinitions = new ColumnDefinitions("*,Auto"), RowDefinitions = new RowDefinitions("Auto,Auto"), RowSpacing = 8 };
        At(footer, status); cancel.IsVisible = false; At(footer, cancel, 0, 1); At(footer, progress, 1, 0, 2); At(shell, footer, 2, 0, 3);
        writeActions.AddRange(new[] { chooseCard, addGames, autoCover, refresh, changeCover });
        return shell;
    }

    Button FolderButton(string label, string relative) {
        var button = ActionButton(label); button.FontSize = 11;
        button.Click += (_, _) => { if (card == null || busy) return; try { string path = card.PathOnCard(relative); if (Directory.Exists(path)) Explore(path, false); else SetStatus("Dieser Ordner ist noch nicht vorhanden."); } catch (Exception e) { SetStatus(e.Message, true); } };
        return button;
    }
    Control GameRow(Game game) {
        // Avalonia requests an empty template while changing the collection.
        if (game == null) return new Border();
        var row = new Grid { ColumnDefinitions = new ColumnDefinitions("68,*,Auto"), Margin = new Thickness(4, 8), ColumnSpacing = 12 };
        var art = new Grid { Width = 57, Height = 72 };
        if (game.Cover != null) art.Children.Add(new Image { Source = game.Cover, Stretch = Stretch.Uniform, Opacity = game.ArtOpacity });
        else art.Children.Add(new Border { Background = Brush("#213342"), CornerRadius = new CornerRadius(6), Child = new TextBlock { Text = game.Native ? "3DS" : game.System.ToUpperInvariant(), HorizontalAlignment = HorizontalAlignment.Center, VerticalAlignment = VerticalAlignment.Center, Foreground = Muted, FontSize = 11 } });
        At(row, art);
        var label = Text(game.Title, 15); label.FontWeight = FontWeight.SemiBold;
        var labels = Stack(label, Text(game.Caption, 11, Muted)); labels.Spacing = 5; labels.VerticalAlignment = VerticalAlignment.Center; At(row, labels, 0, 1);
        var cover = Text(game.Cover != null ? "Cover" : "Ohne Cover", 10, game.Cover != null ? Accent : Muted); cover.VerticalAlignment = VerticalAlignment.Center; At(row, cover, 0, 2);
        return row;
    }

    async Task ChooseCardAsync() {
        if (busy) return; Card chosen = null;
        bool ready = await WorkAsync(async token => {
            var selected = await StorageProvider.OpenFolderPickerAsync(new FolderPickerOpenOptions { Title = "Hauptverzeichnis der 3DS-SD-Karte auswählen", AllowMultiple = false });
            token.ThrowIfCancellationRequested(); string path = selected.FirstOrDefault()?.TryGetLocalPath(); if (path == null) return;
            chosen = await Task.Run(() => new Card(path)); token.ThrowIfCancellationRequested();
        });
        if (ready && chosen != null && !closeRequested) await LoadCardAsync(chosen);
    }
    async Task LoadCardAsync(Card selected, string preferred = null) {
        if (busy) return;
        await WorkAsync(async token => {
            SetStatus("Spiele auf der SD-Karte werden gelesen …");
            var loaded = await Task.Run(() => {
                var found = selected.Scan(); var decoded = new List<Bitmap>();
                try {
                    foreach (var game in found) {
                        token.ThrowIfCancellationRequested();
                        if (!File.Exists(game.CoverPath)) continue;
                        try { var image = PortableImages.LoadPreview(game.CoverPath); game.Cover = image; decoded.Add(image); } catch (IOException) { game.Cover = null; }
                    }
                    return (Games: found, Images: decoded, Free: PortableVolume.FreeBytes(selected.Root));
                } catch { foreach (var image in decoded) image.Dispose(); throw; }
            });
            bool adopted = false;
            try {
                token.ThrowIfCancellationRequested(); DisposeImages(); card = selected; games.Clear(); games.AddRange(loaded.Games); images.AddRange(loaded.Images); adopted = true;
                cardLabel.Text = Path.GetFileName(card.Root.TrimEnd(Path.DirectorySeparatorChar)) + " · " + Card.Bytes(loaded.Free) + " frei";
                Filter(); if (preferred != null) library.SelectedItem = games.FirstOrDefault(g => Identity(g) == preferred) ?? library.SelectedItem;
                SetStatus(string.IsNullOrEmpty(card.DiscoveryNotice) ? games.Count + " Spiele gelesen · SD-Karte verbunden" : card.DiscoveryNotice, !string.IsNullOrEmpty(card.DiscoveryNotice));
            } finally { if (!adopted) foreach (var image in loaded.Images) image.Dispose(); }
        });
    }
    static string Identity(Game game) => game.System + "/" + game.Key + "/" + game.FilePath;
    void Filter() {
        string previous = Selected == null ? null : Identity(Selected), query = (search.Text ?? "").Trim();
        var filtered = games.Where(g => (category == "all" || g.Category == category) && (query.Length == 0 || g.Title.Contains(query, StringComparison.CurrentCultureIgnoreCase) || g.Key.Contains(query, StringComparison.OrdinalIgnoreCase))).ToList();
        library.ItemsSource = filtered; library.SelectedItem = filtered.FirstOrDefault(g => Identity(g) == previous) ?? filtered.FirstOrDefault();
        heading.Text = category == "all" ? "Deine Spiele." : Card.Platform(category);
        summary.Text = filtered.Count + " Einträge · " + games.Count(g => !File.Exists(g.CoverPath)) + " ohne Cover";
        empty.Text = card == null ? "SD-Karte auswählen, um deine Sammlung zu öffnen." : "Keine passenden Spiele gefunden."; empty.IsVisible = filtered.Count == 0;
        foreach (var entry in navigation) {
            int count = entry.Key == "all" ? games.Count : games.Count(g => g.Category == entry.Key);
            entry.Value.Content = (entry.Key == "all" ? "Alle Spiele" : Card.Platform(entry.Key)) + "  ·  " + count;
            entry.Value.Background = Brush(entry.Key == category ? "#203E36" : "#14202A"); entry.Value.Foreground = entry.Key == category ? Accent : Muted;
        }
        ShowDetails();
    }
    void ShowDetails() {
        Game game = Selected; detailCover.Source = game?.Cover; artPlaceholder.IsVisible = game?.Cover == null; artPlaceholder.Text = game == null ? "Wähle ein Spiel" : "Noch kein Cover";
        detailTitle.Text = game?.Title ?? "Deine nächste Session."; detailPlatform.Text = game?.Caption ?? ""; detailFile.Text = game?.Key ?? "";
        detailNote.Text = game == null ? "Spiele und Cover verwalten, ohne vorhandene Spielstände zu verschieben." : game.Pending ? "CIA bereitgestellt. Auf dem 3DS in Flow auswählen und mit A installieren. Ein Cover kannst du hier schon zuordnen." : game.Native ? "Titel-ID auf der SD erkannt. 3DS-Titelnamen sind am Computer verschlüsselt. Originale Banner bleiben erhalten." : "Datei: " + Relative(game.FilePath) + "\nDer Dateiname bleibt erhalten, damit Spielstände weiter zugeordnet werden.";
        try { saves = game == null || card == null ? new() : card.SaveCandidates(game); } catch (Exception e) { saves = new(); SetStatus(e.Message, true); }
        saveInfo.Text = game == null ? "Bekannte Speicherorte werden nur gelesen." : game.Native ? "3DS-Spielstände sind verschlüsselt. Ein Export wird hier nicht durchgeführt." : saves.Count == 0 ? "An bekannten Speicherorten keine Datei gefunden. Emulator-Menüs können andere Speicherorte verwenden." : string.Join("\n", saves.Select(Relative)) + "\n\nNur angezeigt. Kein Austausch mit PC-Emulatoren.";
        changeCover.Content = game != null && File.Exists(game.CoverPath) ? "Cover ändern" : "Cover hinzufügen";
        changeCover.IsEnabled = game != null && !busy; openGame.IsEnabled = game?.FilePath != null && !busy; openSave.IsVisible = saves.Count > 0; openSave.IsEnabled = !busy && saves.Count > 0;
    }
    string Relative(string path) => path == null || card == null ? "" : Path.GetRelativePath(card.Root, path);
    void DisposeImages() { detailCover.Source = null; foreach (var image in images) image.Dispose(); images.Clear(); }
    void SetStatus(string message, bool error = false) { status.Text = message; status.Foreground = error ? Brush("#FFBE9E") : Muted; }
    void SetBusy(bool value) {
        busy = value;
        foreach (var button in writeActions) button.IsEnabled = !value && (button == chooseCard || card != null);
        foreach (var button in navigation.Values) button.IsEnabled = !value && card != null;
        search.IsEnabled = !value && card != null; library.IsEnabled = !value; cancel.IsVisible = value; progress.IsVisible = value; progress.IsIndeterminate = value;
        ShowDetails();
    }
    void CancelOperation() { operation?.Cancel(); SetStatus("Vorgang wird sicher beendet …"); }
    async Task<bool> WorkAsync(Func<CancellationToken, Task> action) {
        if (busy) return false;
        bool complete = false; operation = new CancellationTokenSource(); operationGeneration++; SetBusy(true);
        try { await action(operation.Token); complete = true; }
        catch (OperationCanceledException) { SetStatus("Vorgang abgebrochen. Bereits abgeschlossene Dateien bleiben erhalten."); }
        catch (Exception e) { SetStatus(e.Message, true); }
        finally { operation.Dispose(); operation = null; SetBusy(false); if (closeRequested) Close(); }
        return complete;
    }
    void PostProgress(int generation, double percent, string message) => Dispatcher.UIThread.Post(() => {
        if (!busy || generation != operationGeneration) return;
        progress.Value = percent; SetStatus(message);
    });
    void Explore(string path, bool reveal) {
        if (busy || card == null) return;
        try {
            card.Validate();
            var process = new ProcessStartInfo(OperatingSystem.IsMacOS() ? "/usr/bin/open" : "xdg-open") { UseShellExecute = false };
            if (OperatingSystem.IsMacOS() && reveal) process.ArgumentList.Add("-R");
            process.ArgumentList.Add(!OperatingSystem.IsMacOS() && reveal ? Path.GetDirectoryName(path) ?? path : path);
            Process.Start(process);
        } catch (Exception e) { SetStatus("Ordner konnte nicht geöffnet werden: " + e.Message, true); }
    }

    async Task ChooseGamesAsync() {
        if (busy || card == null) return; string[] files = null;
        bool ready = await WorkAsync(async token => {
            var selected = await StorageProvider.OpenFilePickerAsync(new FilePickerOpenOptions { Title = "Spiele hinzufügen", AllowMultiple = true, FileTypeFilter = new[] { new FilePickerFileType("Spiele und CIA-Pakete") { Patterns = new[] { "*.nds", "*.gba", "*.n64", "*.v64", "*.z64", "*.sfc", "*.smc", "*.nes", "*.vb", "*.gb", "*.gbc", "*.cia" } }, FilePickerFileTypes.All } });
            token.ThrowIfCancellationRequested(); files = selected.Select(x => x.TryGetLocalPath()).Where(x => x != null).ToArray();
        });
        if (ready && files != null && !closeRequested) await PreviewImportAsync(files);
    }
    async Task ChooseCoverAsync() {
        if (busy || card == null || Selected == null) return; Game game = Selected; string file = null;
        bool ready = await WorkAsync(async token => {
            var selected = await StorageProvider.OpenFilePickerAsync(new FilePickerOpenOptions { Title = "Cover auswählen · PNG oder JPG", AllowMultiple = false, FileTypeFilter = new[] { new FilePickerFileType("Cover-Bilder") { Patterns = new[] { "*.png", "*.jpg", "*.jpeg" } } } });
            token.ThrowIfCancellationRequested(); file = selected.FirstOrDefault()?.TryGetLocalPath();
        });
        if (ready && file != null && !closeRequested) await PutCoverAsync(game, file);
    }
    async Task PutCoverAsync(Game game, string file) {
        if (busy || card == null) return; string result = null, selected = Identity(game);
        await WorkAsync(async token => { result = await Task.Run(() => { token.ThrowIfCancellationRequested(); return card.AssignCover(game, file); }); });
        if (result != null && !closeRequested) { await LoadCardAsync(card, selected); SetStatus(result); }
    }
    void OnDragOver(object sender, DragEventArgs e) { e.DragEffects = !busy && card != null && e.DataTransfer.TryGetFiles()?.Any() == true ? DragDropEffects.Copy : DragDropEffects.None; e.Handled = true; }
    async void OnDrop(object sender, DragEventArgs e) {
        e.Handled = true; if (busy || card == null) return;
        try {
            var files = e.DataTransfer.TryGetFiles()?.Select(x => x.TryGetLocalPath()).Where(x => x != null).ToArray() ?? Array.Empty<string>();
            if (files.Length == 1 && Card.IsImage(files[0])) { if (Selected == null) SetStatus("Wähle zuerst das Spiel für dieses Cover.", true); else await PutCoverAsync(Selected, files[0]); }
            else await PreviewImportAsync(files);
        } catch (Exception error) { SetStatus(error.Message, true); }
    }
    Window BuildImportPreview(IReadOnlyList<ImportPlan> plans, out Button confirm) {
        var dialog = new Window { Title = "Spiele hinzufügen", Width = 760, Height = 540, MinWidth = 620, MinHeight = 400, Background = Panel, Foreground = Ink, RequestedThemeVariant = ThemeVariant.Dark, WindowStartupLocation = WindowStartupLocation.CenterOwner };
        int valid = plans.Count(p => !p.Skip);
        var content = new Grid { RowDefinitions = new RowDefinitions("Auto,*,Auto"), Margin = new Thickness(25), RowSpacing = 18 };
        At(content, Stack(Text(valid + " Dateien bereit zum Import", 24), Text("Vorhandene Dateien bleiben erhalten. CIA-Pakete anschließend auf dem 3DS installieren. ZIP/7z vorher entpacken.", 12, Muted)));
        var rows = new StackPanel { Spacing = 14 }; foreach (var plan in plans) rows.Children.Add(Stack(Text(plan.Name, 14), Text(plan.Destination, 12, plan.Skip ? Brush("#FFBE9E") : Accent)));
        At(content, new ScrollViewer { Content = rows }, 1);
        var actions = new StackPanel { Orientation = Orientation.Horizontal, HorizontalAlignment = HorizontalAlignment.Right, Spacing = 12 };
        var back = ActionButton("Zurück"); back.Click += (_, _) => dialog.Close(false);
        confirm = ActionButton("Jetzt importieren", true); confirm.IsEnabled = valid > 0; confirm.Click += (_, _) => dialog.Close(true);
        actions.Children.Add(back); actions.Children.Add(confirm); At(content, actions, 2); dialog.Content = content; return dialog;
    }
    async Task PreviewImportAsync(string[] files) {
        if (busy || card == null || files.Length == 0) return;
        List<ImportPlan> plans = null; bool ready = await WorkAsync(async token => { plans = await Task.Run(() => card.Plan(files)); token.ThrowIfCancellationRequested(); });
        if (!ready || plans == null || closeRequested) return;
        var dialog = BuildImportPreview(plans, out _); if (!await dialog.ShowDialog<bool>(this)) return;
        await ImportAsync(plans);
    }
    async Task ImportAsync(List<ImportPlan> plans) {
        var valid = plans.Where(p => !p.Skip).ToList(); if (valid.Count == 0 || busy || card == null) return;
        int copied = 0, skipped = plans.Count - valid.Count; string preferred = null; var errors = new List<string>();
        bool complete = await WorkAsync(async token => {
            progress.IsIndeterminate = false; int generation = operationGeneration;
            await Task.Run(() => {
                long total = valid.Sum(p => new FileInfo(p.Source).Length), completed = 0;
                foreach (var plan in valid) {
                    token.ThrowIfCancellationRequested(); long length = 0;
                    try {
                        length = new FileInfo(plan.Source).Length;
                        card.Import(plan, item => PostProgress(generation, Math.Min(100, (completed + item.Done) * 100d / Math.Max(1, total)), "Kopiere und prüfe · " + item.Name), () => token.IsCancellationRequested);
                        copied++; preferred = plan.Target;
                    } catch (OperationCanceledException) { throw; }
                    catch (Exception e) { errors.Add(plan.Name + ": " + e.Message); }
                    completed += length;
                }
            });
        });
        string failure = status.Text;
        if (!closeRequested) { category = "all"; await LoadCardAsync(card); if (preferred != null) library.SelectedItem = games.FirstOrDefault(g => g.FilePath == preferred); SetStatus(copied + " importiert · " + skipped + " übersprungen" + (!complete ? " · " + failure : errors.Count > 0 ? " · " + errors[0] : ". Auf dem 3DS neu einlesen; CIA dort mit A installieren."), !complete || errors.Count > 0); }
    }
    async Task DownloadCoversAsync() {
        if (busy || card == null) return; CoverBatch result = null; var snapshot = games.ToList(); string selected = Selected == null ? null : Identity(Selected);
        await WorkAsync(async token => {
            progress.IsIndeterminate = false; int generation = operationGeneration;
            result = await Task.Run(() => new AutoCovers().Run(card, snapshot, (percent, text) => PostProgress(generation, percent, text), () => token.IsCancellationRequested));
        });
        if (result == null || closeRequested) return;
        await LoadCardAsync(card, selected); SetStatus(result.Summary);
        var rows = new StackPanel { Spacing = 14 }; foreach (var item in result.Items.Where(x => x.Status != "vorhanden")) rows.Children.Add(Stack(Text(item.Title + " · " + item.Status, 14), Text(item.Detail, 12, item.Status == "geladen" ? Accent : Muted)));
        var dialog = new Window { Title = "Cover-Suche", Width = 720, Height = 520, Background = Panel, Foreground = Ink, RequestedThemeVariant = ThemeVariant.Dark, WindowStartupLocation = WindowStartupLocation.CenterOwner };
        var layout = new DockPanel { Margin = new Thickness(24) }; var done = ActionButton("Fertig"); done.Click += (_, _) => dialog.Close(); DockPanel.SetDock(done, Dock.Bottom); layout.Children.Add(done); var label = Text(result.Summary, 20); label.Margin = new Thickness(0, 0, 0, 18); DockPanel.SetDock(label, Dock.Top); layout.Children.Add(label); layout.Children.Add(new ScrollViewer { Content = rows }); dialog.Content = layout; await dialog.ShowDialog(this);
    }

    internal async Task SetCardForTestAsync(Card selected) { await LoadCardAsync(selected); if (card != selected) throw new InvalidOperationException("Synthetic card load failed: " + status.Text); }
    internal async Task RunUiChecksAsync() {
        if (card == null || games.Count == 0) throw new InvalidOperationException("UI fixture needs a populated synthetic card.");
        foreach (string view in Card.Views) { navigation[view].RaiseEvent(new RoutedEventArgs(Button.ClickEvent)); if (library.ItemCount != games.Count(g => g.Category == view)) throw new Exception("UI category failed: " + view); }
        navigation["all"].RaiseEvent(new RoutedEventArgs(Button.ClickEvent));
        Game rom = games.FirstOrDefault(g => !g.Native); if (rom != null) {
            search.Text = rom.Key; await Dispatcher.UIThread.InvokeAsync(() => { }, DispatcherPriority.Background); if (library.ItemCount < 1 || Selected == null || detailTitle.Text != Selected.Title) throw new Exception("UI search/details failed.");
            if (!changeCover.IsEnabled || !openGame.IsEnabled) throw new Exception("UI ROM actions unavailable.");
            var plans = card.Plan(new[] { rom.FilePath }); var preview = BuildImportPreview(plans, out var confirm); if (plans.Count != 1 || !plans[0].Skip || confirm.IsEnabled) throw new Exception("UI preview must skip an existing file.");
            if (preview.Content == null) throw new Exception("UI preview has no rows.");
        }
        search.Text = "ThereIsNoSuchGame_5d96b324"; await Dispatcher.UIThread.InvokeAsync(() => { }, DispatcherPriority.Background); if (library.ItemCount != 0 || !empty.IsVisible || changeCover.IsEnabled) throw new Exception("UI empty state failed: items=" + library.ItemCount + " empty=" + empty.IsVisible + " coverEnabled=" + changeCover.IsEnabled);
        search.Text = ""; navigation["homebrew"].RaiseEvent(new RoutedEventArgs(Button.ClickEvent)); if (Selected != null && !Selected.Pending && (openGame.IsEnabled || openSave.IsVisible)) throw new Exception("UI native actions expose encrypted data.");
        SetBusy(true); if (addGames.IsEnabled || autoCover.IsEnabled || changeCover.IsEnabled || chooseCard.IsEnabled || navigation["all"].IsEnabled || !cancel.IsVisible) throw new Exception("UI busy write protection failed.");
        SetBusy(false); navigation["all"].RaiseEvent(new RoutedEventArgs(Button.ClickEvent));
        SetStatus("UI-Prüfung bestanden: Kategorien, Suche, Details, Importvorschau und Schreibschutz.");
    }
}
