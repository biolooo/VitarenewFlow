// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 VitarenewFlow contributors
using System;using System.IO;using System.Linq;using System.Collections.Generic;using System.Text;
namespace VitarenewSetup {
 public sealed class TestManifest {public TestEntry[] Files;}
 public sealed class TestEntry {public string Path,Resource,SHA256;}
 public static class SetupTests {
  static int checks;
  static void Assert(bool b,string title){if(!b)throw new Exception("FAIL "+title);checks++;}
  static void Reject(Action a,string title){bool rejected=false;try{a();}catch{rejected=true;}Assert(rejected,title);}
  static void Write(string p,string text){Directory.CreateDirectory(Path.GetDirectoryName(p));File.WriteAllText(p,text);}
  static PayloadFile FileData(string p,string data){byte[] b=Encoding.UTF8.GetBytes(data);return new PayloadFile{Path=p,Data=b,SHA256=Installer.Hash(b)};}
  static Target Fixture(string p){Directory.CreateDirectory(Path.Combine(p,"Nintendo 3DS"));Write(Path.Combine(p,"boot.firm"),"FIRM protected");Write(Path.Combine(p,"boot.3dsx"),"original hbl");Write(Path.Combine(p,"luma/config.ini"),"unchanged config");Write(Path.Combine(p,"roms/gba/game.sav"),"save untouched");return new Target(p,true);}
  public static int Main(string[] args){try{
   string root=Path.Combine(Path.GetTempPath(),"VF-installer-tests-"+Guid.NewGuid().ToString("N"));Directory.CreateDirectory(root);
   var t=Fixture(Path.Combine(root,"fresh"));var files=new List<PayloadFile>{FileData("3ds/3ds-library/3ds-library.3dsx","3DSX"+new String('a',64)),FileData("3ds/3ds-library/3ds-library.smdh","SMDH test")};
   string boot=Installer.HashFile(t.PathFor("boot.3dsx")),firm=Installer.HashFile(t.PathFor("boot.firm")),cfg=Installer.HashFile(t.PathFor("luma/config.ini")),save=Installer.HashFile(t.PathFor("roms/gba/game.sav"));
   var result=Installer.Install(t,files);Assert(result.Installed==2,"clean install");Assert(Installer.HashFile(t.PathFor(files[0].Path))==files[0].SHA256,"installed checksum");
   Assert(Installer.Install(t,files).Installed==0,"idempotent update");
   var newFiles=new List<PayloadFile>{FileData(files[0].Path,"3DSX"+new String('b',64)),FileData(files[1].Path,"SMDH updated")};
   result=Installer.Install(t,newFiles);Assert(result.Installed==2&&Installer.HashFile(Path.Combine(result.Backup,files[0].Path))==files[0].SHA256,"update retains original backup");
   using(var locked=new FileStream(t.PathFor(files[1].Path),FileMode.Open,FileAccess.Read,FileShare.Read))Reject(()=>Installer.Install(t,files),"locked second target rolls back first");
   Assert(Installer.HashFile(t.PathFor(newFiles[0].Path))==newFiles[0].SHA256&&Installer.HashFile(t.PathFor(newFiles[1].Path))==newFiles[1].SHA256,"rollback restores complete prior installation");
   Assert(Installer.HashFile(t.PathFor("boot.3dsx"))==boot&&Installer.HashFile(t.PathFor("boot.firm"))==firm&&Installer.HashFile(t.PathFor("luma/config.ini"))==cfg&&Installer.HashFile(t.PathFor("roms/gba/game.sav"))==save,"boot config saves preserved");
   var bad=FileData("../boot.firm","malicious");Reject(()=>Installer.Install(t,new[]{bad}),"escape rejected");bad=FileData("boot.firm","malicious");Reject(()=>Installer.Install(t,new[]{bad}),"boot overwrite rejected");bad=FileData("roms/gba/game.sav","malicious");Reject(()=>Installer.Install(t,new[]{bad}),"save overwrite rejected");
   bad=FileData(files[0].Path,"3DSX"+new String('x',64));bad.SHA256="incorrect";Reject(()=>Installer.Install(t,new[]{bad}),"tampered package rejected");
   Reject(()=>Installer.Install(t,new[]{files[0],files[0]}),"duplicate payload rejected");Reject(()=>t.PathFor("sub/../../escape"),"path traversal rejected");
   Write(t.PathFor("3ds/3ds-library/boot-dispatcher.3dsx"),"3DSX OLD DISPATCHER");Write(t.PathFor("boot.3dsx"),"3DSX OLD DISPATCHER");
   Reject(()=>Installer.Install(t,new[]{FileData("3ds/3ds-library/boot-dispatcher.3dsx","3DSX"+new String('n',64))}),"active autoboot helper change blocked");
   Write(t.PathFor("boot.firm"),"BAD!");Reject(()=>t.Validate(),"invalid CFW root rejected");
   if(args.Length==3){
    var manifest=new System.Web.Script.Serialization.JavaScriptSerializer().Deserialize<TestManifest>(System.IO.File.ReadAllText(System.IO.Path.Combine(args[1],"manifest.json")));
    var full=manifest.Files.Select(e=>new PayloadFile{Path=e.Path,Data=System.IO.File.ReadAllBytes(System.IO.Path.Combine(args[1],e.Path)),SHA256=e.SHA256}).ToList();byte[] setupBytes=System.IO.File.ReadAllBytes(args[2]);full.Add(new PayloadFile{Path="VitarenewFlow Setup.exe",Data=setupBytes,SHA256=Installer.Hash(setupBytes)});
    var fresh=Fixture(System.IO.Path.Combine(root,"full-payload"));var outcome=Installer.Install(fresh,full);Assert(outcome.Installed==full.Count,"complete real payload install");Assert(full.All(f=>Installer.HashFile(fresh.PathFor(f.Path))==f.SHA256),"complete real payload SHA256");Assert(Installer.Install(fresh,full).Installed==0,"complete real payload repeated repair");
   }
   Console.WriteLine("PASS installer: "+checks+" checks. Fixture retained: "+root);if(args.Length>0)System.IO.File.WriteAllText(args[0],"passed="+checks+"\r\nfixture="+root);return 0;
  }catch(Exception e){Console.Error.WriteLine(e);return 1;}}
 }
}
