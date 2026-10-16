using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Web.Script.Serialization;
using Drawing=System.Drawing;

namespace LibraryDesktop {
    internal static class AutoCoverTests {
        static readonly List<string> Passed=new List<string>();
        static void Assert(bool ok,string name){if(!ok)throw new Exception("Auto cover FAIL: "+name);Passed.Add(name);}
        internal static byte[] Png(){using(var b=new Drawing.Bitmap(600,400)){
            using(var g=Drawing.Graphics.FromImage(b))using(var brush=new Drawing.Drawing2D.LinearGradientBrush(new Drawing.Point(0,0),new Drawing.Point(600,400),Drawing.Color.Coral,Drawing.Color.Navy))g.FillRectangle(brush,0,0,600,400);
            using(var s=new MemoryStream()){b.Save(s,Drawing.Imaging.ImageFormat.Png);return s.ToArray();}
        }}
        internal static byte[] Response(string url,byte[] png){
            var json=new JavaScriptSerializer();
            if(url.Contains("hax0kartik/3dsdb"))return Encoding.UTF8.GetBytes(json.Serialize(new[]{new{TitleID="0004000012345600",Name="Legend of Zelda, The - The Minish Cap"}}));
            if(url.EndsWith("/master"))return Encoding.UTF8.GetBytes(json.Serialize(new{tree=new[]{new{path="Named_Boxarts",sha=new string('a',40)}}}));
            if(url.EndsWith(new string('a',40))){
                string[] titles=url.Contains("Nintendo_DS")?new[]{"Mario Kart DS (USA).png"}:url.Contains("Game_Boy_Color")?new[]{"Pokemon - Crystal Version (USA).png"}:url.Contains("Nintendo_64")?new[]{"Diddy Kong Racing (Europe) (Rev 1).png"}:new[]{"Legend of Zelda, The - The Minish Cap (Europe).png","BadArt (Europe).png","Super Mario Bros. 3 (Europe).png","AutoCover UI (Europe).png"};
                return Encoding.UTF8.GetBytes(json.Serialize(new{truncated=false,tree=titles.Select(x=>new{path=x}).ToArray()}));
            }
            if(url.Contains("BadArt"))return Encoding.UTF8.GetBytes("not an image");
            if(url.Contains("Nintendo_64")&&Uri.UnescapeDataString(url).Contains("Rev 1"))return Encoding.UTF8.GetBytes("Diddy Kong Racing (Europe).png");
            if(url.Contains("Nintendo_DS")&&url.Contains("raw.githubusercontent.com"))return Encoding.UTF8.GetBytes("version https://git-lfs.github.com/spec/v1\noid sha256:abc\nsize 100\n");
            return png;
        }
        static Game Add(Card card,string system,string name){string path=card.PathOnCard("roms/"+Card.RomFolder(system)+"/"+name);Directory.CreateDirectory(Path.GetDirectoryName(path));File.WriteAllBytes(path,new byte[128]);return card.Scan().Single(x=>x.Key==name);}
        public static List<string> Run(){
            Passed.Clear();string root=Path.Combine(Path.GetTempPath(),"3ds-autocover-tests-"+Guid.NewGuid().ToString("N"));
            Directory.CreateDirectory(Path.Combine(root,"Nintendo 3DS"));Directory.CreateDirectory(Path.Combine(root,"3ds/3ds-library"));File.WriteAllText(Path.Combine(root,"3ds/3ds-library/3ds-library.3dsx"),"test");
            var card=new Card(root,true);byte[] png=Png();
            var gba=Add(card,"gba","Legend of Zelda, The - The Minish Cap (Europe).gba");
            var bad=Add(card,"gba","BadArt (Europe).gba");var unknown=Add(card,"gba","ThisGameDoesNotExist (Europe).gba");
            var nes=Add(card,"nes","Super Mario Bros. 3 (Europe).nes");var ds=Add(card,"ds","0205 - Mario Kart DS (USA).nds");
            var gb=Add(card,"gb","Pokemon - Crystal Version (USA).gbc");var n64=Add(card,"n64","Diddy Kong Racing (Europe) (Rev A).n64");
            string preserve=card.CoverFor("gba",unknown.Key);Directory.CreateDirectory(Path.GetDirectoryName(preserve));File.WriteAllBytes(preserve,png);string preservedHash=Card.Hash(preserve);
            var absent=Add(card,"gba","Unknown Other Game.gba");int catalogRequests=0;
            var engine=new AutoCovers((url,cancel)=>{if(url.Contains("Game_Boy_Advance")&&url.Contains("/git/trees/"))catalogRequests++;return Response(url,png);});
            var report=engine.Run(card,new[]{gba,bad,unknown,nes,ds,gb,n64,absent},(p,s)=>Assert(p>=0&&p<=100,"Bounded cover progress"),()=>false);
            Assert(report.Loaded==5,"Five real pipeline imports across GBA NES DS GBC N64");
            Assert(catalogRequests==2,"One index fetch per platform reused within run");
            Assert(report.Items.Single(x=>x.Title==bad.Title).Status=="Fehler"&&!File.Exists(bad.CoverPath),"Invalid image rejected and later games continue");
            Assert(report.Items.Last().Status=="nicht gefunden"&&!File.Exists(absent.CoverPath),"Unknown game does not receive unrelated artwork");
            Assert(Card.Hash(preserve)==preservedHash,"Existing cover never overwritten");
            using(var image=Drawing.Image.FromFile(gba.CoverPath))Assert(image.Width==256&&image.Height==171,"Automatic aspect-preserving 256px PNG compression");
            Assert(report.Items.Where(x=>x.Status=="geladen").All(x=>x.StoredBytes>0&&x.Url!=null),"Sources and stored bytes recorded");
            Assert(report.Items.Single(x=>x.Title==ds.Title).Url.Contains("media.githubusercontent.com"),"Git LFS image resolved");
            Assert(report.Items.Single(x=>x.Title==n64.Title).Url.Contains("Diddy"),"Image symlink resolved within trusted folder");
            Assert(AutoCovers.Normalize("Dr. Mario 64 (USA).n64")=="drmario64","Title punctuation not mistaken for extension");
            Assert(AutoCovers.Match(new Game{System="nes",Title="Super Mario Bros. 3",Key="game.nes"},new[]{"Named_Boxarts/Super Mario Bros. 2 (Europe).png"})==null,"No fuzzy match across sequels");
            Assert(AutoCovers.Match(ds,new[]{"Named_Boxarts/Mario Kart DS (Europe).png","Named_Boxarts/Mario Kart DS (USA).png"}).Contains("USA"),"Correct region preferred");
            int requests=0;var cancelled=new AutoCovers((url,c)=>{requests++;return Response(url,png);}).Run(card,new[]{absent},(p,s)=>{},()=>true);
            Assert(cancelled.Cancelled&&requests==0&&!File.Exists(absent.CoverPath),"Cancellation before network writes nothing");
            var cancelGame=Add(card,"gba","1001 - Legend of Zelda, The - The Minish Cap (Europe).gba");bool stop=false;
            var midCancel=new AutoCovers((url,c)=>{if(url.Contains("raw.githubusercontent.com"))stop=true;return Response(url,png);}).Run(card,new[]{cancelGame},(p,s)=>{},()=>stop);
            Assert(midCancel.Cancelled&&!File.Exists(cancelGame.CoverPath),"Cancellation after download never commits cover");
            string marker=card.PathOnCard("3ds/3ds-library/3ds-library.3dsx");
            var removed=new AutoCovers((url,c)=>{if(url.Contains("raw.githubusercontent.com"))File.Move(marker,marker+".removed");return Response(url,png);}).Run(card,new[]{cancelGame},(p,s)=>{},()=>false);
            Assert(removed.Items.Single().Status=="Fehler"&&!File.Exists(cancelGame.CoverPath),"Removed SD marker blocks post-download write");File.Move(marker+".removed",marker);
            var offline=new AutoCovers((url,c)=>{throw new IOException("Network unavailable");}).Run(card,new[]{absent},(p,s)=>{},()=>false);
            Assert(offline.Items.Single().Status=="Fehler"&&!File.Exists(absent.CoverPath),"Offline is a visible error without false success");
            var race=Add(card,"gba","AutoCover UI (Europe).gba");
            var racing=new AutoCovers((url,c)=>{if(url.Contains("raw.githubusercontent.com")){Directory.CreateDirectory(Path.GetDirectoryName(race.CoverPath));File.WriteAllBytes(race.CoverPath,png);}return Response(url,png);}).Run(card,new[]{race},(p,s)=>{},()=>false);
            Assert(racing.Items.Single().Status=="vorhanden"&&Card.Hash(race.CoverPath)==Card.Hash(preserve),"Cover added during download preserved");
            Assert(!Directory.GetFiles(Path.GetDirectoryName(gba.CoverPath),"*.partial").Any(),"No partial files after outcomes");
            Directory.CreateDirectory(card.PathOnCard("Nintendo 3DS/"+new string('a',32)+"/"+new string('b',32)+"/title/00040000/12345600/content"));
            var native=card.Scan().Single(x=>x.Native&&x.Key=="0004000012345600");
            var nativeResult=new AutoCovers((url,c)=>Response(url,png)).Run(card,new[]{native},(p,s)=>{},()=>false);
            Assert(nativeResult.Loaded==1&&File.Exists(native.CoverPath),"Unknown installed 3DS title resolved by metadata title ID");
            File.WriteAllText(gba.CoverPath+".thumb.png","cached");File.WriteAllText(gba.CoverPath+".thumb.size","123");string replacement=Path.Combine(root,"replacement.png");File.WriteAllBytes(replacement,png);card.AssignCover(gba,replacement);
            Assert(!File.Exists(gba.CoverPath+".thumb.png")&&!File.Exists(gba.CoverPath+".thumb.size"),"Replacing cover invalidates only its derived console caches");
            return Passed.ToList();
        }
        public static int Live(string report){
            string root=Path.Combine(Path.GetTempPath(),"3ds-cover-live-"+Guid.NewGuid().ToString("N"));Directory.CreateDirectory(Path.Combine(root,"Nintendo 3DS"));Directory.CreateDirectory(Path.Combine(root,"3ds/3ds-library"));File.WriteAllText(Path.Combine(root,"3ds/3ds-library/3ds-library.3dsx"),"fixture");
            var card=new Card(root,true);
            var gba=Add(card,"gba","Legend of Zelda, The - The Minish Cap (Europe).gba");
            var ds=Add(card,"ds","0205 - Mario Kart DS (USA).nds");var n64=Add(card,"n64","Diddy Kong Racing (Europe) (En,Fr,De) (Rev A).n64");
            var unknown=Add(card,"gba","This Game Does Not Exist 987654.gba");
            Directory.CreateDirectory(card.PathOnCard("Nintendo 3DS/"+new string('a',32)+"/"+new string('b',32)+"/title/00040000/00030700/content"));var native=card.Scan().Single(x=>x.Native);
            var result=new AutoCovers().Run(card,new[]{gba,ds,n64,native,unknown},(p,s)=>{},()=>false);
            bool passed=result.Loaded==4&&result.Items.Last().Status=="nicht gefunden";
            File.WriteAllText(report,new JavaScriptSerializer().Serialize(new{passed=passed,root=root,result=result}),Encoding.UTF8);
            if(passed){var app=new System.Windows.Application{ShutdownMode=System.Windows.ShutdownMode.OnExplicitShutdown};new Desktop(card).Render(Path.ChangeExtension(report,".png"));app.Shutdown();}
            return passed?0:1;
        }
    }
}
