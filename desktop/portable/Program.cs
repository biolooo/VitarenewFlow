using Avalonia;
using Avalonia.Controls;
using Avalonia.Controls.ApplicationLifetimes;
using Avalonia.Headless;
using Avalonia.Media.Imaging;
using Avalonia.Themes.Fluent;
using Avalonia.Threading;
using Avalonia.Styling;
using System.Text.Json;
namespace LibraryDesktop;
internal static class Program {
    internal static string UiReport;
    [STAThread] public static int Main(string[] args) {
        try {
            if (args.Length == 2 && args[0] == "--self-test") return EngineTests.Run(args[1]);
            if (args.Length == 3 && args[0] is "--probe-volume" or "--scan-readonly") {
                string root = args[1]; var card = new Card(root); var snapshot = PortableVolume.Snapshot(root);
                var result = new Dictionary<string,object> { ["valid"] = true, ["format"] = snapshot.Format, ["freeBytes"] = PortableVolume.FreeBytes(root), ["version"] = "0.4.2" };
                if (args[0] == "--scan-readonly") {
                    var files = PortableDiscovery.Scan(card); result["roms"] = files.Count;
                    result["systems"] = files.GroupBy(Card.Detect).ToDictionary(g => g.Key, g => g.Count());
                    result["partial"] = card.DiscoveryNotice.Length != 0;
                }
                if (Path.GetFullPath(args[2]).StartsWith(card.Root, StringComparison.OrdinalIgnoreCase)) throw new IOException("Testbericht darf nicht auf der SD liegen.");
                File.WriteAllText(args[2], JsonSerializer.Serialize(result, new JsonSerializerOptions { WriteIndented = true })); return 0;
            }
            if (args.Length == 2 && args[0] is "--ui-test" or "--ui-test-headed") {
                if (args[0] == "--ui-test-headed") { UiReport=args[1]; return AppBuilder.Configure<FlowApp>().UsePlatformDetect().StartWithClassicDesktopLifetime(Array.Empty<string>()); }
                UiReport = args[1]; return AppBuilder.Configure<FlowApp>().UseHeadless(new AvaloniaHeadlessPlatformOptions { UseHeadlessDrawing = false }).UseSkia().StartWithClassicDesktopLifetime(Array.Empty<string>());
            }
            return AppBuilder.Configure<FlowApp>().UsePlatformDetect().StartWithClassicDesktopLifetime(args);
        } catch (Exception ex) { Console.Error.WriteLine(ex); return 1; }
    }
}
public sealed class FlowApp : Application {
    public override void Initialize() { Styles.Add(new FluentTheme()); RequestedThemeVariant = ThemeVariant.Dark; }
    public override void OnFrameworkInitializationCompleted() {
        if (ApplicationLifetime is IClassicDesktopStyleApplicationLifetime desktop) {
            var type = typeof(FlowApp).Assembly.GetType("LibraryDesktop.DesktopWindow") ?? throw new InvalidOperationException("Desktop UI is missing from this build.");
            var window = (Window)Activator.CreateInstance(type); desktop.MainWindow = window;
            if (Program.UiReport != null) Dispatcher.UIThread.Post(async () => {
                try {
                    var card = EngineTests.CreateUiCard();
                    await (Task)type.GetMethod("SetCardForTestAsync", System.Reflection.BindingFlags.Instance | System.Reflection.BindingFlags.NonPublic).Invoke(window, new object[] { card });
                    await (Task)type.GetMethod("RunUiChecksAsync", System.Reflection.BindingFlags.Instance | System.Reflection.BindingFlags.NonPublic).Invoke(window, null);
                    window.Width = 1320; window.Height = 860; window.UpdateLayout();
                    using var screenshot = new RenderTargetBitmap(new PixelSize(1320,860)); screenshot.Render(window); screenshot.Save(Path.ChangeExtension(Program.UiReport,".png"), PngBitmapEncoderOptions.Default);
                    File.WriteAllText(Program.UiReport, "{\"version\":\"0.4.2\",\"passed\":true}"); desktop.Shutdown(0);
                } catch (Exception ex) { File.WriteAllText(Program.UiReport, JsonSerializer.Serialize(new { passed=false,error=ex.ToString() })); desktop.Shutdown(1); }
            });
        }
        base.OnFrameworkInitializationCompleted();
    }
}
