using System;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Net;
using System.Text;
using System.Text.RegularExpressions;
using System.Web.Script.Serialization;

namespace LibraryDesktop {
    public sealed class CoverExistsException : IOException { public CoverExistsException() : base("Vorhandenes Cover bleibt erhalten.") { } }
    public sealed class CoverOutcome { public string Title, Status, Detail, Url; public long DownloadBytes, StoredBytes; }
    public sealed class CoverBatch {
        public readonly List<CoverOutcome> Items = new List<CoverOutcome>();
        public bool Cancelled;
        public int Loaded { get { return Items.Count(x => x.Status == "geladen"); } }
        public string Summary { get { return Loaded + " geladen · " + Items.Count(x => x.Status == "vorhanden") + " vorhanden · " + Items.Count(x => x.Status == "nicht gefunden") + " nicht gefunden · " + Items.Count(x => x.Status == "Fehler") + " Fehler" + (Cancelled ? " · abgebrochen" : " · fertig"); } }
    }
    public sealed class AutoCovers {
        readonly Func<string, Func<bool>, byte[]> fetch;
        readonly Dictionary<string, List<string>> catalogs = new Dictionary<string, List<string>>();
        readonly Dictionary<string, string> catalogErrors = new Dictionary<string, string>();
        readonly Dictionary<string,string> nativeNames=new Dictionary<string,string>(StringComparer.OrdinalIgnoreCase);
        readonly HashSet<string> nativeRegions=new HashSet<string>();
        static readonly Dictionary<string, string> Repos = new Dictionary<string, string> {
            {"3ds","Nintendo_-_Nintendo_3DS"},{"ds","Nintendo_-_Nintendo_DS"},{"gba","Nintendo_-_Game_Boy_Advance"},
            {"gb","Nintendo_-_Game_Boy"},{"gbc","Nintendo_-_Game_Boy_Color"},{"n64","Nintendo_-_Nintendo_64"},
            {"snes","Nintendo_-_Super_Nintendo_Entertainment_System"},{"nes","Nintendo_-_Nintendo_Entertainment_System"},{"vb","Nintendo_-_Virtual_Boy"}
        };
        // Names of the supplied retail test packages remain identifiable after CIA cleanup.
        static readonly Dictionary<string,string> NativeTitles = new Dictionary<string,string>(StringComparer.OrdinalIgnoreCase) {
            {"000400000012BE00","Harvest Moon - The Lost Valley"},{"000400000014D900","Gravity Falls - Legend of the Gnome Gemulets"},
            {"000400000012CA00","Mario vs Donkey Kong - Tipping Stars"},{"000400000012CB00","Shovel Knight"},
            {"000400000007C800","Mario Tennis Open"},{"00040000001C4D00","Mario Party - The Top 100"},{"0004000000053F00","Super Mario 3D Land"}
        };
        public AutoCovers() : this(Get) { }
        internal AutoCovers(Func<string, Func<bool>, byte[]> transport) { fetch = transport; }
        static void Check(Func<bool> cancel) { if (cancel()) throw new OperationCanceledException(); }
        static bool Allowed(Uri uri) { return uri.Scheme == "https" && new[] { "api.github.com", "raw.githubusercontent.com", "media.githubusercontent.com" }.Contains(uri.Host) && uri.IsDefaultPort; }
        static byte[] Get(string url, Func<bool> cancel) {
            Check(cancel); var uri = new Uri(url); if (!Allowed(uri)) throw new IOException("Nicht unterstützte Cover-Quelle.");
            ServicePointManager.SecurityProtocol |= SecurityProtocolType.Tls12;
            var request = (HttpWebRequest)WebRequest.Create(uri); request.UserAgent = "3DS-Library-Desktop/0.3.0"; request.Timeout = 12000; request.ReadWriteTimeout = 1500; request.AllowAutoRedirect = false;
            // Abort even while DNS/headers/read are pending, so cancellation stays responsive.
            using (var timer = new System.Threading.Timer(delegate { if (cancel()) request.Abort(); }, null, 100, 100)) {
                try {
                    using (var response = (HttpWebResponse)request.GetResponse()) {
                        if (response.StatusCode != HttpStatusCode.OK) throw new IOException("Cover-Server: " + (int)response.StatusCode);
                        int limit = uri.Host == "api.github.com" ? 16 * 1024 * 1024 : 8 * 1024 * 1024;
                        if (response.ContentLength > limit) throw new IOException("Cover-Antwort zu groß.");
                        using (var stream = response.GetResponseStream()) using (var output = new MemoryStream()) {
                            byte[] buffer = new byte[16384]; int n;
                            while ((n = stream.Read(buffer, 0, buffer.Length)) > 0) { Check(cancel); if (output.Length + n > limit) throw new IOException("Cover-Antwort zu groß."); output.Write(buffer, 0, n); }
                            Check(cancel); return output.ToArray();
                        }
                    }
                } catch (WebException) { Check(cancel); throw; }
            }
        }
        internal static string Normalize(string text) {
            text = Regex.Replace(Path.GetFileName(text ?? ""), @"\.(png|cia|nds|gba|gbc|gb|nes|sfc|smc|n64|v64|z64|vb)$", "", RegexOptions.IgnoreCase);
            text = Regex.Replace(text, @"^\d{4,5}(?:\s*-\s*|_)", "");
            text = Regex.Replace(text, @"\([^)]*\)|\[[^]]*\]", "").Trim();
            text = Regex.Replace(text, @",\s*The$", "", RegexOptions.IgnoreCase);
            text = Regex.Replace(text, @"^The\s+", "", RegexOptions.IgnoreCase);
            text = text.Normalize(NormalizationForm.FormD);
            text = Regex.Replace(text.ToLowerInvariant(), @"[^a-z0-9]", "");
            return text == "shovelknighttreasuretrove" ? "shovelknight" : text;
        }
        static bool SafeImagePath(string path) { return path.StartsWith("Named_Boxarts/", StringComparison.Ordinal) && path.EndsWith(".png", StringComparison.OrdinalIgnoreCase) && path.Substring(14).IndexOfAny(new[] {'/','\\','\r','\n'}) < 0 && !path.Contains(".."); }
        List<string> Catalog(string repo, Func<bool> cancel) {
            List<string> cached; if (catalogs.TryGetValue(repo, out cached)) return cached;
            string error; if (catalogErrors.TryGetValue(repo, out error)) throw new IOException(error);
            try {
                var json = new JavaScriptSerializer { MaxJsonLength = 16 * 1024 * 1024 };
                string api = "https://api.github.com/repos/libretro-thumbnails/" + repo + "/git/trees/";
                var root = (Dictionary<string,object>)json.DeserializeObject(Encoding.UTF8.GetString(fetch(api + "master", cancel)));
                var folder = ((object[])root["tree"]).Cast<Dictionary<string,object>>().First(x => (string)x["path"] == "Named_Boxarts");
                string sha = (string)folder["sha"]; if (!Regex.IsMatch(sha, "^[a-f0-9]{40}$")) throw new IOException("Ungültiger Cover-Index.");
                var tree = (Dictionary<string,object>)json.DeserializeObject(Encoding.UTF8.GetString(fetch(api + sha, cancel)));
                if (tree.ContainsKey("truncated") && (bool)tree["truncated"]) throw new IOException("Cover-Index ist unvollständig.");
                cached = ((object[])tree["tree"]).Cast<Dictionary<string,object>>().Select(x => "Named_Boxarts/" + (string)x["path"]).Where(SafeImagePath).ToList();
                catalogs[repo] = cached; return cached;
            } catch (OperationCanceledException) { throw; }
            catch (Exception ex) { catalogErrors[repo] = "Cover-Index nicht erreichbar: " + ex.Message; throw new IOException(catalogErrors[repo]); }
        }
        internal static string Match(Game game, IEnumerable<string> names) {
            string title = game.Title, known;
            if (game.Native && NativeTitles.TryGetValue(game.Key, out known)) title = known;
            var keys = new[] { Normalize(title), game.Native ? "" : Normalize(game.Key) }.Where(x => x.Length > 0).ToArray();
            string source = (game.Title + " " + game.Key).ToLowerInvariant();
            var candidates = names.Where(SafeImagePath).Where(x => keys.Contains(Normalize(x.Substring(14))))
                .Where(x => !Regex.IsMatch(x, @"\((?:Proto|Beta|Demo|Sample)", RegexOptions.IgnoreCase) || Regex.IsMatch(source, "proto|beta|demo|sample")).ToList();
            string region = source.Contains("usa") ? "USA" : source.Contains("japan") ? "Japan" : "Europe";
            return candidates.OrderBy(x => x.Contains("(" + region) ? 0 : region == "Europe" && x.Contains("(Germany") ? 1 : x.Contains("(World") ? 2 : x.Contains("(USA") ? 3 : 4).ThenBy(x => x, StringComparer.Ordinal).FirstOrDefault();
        }
        string NativeName(Game game,Func<bool> cancel){
            string name;if(NativeTitles.TryGetValue(game.Key,out name))return name;
            if(!game.Title.StartsWith("3DS-Titel",StringComparison.Ordinal)&&!Regex.IsMatch(game.Title,"^[a-fA-F0-9]{16}$"))return game.Title;
            foreach(string region in new[]{"GB","US","JP","KR","TW"}){
                if(nativeNames.TryGetValue(game.Key,out name))return name;
                if(nativeRegions.Contains(region))continue;
                byte[] bytes=fetch("https://raw.githubusercontent.com/hax0kartik/3dsdb/master/jsons/list_"+region+".json",cancel);
                var entries=(object[])new JavaScriptSerializer{MaxJsonLength=16*1024*1024}.DeserializeObject(Encoding.UTF8.GetString(bytes));
                foreach(var entry in entries.Cast<Dictionary<string,object>>()){
                    object id,label;if(entry.TryGetValue("TitleID",out id)&&entry.TryGetValue("Name",out label)&&id is string&&label is string&&Regex.IsMatch((string)id,"^[a-fA-F0-9]{16}$")&&((string)label).Length<=200)
                        nativeNames[(string)id]=(string)label+(region=="US"?" (USA)":region=="JP"?" (Japan)":region=="GB"?" (Europe)":"");
                }
                nativeRegions.Add(region);
            }
            return nativeNames.TryGetValue(game.Key,out name)?name:game.Title;
        }
        byte[] Image(string repo, ref string path, out string url, Func<bool> cancel) {
            for (int i = 0; i < 4; i++) {
                url = "https://raw.githubusercontent.com/libretro-thumbnails/" + repo + "/master/" + Uri.EscapeDataString(path).Replace("%2F", "/");
                byte[] bytes = fetch(url, cancel);
                if (bytes.Length < 512) {
                    string link = Encoding.UTF8.GetString(bytes).Trim();
                    if (link.EndsWith(".png", StringComparison.OrdinalIgnoreCase)) { string resolved = "Named_Boxarts/" + link; if (!SafeImagePath(resolved)) throw new IOException("Ungültiger Bildverweis."); path = resolved; continue; }
                    if (link.StartsWith("version https://git-lfs.github.com/spec/v1", StringComparison.Ordinal)) { url = url.Replace("raw.githubusercontent.com/", "media.githubusercontent.com/media/"); return fetch(url, cancel); }
                }
                return bytes;
            }
            throw new IOException("Zu viele Bildverweise.");
        }
        public CoverBatch Run(Card card, IEnumerable<Game> collection, Action<int,string> progress, Func<bool> cancel) {
            var batch = new CoverBatch(); var games = collection.ToList();
            for (int i = 0; i < games.Count; i++) {
                if (cancel()) { batch.Cancelled = true; break; }
                var game = games[i]; var item = new CoverOutcome { Title = game.Title };
                string temp = null;
                try {
                    card.Validate(); progress(i * 100 / Math.Max(1,games.Count), "Cover " + (i+1) + "/" + games.Count + " · " + game.Title);
                    if (File.Exists(game.CoverPath)) { item.Status = "vorhanden"; item.Detail = "Unverändert"; }
                    else {
                        string system = game.System == "gb" && game.Key.EndsWith(".gbc",StringComparison.OrdinalIgnoreCase) ? "gbc" : game.System, repo;
                        if (game.Homebrew || !Repos.TryGetValue(system, out repo)) { item.Status = "nicht gefunden"; item.Detail = "Keine Boxart-Quelle für diese Homebrew-App/Plattform; manuell zuordnen."; }
                        else {
                            Game matchGame=game;
                            if(game.Native){string name=NativeName(game,cancel);matchGame=new Game{System=game.System,Key=game.Key,Title=name};item.Title=name;}
                            string path = Match(matchGame, Catalog(repo,cancel));
                            if (path == null) { item.Status = "nicht gefunden"; item.Detail = "Kein passender Titelname im Cover-Bestand; manuell zuordnen."; }
                            else {
                                string url; byte[] bytes = Image(repo,ref path,out url,cancel); Check(cancel);
                                temp = Path.Combine(Path.GetTempPath(),"3ds-cover-"+Guid.NewGuid().ToString("N")+".png"); File.WriteAllBytes(temp,bytes); Check(cancel);
                                card.AssignCover(game,temp,true); item.Status="geladen";item.Url=url;item.DownloadBytes=bytes.Length;item.StoredBytes=new FileInfo(game.CoverPath).Length;
                                item.Detail="PNG · max.256 px · "+Card.Bytes(item.DownloadBytes)+" → "+Card.Bytes(item.StoredBytes)+" · "+path.Substring(14);
                            }
                        }
                    }
                } catch (CoverExistsException) { item.Status="vorhanden";item.Detail="Während der Suche hinzugefügt; unverändert."; }
                catch (OperationCanceledException) { batch.Cancelled=true;break; }
                catch (Exception ex) { item.Status="Fehler";item.Detail=ex.Message; }
                finally { if(temp!=null&&File.Exists(temp))File.Delete(temp); }
                batch.Items.Add(item);
            }
            progress(100,batch.Summary);return batch;
        }
    }
}

