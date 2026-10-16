using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Markup;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using Microsoft.Win32;

namespace LibraryDesktop {
    public sealed class Desktop {
        readonly Card card;
        readonly Window window;
        readonly ListBox grid;
        List<Game> games = new List<Game>();
        readonly Dictionary<string, Button> nav = new Dictionary<string, Button>();
        List<string> saves = new List<string>();
        string category = "all";
        bool busy;
        BackgroundWorker worker;
        internal Func<AutoCovers> AutoCoverFactory = () => new AutoCovers();
        internal Action<CoverBatch> AutoCoverCompleted;
        static Brush Brush(string color) { return (Brush)new BrushConverter().ConvertFromString(color); }
        T Get<T>(string name) where T : FrameworkElement { return (T)window.FindName(name); }

        public Desktop(Card c) {
            card = c;
            using (var stream = Assembly.GetExecutingAssembly().GetManifestResourceStream("MainWindow.xaml")) window = (Window)XamlReader.Load(stream);
            using (var stream = Assembly.GetExecutingAssembly().GetManifestResourceStream("BrandIcon.png")) {
                var icon = new BitmapImage(); icon.BeginInit(); icon.CacheOption = BitmapCacheOption.OnLoad; icon.StreamSource = stream; icon.EndInit(); icon.Freeze();
                window.Icon = icon; Get<System.Windows.Controls.Image>("BrandIcon").Source = icon;
            }
            grid = Get<ListBox>("Games");
            Get<Button>("AddGames").Click += delegate { BrowseGames(); };
            Get<Button>("AutoCovers").Click += delegate { StartAutoCovers(); };
            Get<Button>("Refresh").Click += delegate { Reload(null); };
            Get<TextBox>("Search").TextChanged += delegate { Get<TextBlock>("SearchHint").Visibility = Get<TextBox>("Search").Text.Length == 0 ? Visibility.Visible : Visibility.Collapsed; Filter(); };
            Get<Button>("OpenCard").Click += delegate { Explore(card.Root, false); };
            Get<Button>("OpenBackups").Click += delegate { string folder = card.PathOnCard("3ds/3ds-library/pc-backups"); if (Directory.Exists(folder)) Explore(folder, false); else SetStatus("Noch keine Cover-Backups. Sie entstehen beim Ersetzen eines vorhandenen Covers."); };
            Get<Button>("ChangeCover").Click += delegate { BrowseCover(); };
            Get<Button>("OpenGameFolder").Click += delegate { var g = grid.SelectedItem as Game; if (g != null && g.FilePath != null) Explore(g.FilePath, true); };
            Get<Button>("OpenSave").Click += delegate { if (saves.Count > 0) Explore(saves[0], true); };
            Get<Button>("CancelImport").Click += delegate { if (worker != null) { worker.CancelAsync(); SetStatus("Vorgang wird sicher abgebrochen …"); } };
            grid.SelectionChanged += delegate { ShowDetails(); };
            window.DragOver += OnDragOver;
            window.Drop += OnDrop;
            window.Closing += delegate(object sender, CancelEventArgs e) { if (busy) { if (worker != null) worker.CancelAsync(); e.Cancel = true; SetStatus("Bitte kurz warten: Der Vorgang wird sicher beendet. Danach kannst du die App schließen."); } };
            window.PreviewKeyDown += delegate(object sender, KeyEventArgs e) { if (e.Key == Key.F5 && !busy) { Reload(null); e.Handled = true; } };
            BuildNav(); Reload(null);
        }
        void BuildNav() {
            var stack = Get<StackPanel>("Navigation");
            foreach (string system in new[] { "all" }.Concat(Card.Views)) {
                string s = system;
                var button = new Button { Tag = s, HorizontalContentAlignment = HorizontalAlignment.Stretch, Margin = new Thickness(0, 0, 0, 5), Padding = new Thickness(11, 11, 10, 11), FontSize = 12, BorderThickness = new Thickness(0) };
                button.Click += delegate { category = s; Get<TextBox>("Search").Text = ""; Filter(); };
                nav[s] = button; stack.Children.Add(button);
            }
        }
        void UpdateNav() {
            foreach (var pair in nav) {
                int count = pair.Key == "all" ? games.Count : games.Count(g => g.Category == pair.Key);
                var content = new DockPanel();
                var amount = new TextBlock { Text = count.ToString(), Foreground = Brush(pair.Key == category ? "#98E6D0" : "#688597"), HorizontalAlignment = HorizontalAlignment.Right, FontSize = 11, VerticalAlignment = VerticalAlignment.Center };
                DockPanel.SetDock(amount, Dock.Right); content.Children.Add(amount);
                content.Children.Add(new TextBlock { Text = pair.Key == "all" ? "Alle Spiele" : Card.Platform(pair.Key), FontSize = 11, TextTrimming = TextTrimming.CharacterEllipsis });
                pair.Value.Content = content;
                pair.Value.Background = Brush(pair.Key == category ? "#203B35" : "#101821");
                pair.Value.Foreground = Brush(pair.Key == category ? "#94E5CF" : "#AFBFCC");
            }
        }
        static ImageSource LoadCover(string path, int width) {
            if (!File.Exists(path)) return null;
            try {
                using (var stream = File.OpenRead(path)) {
                    var image = new BitmapImage(); image.BeginInit(); image.CacheOption = BitmapCacheOption.OnLoad; image.DecodePixelWidth = width; image.StreamSource = stream; image.EndInit(); image.Freeze(); return image;
                }
            } catch { return null; }
        }
        void Reload(string preferred) {
            if (busy) return;
            try {
                games = card.Scan(); foreach (var game in games) game.Cover = LoadCover(game.CoverPath, 300);
                var drive = new DriveInfo(Path.GetPathRoot(card.Root));
                Get<TextBlock>("CardLabel").Text = Path.GetPathRoot(card.Root).TrimEnd('\\') + "  ·  " + Card.Bytes(drive.AvailableFreeSpace) + " frei";
                Filter(); if (preferred != null) grid.SelectedItem = games.FirstOrDefault(g => g.System + "/" + g.Key == preferred);
            } catch (Exception e) { SetStatus(e.Message, true); }
        }
        void Filter() {
            if (grid == null) return;
            Game previous = grid.SelectedItem as Game;
            string query = Get<TextBox>("Search").Text.Trim();
            var visible = games.Where(g => (category == "all" || g.Category == category) && (query.Length == 0 || g.Title.IndexOf(query, StringComparison.CurrentCultureIgnoreCase) >= 0 || g.Key.IndexOf(query, StringComparison.CurrentCultureIgnoreCase) >= 0)).ToList();
            grid.ItemsSource = visible;
            grid.SelectedItem = previous != null && visible.Contains(previous) ? previous : visible.FirstOrDefault();
            Get<TextBlock>("Heading").Text = category == "all" ? "Deine Spiele." : Card.Platform(category);
            Get<TextBlock>("Summary").Text = visible.Count + " Einträge  ·  " + games.Count(g => !g.Native && !File.Exists(g.CoverPath)) + " ROMs ohne Cover in der Sammlung";
            Get<Border>("Empty").Visibility = visible.Count == 0 ? Visibility.Visible : Visibility.Collapsed;
            UpdateNav(); ShowDetails();
        }
        void ShowDetails() {
            Game g = grid.SelectedItem as Game;
            Get<Button>("ChangeCover").IsEnabled = g != null && !busy;
            Get<Button>("OpenGameFolder").IsEnabled = g != null && g.FilePath != null;
            Get<Image>("DetailCover").Source = g == null ? null : g.Cover;
            Get<TextBlock>("DetailPlaceholder").Visibility = g != null && g.Cover != null ? Visibility.Collapsed : Visibility.Visible;
            Get<TextBlock>("DetailPlaceholder").Text = g == null ? "Wähle ein Spiel" : "Noch kein Cover";
            Get<TextBlock>("DetailTitle").Text = g == null ? "Deine nächste Session." : g.Title;
            Get<TextBlock>("DetailPlatform").Text = g == null ? "" : g.Caption;
            Get<Button>("ChangeCover").Content = g != null && File.Exists(g.CoverPath) ? "Cover ändern" : "Cover hinzufügen";
            Get<TextBlock>("DetailFile").Text = g == null ? "" : g.Key;
            Get<TextBlock>("DetailNote").Text = g == null ? "" : g.Pending ? "CIA bereitgestellt. Auf dem 3DS in der Library auswählen und mit A installieren. Ein eigenes Cover kannst du schon hier zuordnen." : g.Native ? "Titel-ID auf der SD erkannt. Originale Banner bleiben erhalten; das Cover dient dem Desktop und als Fallback. Unbekannte Titelnamen sind am PC verschlüsselt." : "Ordner: " + g.FilePath.Substring(card.Root.Length).Replace('\\', '/') + "\nDer Dateiname bleibt erhalten, damit Spielstände weiter zugeordnet werden.";
            saves = g == null ? new List<string>() : card.SaveCandidates(g);
            Get<TextBlock>("SaveInfo").Text = g == null ? "Bekannte Speicherorte werden nur gelesen." : g.Native ? "3DS-Spielstände sind verschlüsselt. Export und PC-Emulation planen wir getrennt." : saves.Count == 0 ? "An bekannten Speicherorten keine Datei gefunden. Speicherstände aus dem Emulator-Menü können anders liegen." : String.Join("\n", saves.Select(p => p.Substring(card.Root.Length).Replace('\\', '/'))) + "\n\nNur angezeigt. Kein Austausch mit PC-Emulatoren.";
            Get<Button>("OpenSave").Visibility = saves.Count > 0 ? Visibility.Visible : Visibility.Collapsed;
        }
        void SetStatus(string message, bool error = false) { Get<TextBlock>("Status").Text = message; Get<TextBlock>("Status").Foreground = Brush(error ? "#FFBD9E" : "#B8CAD5"); }
        void Explore(string path, bool select) {
            try { card.Validate(); if (select) Process.Start(new ProcessStartInfo("explorer.exe", "/select,\"" + path + "\"") { UseShellExecute = true }); else Process.Start(new ProcessStartInfo(path) { UseShellExecute = true }); }
            catch (Exception e) { SetStatus(e.Message, true); }
        }
        void BrowseGames() {
            if (busy) return;
            var picker = new OpenFileDialog { Title = "Spiele für deine SD-Karte auswählen", Multiselect = true, Filter = "Spiele und CIA-Pakete|*.nds;*.gba;*.n64;*.v64;*.z64;*.sfc;*.smc;*.nes;*.vb;*.gb;*.gbc;*.cia|Alle Dateien|*.*" };
            if (picker.ShowDialog(window) == true) PreviewImport(picker.FileNames);
        }
        void BrowseCover() {
            if (busy || grid.SelectedItem == null) return;
            var picker = new OpenFileDialog { Title = "Cover auswählen · PNG oder JPG", Filter = "Cover-Bilder|*.png;*.jpg;*.jpeg" };
            if (picker.ShowDialog(window) == true) PutCover((Game)grid.SelectedItem, picker.FileName);
        }
        void PutCover(Game game, string file) {
            try { string status = card.AssignCover(game, file); Reload(game.System + "/" + game.Key); SetStatus(status); }
            catch (Exception e) { SetStatus("Cover nicht geändert: " + e.Message, true); }
        }
        void OnDragOver(object sender, DragEventArgs e) {
            e.Effects = !busy && e.Data.GetDataPresent(DataFormats.FileDrop) ? DragDropEffects.Copy : DragDropEffects.None; e.Handled = true;
        }
        void OnDrop(object sender, DragEventArgs e) {
            e.Handled = true; if (busy || !e.Data.GetDataPresent(DataFormats.FileDrop)) return;
            string[] files = (string[])e.Data.GetData(DataFormats.FileDrop);
            if (files.Length == 1 && Card.IsImage(files[0])) {
                DependencyObject element = e.OriginalSource as DependencyObject;
                while (element != null) {
                    var framework = element as FrameworkElement;
                    if (framework != null && framework.DataContext is Game) { grid.SelectedItem = framework.DataContext; break; }
                    element = element is Visual || element is System.Windows.Media.Media3D.Visual3D ? VisualTreeHelper.GetParent(element) : LogicalTreeHelper.GetParent(element);
                }
                var game = grid.SelectedItem as Game;
                if (game == null) SetStatus("Wähle zuerst das Spiel für dieses Cover.", true); else PutCover(game, files[0]);
            } else PreviewImport(files);
        }
        void PreviewImport(string[] files) {
            if (busy) return;
            try {
                var plans = card.Plan(files); var valid = plans.Where(x => !x.Skip).ToList();
                var dialog = new Window { Title = "Spiele hinzufügen", Owner = window, Width = 760, Height = 520, MinWidth = 620, MinHeight = 400, WindowStartupLocation = WindowStartupLocation.CenterOwner, Background = Brush("#101821"), Foreground = Brush("#EDF3F7"), FontFamily = window.FontFamily, FontSize = 13 };
                // Share button styling, without modifying global Windows settings.
                dialog.Resources = window.Resources;
                var layout = new Grid { Margin = new Thickness(25) };
                layout.RowDefinitions.Add(new RowDefinition { Height = GridLength.Auto }); layout.RowDefinitions.Add(new RowDefinition()); layout.RowDefinitions.Add(new RowDefinition { Height = GridLength.Auto });
                var header = new StackPanel(); header.Children.Add(new TextBlock { Text = valid.Count + " Dateien bereit zum Import", FontSize = 23, FontWeight = FontWeights.SemiBold });
                header.Children.Add(new TextBlock { Text = "Vorhandene Dateien werden übersprungen. CIA-Spiele auf dem 3DS direkt in unserer Library installieren. ZIP/7z zuerst entpacken.", TextWrapping = TextWrapping.Wrap, Foreground = Brush("#94A8B8"), Margin = new Thickness(0, 9, 0, 18) }); layout.Children.Add(header);
                var rows = new StackPanel();
                foreach (var p in plans) {
                    var row = new StackPanel { Margin = new Thickness(0, 0, 0, 13) };
                    row.Children.Add(new TextBlock { Text = p.Name, TextWrapping = TextWrapping.Wrap, FontWeight = FontWeights.SemiBold });
                    row.Children.Add(new TextBlock { Text = p.Destination, TextWrapping = TextWrapping.Wrap, FontSize = 12, Foreground = Brush(p.Skip ? "#FFBD9E" : "#64E8C4"), Margin = new Thickness(0, 4, 0, 0) }); rows.Children.Add(row);
                }
                var scroll = new ScrollViewer { Content = rows, VerticalScrollBarVisibility = ScrollBarVisibility.Auto }; Grid.SetRow(scroll, 1); layout.Children.Add(scroll);
                var actions = new StackPanel { Orientation = Orientation.Horizontal, HorizontalAlignment = HorizontalAlignment.Right, Margin = new Thickness(0, 20, 0, 0) };
                var back = new Button { Content = "Zurück", Margin = new Thickness(0, 0, 12, 0), IsCancel = true }; back.Click += delegate { dialog.DialogResult = false; };
                var start = new Button { Content = "Jetzt importieren", Style = (Style)window.FindResource("Primary"), IsEnabled = valid.Count > 0, IsDefault = true }; start.Click += delegate { dialog.DialogResult = true; };
                actions.Children.Add(back); actions.Children.Add(start); Grid.SetRow(actions, 2); layout.Children.Add(actions); dialog.Content = layout;
                if (dialog.ShowDialog() == true) StartImport(valid, plans.Count - valid.Count);
            } catch (Exception e) { SetStatus("Import nicht gestartet: " + e.Message, true); }
        }
        sealed class BatchResult { public int Copied, Skipped; public bool Cancelled; public List<string> Errors = new List<string>(); public string LastGame; }
        void Busy(bool value) {
            Get<Button>("AutoCovers").IsEnabled = !value;
            busy = value; Get<Button>("AddGames").IsEnabled = !value; Get<Button>("Refresh").IsEnabled = !value;
            Get<TextBox>("Search").IsEnabled = !value; foreach (Button b in nav.Values) b.IsEnabled = !value;
            Get<ProgressBar>("Progress").Visibility = value ? Visibility.Visible : Visibility.Collapsed;
            Get<Button>("CancelImport").Visibility = value ? Visibility.Visible : Visibility.Collapsed; ShowDetails();
        }
        void StartAutoCovers() {
            if (busy) return;
            var snapshot = games.ToList();
            if (!snapshot.Any(g => !File.Exists(g.CoverPath))) { SetStatus("Alle Spiele haben bereits ein Cover. Es wurde nichts geändert."); return; }
            Busy(true); SetStatus("Fehlende Cover werden gesucht · alle Kategorien …");
            worker = new BackgroundWorker { WorkerReportsProgress = true, WorkerSupportsCancellation = true };
            worker.DoWork += delegate(object sender, DoWorkEventArgs e) {
                var w=(BackgroundWorker)sender;
                e.Result = AutoCoverFactory().Run(card,snapshot,(p,s)=>w.ReportProgress(p,s),()=>w.CancellationPending);
            };
            worker.ProgressChanged += delegate(object sender, ProgressChangedEventArgs e) { Get<ProgressBar>("Progress").Value=e.ProgressPercentage;SetStatus((string)e.UserState); };
            worker.RunWorkerCompleted += delegate(object sender, RunWorkerCompletedEventArgs e) {
                Busy(false);Reload(null);
                if(e.Error!=null){SetStatus("Cover-Suche beendet: "+e.Error.Message,true);return;}
                var result=(CoverBatch)e.Result;SetStatus(result.Summary+" · auf dem 3DS neu einlesen.",result.Items.Any(x=>x.Status=="Fehler"));
                if(AutoCoverCompleted!=null){AutoCoverCompleted(result);return;}
                var dialog = new Window { Title="Cover-Suche · Ergebnis",Owner=window,Width=760,Height=540,MinWidth=500,Background=Brush("#101821"),Foreground=Brush("#EDF3F7"),FontFamily=window.FontFamily };
                dialog.Resources=window.Resources;var layout=new DockPanel{Margin=new Thickness(24)};
                var close=new Button{Content="Fertig",HorizontalAlignment=HorizontalAlignment.Right,Margin=new Thickness(0,15,0,0)};close.Click+=delegate{dialog.Close();};DockPanel.SetDock(close,Dock.Bottom);layout.Children.Add(close);
                var heading=new TextBlock{Text=result.Summary,FontSize=17,TextWrapping=TextWrapping.Wrap,Margin=new Thickness(0,0,0,18)};DockPanel.SetDock(heading,Dock.Top);layout.Children.Add(heading);
                var rows=new StackPanel();foreach(var item in result.Items.Where(x=>x.Status!="vorhanden")){
                    rows.Children.Add(new TextBlock{Text=item.Title+" · "+item.Status,FontWeight=FontWeights.SemiBold,TextWrapping=TextWrapping.Wrap,Margin=new Thickness(0,0,0,4)});
                    rows.Children.Add(new TextBlock{Text=item.Detail,TextWrapping=TextWrapping.Wrap,Foreground=Brush(item.Status=="geladen"?"#98E4CF":"#FFBD9E"),Margin=new Thickness(0,0,0,15)});
                }
                layout.Children.Add(new ScrollViewer{Content=rows,VerticalScrollBarVisibility=ScrollBarVisibility.Auto});dialog.Content=layout;dialog.ShowDialog();
            };
            worker.RunWorkerAsync();
        }
        void StartImport(List<ImportPlan> plans, int skipped) {
            Busy(true); Get<ProgressBar>("Progress").Value = 0;
            worker = new BackgroundWorker { WorkerReportsProgress = true, WorkerSupportsCancellation = true };
            worker.DoWork += delegate(object sender, DoWorkEventArgs e) {
                var w = (BackgroundWorker)sender; var result = new BatchResult { Skipped = skipped };
                long total = plans.Sum(p => new FileInfo(p.Source).Length), completed = 0;
                for (int i = 0; i < plans.Count; i++) {
                    ImportPlan plan = plans[i]; if (w.CancellationPending) { result.Cancelled = true; break; }
                    long length = 0;
                    try {
                        length = new FileInfo(plan.Source).Length;
                        card.Import(plan, p => w.ReportProgress((int)Math.Min(100, (completed + p.Done) * 100.0 / Math.Max(1, total)), "Kopiere und prüfe " + (i + 1) + "/" + plans.Count + " · " + p.Name), () => w.CancellationPending);
                        result.Copied++; result.LastGame = plan.System + "/" + plan.Name;
                    } catch (OperationCanceledException) { result.Cancelled = true; break; }
                    catch (Exception ex) { result.Errors.Add(plan.Name + ": " + ex.Message); }
                    completed += length;
                }
                e.Result = result;
            };
            worker.ProgressChanged += delegate(object sender, ProgressChangedEventArgs e) { Get<ProgressBar>("Progress").Value = e.ProgressPercentage; SetStatus((string)e.UserState); };
            worker.RunWorkerCompleted += delegate(object sender, RunWorkerCompletedEventArgs e) {
                Busy(false);
                if (e.Error != null) { Reload(null); SetStatus("Import beendet: " + e.Error.Message, true); return; }
                var result = (BatchResult)e.Result;
                string ciaPath = result.LastGame != null && result.LastGame.StartsWith("cia/") ? plans.First(p => p.Name == result.LastGame.Substring(4)).Target : null;
                if (ciaPath != null) result.LastGame = null;
                category = "all"; Reload(result.LastGame);
                if (ciaPath != null) grid.SelectedItem = games.FirstOrDefault(g => g.FilePath == ciaPath);
                string message = result.Copied + " importiert · " + result.Skipped + " übersprungen" + (result.Cancelled ? " · abgebrochen" : " · fertig");
                if (result.Errors.Count > 0) message += " · " + result.Errors.Count + " Fehler: " + result.Errors[0];
                else message += ". Auf dem 3DS neu einlesen; CIA-Spiele dort mit A installieren.";
                SetStatus(message, result.Errors.Count > 0);
            };
            worker.RunWorkerAsync();
        }
        public void Run() { window.ShowDialog(); }
        internal string CheckAutoCoverButton() {
            string file=card.PathOnCard("roms/gba/AutoCover UI (Europe).gba");Directory.CreateDirectory(Path.GetDirectoryName(file));File.WriteAllBytes(file,new byte[128]);Reload(null);
            byte[] png=AutoCoverTests.Png();AutoCoverFactory=()=>new AutoCovers((url,c)=>{System.Threading.Thread.Sleep(15);return AutoCoverTests.Response(url,png);});
            var previousContext=System.Threading.SynchronizationContext.Current;
            System.Threading.SynchronizationContext.SetSynchronizationContext(new System.Windows.Threading.DispatcherSynchronizationContext(window.Dispatcher));
            var frame=new System.Windows.Threading.DispatcherFrame();CoverBatch result=null;
            AutoCoverCompleted=r=>{result=r;frame.Continue=false;};
            var timeout=new System.Windows.Threading.DispatcherTimer{Interval=TimeSpan.FromSeconds(15)};timeout.Tick+=delegate{frame.Continue=false;};timeout.Start();
            Get<Button>("AutoCovers").RaiseEvent(new RoutedEventArgs(Button.ClickEvent));
            if(!busy||Get<Button>("AutoCovers").IsEnabled||Get<Button>("AddGames").IsEnabled)throw new Exception("Cover button did not enter protected busy state");
            System.Windows.Threading.Dispatcher.PushFrame(frame);timeout.Stop();
            System.Threading.SynchronizationContext.SetSynchronizationContext(previousContext);
            if(result==null||result.Loaded<1||busy||!Get<Button>("AutoCovers").IsEnabled)throw new Exception("Cover button did not complete async import");
            if(!games.Any(g=>g.Key=="AutoCover UI (Europe).gba"&&g.Cover!=null))throw new Exception("Downloaded cover not refreshed in UI");
            window.Close();return "Actual cover button: async search, write-action locking, completion, re-enable and displayed downloaded image";
        }
        internal List<string> CheckUI() {
            var checks = new List<string>();
            foreach (string sys in Card.Views) {
                nav[sys].RaiseEvent(new RoutedEventArgs(Button.ClickEvent));
                if (grid.Items.Count != games.Count(g => g.Category == sys)) throw new Exception("Category filter failed: " + sys);
                checks.Add("UI category button " + sys);
            }
            nav["all"].RaiseEvent(new RoutedEventArgs(Button.ClickEvent));
            Get<TextBox>("Search").Text = "Sample";
            if (grid.Items.Count != 1 || Get<TextBlock>("DetailTitle").Text != "Sample") throw new Exception("Search/detail selection failed");
            checks.Add("UI search and selected game details");
            if (!Get<Button>("ChangeCover").IsEnabled || !Get<Button>("OpenGameFolder").IsEnabled) throw new Exception("ROM actions unavailable");
            checks.Add("UI ROM actions available");
            Get<TextBox>("Search").Text = "ThereIsNoSuchGame123";
            if (grid.Items.Count != 0 || Get<Button>("ChangeCover").IsEnabled || Get<Border>("Empty").Visibility != Visibility.Visible) throw new Exception("Empty search state failed");
            checks.Add("UI empty search state and action disabling");
            Get<TextBox>("Search").Text = "";
            nav["homebrew"].RaiseEvent(new RoutedEventArgs(Button.ClickEvent));
            if (Get<Button>("OpenGameFolder").IsEnabled || Get<Button>("OpenSave").Visibility != Visibility.Collapsed) throw new Exception("Native title actions unsafe");
            checks.Add("UI native title does not expose encrypted save files");
            Game pending = games.FirstOrDefault(g => g.Pending);
            if (pending != null) {
                nav[pending.Category].RaiseEvent(new RoutedEventArgs(Button.ClickEvent));grid.SelectedItem = pending;
                if (!Get<Button>("ChangeCover").IsEnabled || !Get<Button>("OpenGameFolder").IsEnabled || !Get<TextBlock>("DetailNote").Text.Contains("installieren")) throw new Exception("Pending CIA actions failed");
                checks.Add("UI pending CIA allows covers and explains console installation");
            }
            Busy(true);
            if (Get<Button>("AddGames").IsEnabled || Get<Button>("AutoCovers").IsEnabled || Get<Button>("ChangeCover").IsEnabled || nav["all"].IsEnabled) throw new Exception("Import busy state failed");
            checks.Add("UI conflicting write actions disabled during import");
            Busy(false); window.Close(); return checks;
        }
        public void Render(string output) {
            category = "gameboy"; Filter();
            window.Width = 1320; window.Height = 860; window.WindowStartupLocation = WindowStartupLocation.Manual; window.Left = -10000; window.Top = -10000; window.Show(); window.UpdateLayout();
            var content = (FrameworkElement)window.Content;
            var bitmap = new RenderTargetBitmap((int)content.ActualWidth, (int)content.ActualHeight, 96, 96, PixelFormats.Pbgra32); bitmap.Render(content);
            var encoder = new PngBitmapEncoder(); encoder.Frames.Add(BitmapFrame.Create(bitmap));
            using (var file = File.Create(output)) encoder.Save(file);
            window.Close();
        }
    }
    public static class Program {
        [STAThread]
        public static int Main(string[] args) {
            try {
                if (args.Length == 2 && args[0] == "--self-test") return Tests.Run(args[1]);
                if (args.Length == 2 && args[0] == "--cover-live-test") return AutoCoverTests.Live(args[1]);
                var app = new Application { ShutdownMode = ShutdownMode.OnExplicitShutdown };
                if (args.Length == 3 && args[0] == "--preview") {
                    // Read-only preview. Mutating controls are not invoked by this command.
                    new Desktop(new Card(args[1])).Render(args[2]); app.Shutdown(); return 0;
                }
                string root = Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location);
                new Desktop(new Card(root)).Run(); app.Shutdown(); return 0;
            } catch (Exception e) {
                if (args.Length > 0) { File.WriteAllText(Path.Combine(Path.GetTempPath(), "3ds-library-desktop-last-error.txt"), e.ToString()); return 1; }
                MessageBox.Show(e.Message + "\n\nDie App gehört direkt ins Hauptverzeichnis der SD-Karte mit unserer 3DS Library.", "VitarenewFlow Desktop", MessageBoxButton.OK, MessageBoxImage.Information); return 1;
            }
        }
    }
}


