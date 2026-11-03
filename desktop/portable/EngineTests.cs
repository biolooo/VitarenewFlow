using SkiaSharp;
using System.Text;
using System.Text.Json;
namespace LibraryDesktop;
internal static class EngineTests {
    static readonly List<string> Checks = new();
    static void Check(bool value,string name) { if (!value) throw new Exception(name); Checks.Add(name); }
    static void Reject(Action action,string name) { try { action(); } catch { Check(true,name); return; } throw new Exception("Expected rejection: "+name); }
    internal static void Write(string path,byte[] bytes) { Directory.CreateDirectory(Path.GetDirectoryName(path)); File.WriteAllBytes(path,bytes); }
    internal static Card CreateUiCard() {
        string root=Path.Combine(Path.GetTempPath(),"flow-desktop-test-"+Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(Path.Combine(root,"Nintendo 3DS")); Write(Path.Combine(root,"3ds/3ds-library/3ds-library.3dsx"),new byte[32]);
        Write(Path.Combine(root,"GBA/Sample.gba"),new byte[128]); Write(Path.Combine(root,"NES/Other.nes"),new byte[128]);
        return new Card(root,true);
    }
    static byte[] ImageBytes(int width=600,int height=400) {
        using var bitmap=new SKBitmap(width,height);
        for(int y=0;y<height;y++)for(int x=0;x<width;x++)bitmap.SetPixel(x,y,new SKColor((byte)(x*17+y),(byte)(y*11+x),(byte)(x+y*7),x<5&&y<5?(byte)0:(byte)255));
        using var image=SKImage.FromBitmap(bitmap);using var bytes=image.Encode(SKEncodedImageFormat.Png,100);return bytes.ToArray();
    }
    static byte[] SampleCia() {
        byte[] data=new byte[0x5d40];Array.Copy(BitConverter.GetBytes(0x2020U),data,4);Array.Copy(BitConverter.GetBytes(0x200U),0,data,0x10,4);Array.Copy(BitConverter.GetBytes(0x3ac0U),0,data,0x14,4);Array.Copy(BitConverter.GetBytes(64UL),0,data,0x18,8);data[0x2041]=1;data[0x2043]=4;
        Array.Copy(new byte[]{0,4,0,0,0x12,0x34,0x56,0},0,data,0x2040+0x140+0x4c,8);Array.Copy(Encoding.ASCII.GetBytes("SMDH"),0,data,0x2280+0x400,4);Array.Copy(Encoding.Unicode.GetBytes("Test CIA Game"),0,data,0x2280+0x400+8+0x200,26);return data;
    }
    static void Volumes() {
        var mac=PortableVolume.ParseMac("<plist><dict><key>MountPoint</key><string>/Volumes/My SD</string><key>DeviceIdentifier</key><string>disk9s1</string><key>VolumeUUID</key><string>test-uuid</string><key>FilesystemType</key><string>msdos</string><key>FilesystemName</key><string>MS-DOS FAT32</string><key>WritableVolume</key><true/></dict></plist>");
        Check(mac.Format=="FAT32"&&mac.Mount=="/Volumes/My SD"&&mac.Writable,"Mac actual diskutil key variants and spaces");
        const string mount="55 1 8:1 / /media/user/My\\040SD rw,relatime - vfat /dev/sdb1 rw,errors=remount-ro";
        var linux=PortableVolume.ParseLinux(mount,"/media/user/My SD");Check(linux.Format=="vfat"&&linux.Writable,"Linux mount escapes and rw");
        const string metadata="{\"blockdevices\":[{\"maj:min\":\"8:1\",\"fstype\":\"vfat\",\"fsver\":\"FAT32\",\"uuid\":\"ABCD-1234\"}]}";
        Check(PortableVolume.ApplyLinuxMetadata(linux,metadata).Format=="FAT32","Linux FAT32 positively confirmed by FSVER and UUID");
        Reject(()=>PortableVolume.ApplyLinuxMetadata(linux,metadata.Replace("FAT32","FAT16")),"FAT16 rejected, not assumed from vfat");
        Reject(()=>PortableVolume.ParseLinux(mount.Replace("8:1 / ","8:1 /sub "),"/media/user/My SD"),"Bind subdirectory rejected");
        Check(!PortableVolume.ParseLinux(mount.Replace("/dev/sdb1 rw","/dev/sdb1 ro"),"/media/user/My SD").Writable,"Read-only superblock observed");
        Check(PortableVolume.ParseLinux(mount.Replace("55 1","56 1"),"/media/user/My SD").Identity!=linux.Identity,"Remounted card identity differs");
    }
    static void Inventory(Card card) {
        foreach(var pair in new[]{("GBA",234,".gba"),("NES",222,".nes"),("SNES",214,".sfc"),("Virtual Boy",24,".vb"),("Games/games ds",131,".nds"),("3ds/DaedalusX64/Roms",21,".z64")})for(int i=0;i<pair.Item2;i++)Write(Path.Combine(card.Root,pair.Item1,"Game"+i+pair.Item3),new byte[1]);
        File.Delete(Path.Combine(card.Root,"GBA/Sample.gba"));File.Delete(Path.Combine(card.Root,"NES/Other.nes"));
        for(int i=0;i<198;i++)Directory.CreateDirectory(Path.Combine(card.Root,"Nintendo 3DS",new string('a',32),new string('b',32),"title/00040000",(0x100000+i).ToString("x8")));
        Write(Path.Combine(card.Root,"_nds/runtime.nds"),new byte[1]);
        string outside=Path.Combine(Path.GetTempPath(),"flow-outside-"+Guid.NewGuid().ToString("N"));Directory.CreateDirectory(outside);Write(Path.Combine(outside,"private.gba"),new byte[1]);Directory.CreateSymbolicLink(Path.Combine(card.Root,"GBA/escape"),outside);
        var before=Directory.GetFiles(card.Root,"*",SearchOption.AllDirectories).Where(p=>!p.Contains("escape")).ToDictionary(p=>p,Card.Hash);
        var games=card.Scan();Check(games.Count==1044&&games.Count(g=>!g.Native)==846&&games.Count(g=>g.Native)==198,"Actual folder shape: 198 native plus 846 ROMs");
        Check(!games.Any(g=>g.Key=="runtime.nds"||g.Key=="private.gba"),"Runtime directories and symlink escape excluded");
        Check(before.All(p=>Card.Hash(p.Key)==p.Value),"Discovery leaves all fixture bytes unchanged");
        Reject(()=>card.PathOnCard("GBA/escape/new.gba"),"Symlink parent write rejected");
    }
    static void Operations(Card card) {
        string input=Path.Combine(Path.GetTempPath(),"flow-input-"+Guid.NewGuid().ToString("N"));Directory.CreateDirectory(input);
        string rom=Path.Combine(input,"Import.gba");Write(rom,Enumerable.Range(0,10000).Select(i=>(byte)i).ToArray());var plan=card.Plan(new[]{rom,rom});Check(!plan[0].Skip&&plan[1].Skip,"Duplicate import preview");string hash=card.Import(plan[0],null,null);Check(hash==Card.Hash(rom)&&Card.Hash(plan[0].Target)==hash,"Import atomic copy and SHA256 verification");Check(card.Plan(new[]{rom})[0].Skip,"Existing game never overwritten");
        string large=Path.Combine(input,"Cancel.nds");Write(large,new byte[3000000]);var cancel=card.Plan(new[]{large})[0];bool stop=false;Reject(()=>card.Import(cancel,p=>stop=true,()=>stop),"Cancellation aborts long copy");Check(!File.Exists(cancel.Target)&&!Directory.GetFiles(card.Root,"*.partial",SearchOption.AllDirectories).Any(),"Cancelled partial removed without final file");
        string cia=Path.Combine(input,"Test.cia");Write(cia,SampleCia());var ciaPlan=card.Plan(new[]{cia})[0];Check(!ciaPlan.Skip,"CIA metadata validated before import");card.Import(ciaPlan,null,null);Check(card.Scan().Any(g=>g.Pending&&g.Key=="0004000012345600"),"CIA title ID and pending state retained");
        string image=Path.Combine(input,"cover.png");Write(image,ImageBytes());var game=card.Scan().Single(g=>g.Key=="Import.gba");string save=Path.Combine(Path.GetDirectoryName(game.FilePath),"Import.sav");Write(save,new byte[]{1,2,3,4});string saveHash=Card.Hash(save);
        card.AssignCover(game,image);using(var bitmap=SKBitmap.Decode(game.CoverPath))Check(bitmap.Width==256&&bitmap.Height==171,"Real PNG resize without aspect distortion");string coverHash=Card.Hash(game.CoverPath);card.AssignCover(game,image);Check(Directory.GetFiles(card.PathOnCard("3ds/3ds-library/pc-backups"),"*.png",SearchOption.AllDirectories).Any(p=>Card.Hash(p)==coverHash),"Existing cover backed up and hash preserved");
        Write(image,new byte[]{1,2,3});Reject(()=>card.AssignCover(game,image),"Corrupt image rejected");Check(Card.Hash(game.CoverPath)==coverHash&&Card.Hash(save)==saveHash,"Failed cover preserves cover and save");
        Check(card.SaveCandidates(game).Contains(save),"Saves discovered read-only");
        string marker=card.PathOnCard("3ds/3ds-library/3ds-library.3dsx");File.Move(marker,marker+".removed");Reject(()=>card.Import(card.Plan(new[]{rom})[0],null,null),"Removed card marker blocks mutation");File.Move(marker+".removed",marker);
        Reject(()=>card.PathOnCard("../outside"),"Traversal rejected");Reject(()=>card.CoverFor("gba","bad:name.gba"),"FAT filename contract retained on Unix");
    }

    static void ImagesAndAutoCovers(Card card) {
        string temp=Path.Combine(Path.GetTempPath(),"flow-images-"+Guid.NewGuid().ToString("N"));Directory.CreateDirectory(temp);
        using var bitmap=new SKBitmap(80,40);
        SKColor[] colors={SKColors.Red,SKColors.Lime,SKColors.Blue,SKColors.Yellow};
        for(int y=0;y<40;y++)for(int x=0;x<80;x++)bitmap.SetPixel(x,y,colors[(y>=20?2:0)+(x>=40?1:0)]);
        using var image=SKImage.FromBitmap(bitmap);using var jpeg=image.Encode(SKEncodedImageFormat.Jpeg,100);
        int[][] expected={new[]{0,1,2,3},new[]{1,0,3,2},new[]{3,2,1,0},new[]{2,3,0,1},new[]{0,2,1,3},new[]{2,0,3,1},new[]{3,1,2,0},new[]{1,3,0,2}};
        for(int origin=1;origin<=8;origin++) {
            byte[] exif=new byte[32];Array.Copy(Encoding.ASCII.GetBytes("Exif\0\0II"),exif,8);exif[8]=42;exif[10]=8;exif[14]=1;exif[16]=0x12;exif[17]=1;exif[18]=3;exif[20]=1;exif[24]=(byte)origin;
            byte[] raw=jpeg.ToArray();using var bytes=new MemoryStream();bytes.Write(raw,0,2);bytes.Write(new byte[]{255,225,0,34});bytes.Write(exif);bytes.Write(raw,2,raw.Length-2);
            string source=Path.Combine(temp,"orientation"+origin+".jpg"),target=source+".png";File.WriteAllBytes(source,bytes.ToArray());PortableImages.Convert(source,target);
            using var decoded=SKBitmap.Decode(target);Check(decoded.Width==(origin<5?80:40)&&decoded.Height==(origin<5?40:80),"EXIF dimensions "+origin);
            var corners=new[]{decoded.GetPixel(3,3),decoded.GetPixel(decoded.Width-4,3),decoded.GetPixel(3,decoded.Height-4),decoded.GetPixel(decoded.Width-4,decoded.Height-4)};
            int Nearest(SKColor c)=>Enumerable.Range(0,4).OrderBy(i=>Math.Abs(c.Red-colors[i].Red)+Math.Abs(c.Green-colors[i].Green)+Math.Abs(c.Blue-colors[i].Blue)).First();
            Check(corners.Select(Nearest).SequenceEqual(expected[origin-1]),"EXIF quadrant orientation "+origin);
        }
        string alpha=Path.Combine(temp,"alpha.png"),alphaOut=alpha+".out";Write(alpha,ImageBytes(80,120));PortableImages.Convert(alpha,alphaOut);using(var small=SKBitmap.Decode(alphaOut))Check(small.Width==80&&small.Height==120&&small.GetPixel(0,0).Alpha==0,"No upscale and transparency preserved");
        string rom=card.PathOnCard("roms/gba/Auto Cover (Europe).gba");Write(rom,new byte[100]);var game=card.Scan().Single(g=>g.Key==Path.GetFileName(rom));byte[] art=ImageBytes();
        var engine=new AutoCovers((url,cancel)=>url.EndsWith("/master")?Encoding.UTF8.GetBytes("{\"tree\":[{\"path\":\"Named_Boxarts\",\"sha\":\""+new string('a',40)+"\"}]}"):url.Contains("/git/trees/")?Encoding.UTF8.GetBytes("{\"truncated\":false,\"tree\":[{\"path\":\"Auto Cover (Europe).png\"}]}"):art);
        var batch=engine.Run(card,new[]{game},(p,t)=>{},()=>false);Check(batch.Loaded==1&&File.Exists(game.CoverPath),"Real shared auto-cover matching/download/convert/commit via controlled transport");
        string before=Card.Hash(game.CoverPath);var again=engine.Run(card,new[]{game},(p,t)=>{},()=>false);Check(again.Loaded==0&&Card.Hash(game.CoverPath)==before,"Auto-cover preserves existing artwork");
        var cancelled=new AutoCovers((u,c)=>throw new Exception("Network must not run")).Run(card,new[]{game},(p,t)=>{},()=>true);Check(cancelled.Cancelled,"Auto-cover cancellation before network");
    }

    static void DiscoveryEdges() {
        var card=CreateUiCard();string config=card.PathOnCard("3ds/3ds-library/ds-folders.txt");
        string deep=Path.Combine(card.Root,"CustomDeep");
        for(int i=0;i<10;i++)deep=Path.Combine(deep,"level");Write(Path.Combine(deep,"TooDeep.nds"),new byte[1]);
        Write(config,Encoding.UTF8.GetBytes("_nds\nsdmc:/CustomDeep\nsdmc:/Missing\n"));
        var files=PortableDiscovery.Scan(card);Check(card.DiscoveryNotice.Length!=0&&files.Any(p=>p.EndsWith("Sample.gba")),"Partial deep/missing DS branch does not suppress later automatic roots");
        string longPath=card.PathOnCard("GBA");for(int i=0;i<3;i++)longPath=Path.Combine(longPath,new string('界',60));Write(Path.Combine(longPath,"TooLong.gba"),new byte[1]);
        files=PortableDiscovery.Scan(card);Check(card.DiscoveryNotice.Length!=0&&!files.Any(p=>p.EndsWith("TooLong.gba")),"Native 512-byte UTF8 path budget enforced");
        Directory.Delete(Path.Combine(card.Root,"GBA",new string('界',60)),true);
        string valid=Path.Combine(card.Root,"CustomValid");Write(Path.Combine(valid,"Found.nds"),new byte[1]);
        Write(config,Encoding.UTF8.GetBytes(string.Concat(Enumerable.Repeat("invalid-relative\n",9))+"sdmc:/CustomValid\n"));
        Check(PortableDiscovery.Scan(card).Any(p=>p.EndsWith("Found.nds")),"Invalid DS lines do not consume eight valid-root slots");
        Write(config,Encoding.UTF8.GetBytes("sdmc:/\n"));PortableDiscovery.Scan(card);Check(card.DiscoveryNotice.Length!=0,"Whole-SD DS selection visibly marked partial instead of silent skip");
    }
    internal static int Run(string report) {
        Checks.Clear();try { Volumes();var card=CreateUiCard();Inventory(card);Operations(card);ImagesAndAutoCovers(card);DiscoveryEdges();File.WriteAllText(report,JsonSerializer.Serialize(new{version="0.4.2",passed=true,checks=Checks.Count,results=Checks},new JsonSerializerOptions{WriteIndented=true}));return 0; }
        catch(Exception ex){File.WriteAllText(report,JsonSerializer.Serialize(new{version="0.4.2",passed=false,error=ex.ToString(),results=Checks},new JsonSerializerOptions{WriteIndented=true}));Console.Error.WriteLine(ex);return 1;}
    }
}
