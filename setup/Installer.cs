// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 VitarenewFlow contributors
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Security.Cryptography;
using System.Text;

namespace VitarenewSetup {
    public sealed class PayloadFile { public string Path; public byte[] Data; public string SHA256; }
    public sealed class InstallResult { public string Backup; public int Installed; public string[] Components; }
    public sealed class Target {
        public readonly string Root;
        readonly uint serial; readonly bool sandbox;
        [DllImport("kernel32.dll", CharSet=CharSet.Unicode, SetLastError=true)]
        static extern bool GetVolumeInformation(string root,StringBuilder name,int nameSize,out uint number,out uint max,out uint flags,StringBuilder fs,int fsSize);
        public Target(string path) : this(path,false) { }
        internal Target(string path,bool testSandbox) {
            Root=System.IO.Path.GetFullPath(path).TrimEnd('\\','/')+System.IO.Path.DirectorySeparatorChar;sandbox=testSandbox;
            if(!sandbox&&!Root.Equals(System.IO.Path.GetPathRoot(Root),StringComparison.OrdinalIgnoreCase))throw new IOException("Bitte das Hauptverzeichnis der SD-Karte wählen. / Choose the SD root.");
            serial=Serial();Validate();
        }
        uint Serial() { uint n,max,flags;var fs=new StringBuilder(32);
            if(!GetVolumeInformation(System.IO.Path.GetPathRoot(Root),null,0,out n,out max,out flags,fs,fs.Capacity))throw new IOException("SD-Karte nicht verfügbar / SD unavailable.");
            if(!sandbox&&fs.ToString()!="FAT32")throw new IOException("FAT32-SD-Karte erforderlich / FAT32 SD required.");return n;
        }
        public void Validate() {
            if((File.GetAttributes(Root)&FileAttributes.ReparsePoint)!=0)throw new IOException("SD-Verknüpfung nicht erlaubt / Linked SD root is not allowed.");
            if(serial!=Serial()||!Directory.Exists(PathFor("Nintendo 3DS"))||!File.Exists(PathFor("luma/config.ini")))throw new IOException("3DS-CFW-SD fehlt oder wurde gewechselt / 3DS CFW SD missing or replaced.");
            string firm=PathFor("boot.firm");if(!File.Exists(firm))throw new IOException("Luma boot.firm fehlt / Luma boot.firm missing.");
            using(var f=File.OpenRead(firm)){byte[] magic=new byte[4];if(f.Read(magic,0,4)!=4||Encoding.ASCII.GetString(magic)!="FIRM")throw new IOException("Ungültige boot.firm / Invalid boot.firm.");}
        }
        public string PathFor(string relative) {
            if(String.IsNullOrEmpty(relative)||System.IO.Path.IsPathRooted(relative)||relative.Split('/','\\').Any(s=>s==".."||s.Contains(":")))throw new IOException("Ungültiger Paketpfad / Invalid package path.");
            string path=System.IO.Path.GetFullPath(System.IO.Path.Combine(Root,relative.Replace('/',System.IO.Path.DirectorySeparatorChar)));
            if(!path.StartsWith(Root,StringComparison.OrdinalIgnoreCase))throw new IOException("Pfad außerhalb der SD / Path escapes SD.");
            for(string p=path;p!=null&&p.StartsWith(Root,StringComparison.OrdinalIgnoreCase);p=System.IO.Path.GetDirectoryName(p.TrimEnd('\\','/'))){
                try{if((File.GetAttributes(p)&FileAttributes.ReparsePoint)!=0)throw new IOException("Verknüpfungen sind als Ziel nicht erlaubt / Linked targets are not allowed.");}
                catch(FileNotFoundException){}catch(DirectoryNotFoundException){}
            }
            return path;
        }
    }
    public static class Installer {
        public static readonly string[] Allowed={"3ds/3ds-library/3ds-library.3dsx","3ds/3ds-library/3ds-library.smdh","3ds/3ds-library/boot-dispatcher.3dsx","cias/VitarenewFlow.cia","gm9/scripts/VitarenewFlow-Copy.gm9","3DS Library PC.exe","VitarenewFlow Setup.exe","Library-PC/INSTALL-DE.txt","Library-PC/INSTALL-EN.txt","Library-PC/licenses/3ds-template-MIT.txt","Library-PC/licenses/citro2d.txt","Library-PC/licenses/citro3d.txt","Library-PC/licenses/Inter-OFL.txt","Library-PC/licenses/libctru.txt","Library-PC/licenses/libpng.txt","Library-PC/licenses/NOTICES.md","Library-PC/licenses/PROJECT-LICENSE.md","Library-PC/licenses/VitarenewFlow-GPL-3.0.txt","Library-PC/licenses/zlib.txt"};
        public static string Hash(byte[] data){using(var sha=SHA256.Create())return BitConverter.ToString(sha.ComputeHash(data)).Replace("-","").ToLowerInvariant();}
        public static string HashFile(string path){using(var f=File.OpenRead(path))using(var sha=SHA256.Create())return BitConverter.ToString(sha.ComputeHash(f)).Replace("-","").ToLowerInvariant();}
        static void Verify(PayloadFile file) {
            if(file==null||!Allowed.Contains(file.Path,StringComparer.OrdinalIgnoreCase)||file.Path.Contains("\\")||file.Data==null||Hash(file.Data)!=file.SHA256)throw new IOException("Paketprüfung fehlgeschlagen / Package verification failed.");
            if(file.Path.EndsWith(".3dsx",StringComparison.OrdinalIgnoreCase)&&(file.Data.Length<32||Encoding.ASCII.GetString(file.Data,0,4)!="3DSX"))throw new IOException("Ungültige 3DSX / Invalid 3DSX.");
        }
        sealed class Operation { public PayloadFile File; public string Target,Backup,Staged; public bool Moved,Committed; }
        public static InstallResult Install(Target target,IList<PayloadFile> files) {
            if(files.Count==0||files.Select(f=>f.Path).Distinct(StringComparer.OrdinalIgnoreCase).Count()!=files.Count)throw new IOException("Leeres oder mehrdeutiges Paket / Empty or duplicate package.");
            foreach(var file in files)Verify(file);target.Validate();
            string helper=target.PathFor("3ds/3ds-library/boot-dispatcher.3dsx"),boot=target.PathFor("boot.3dsx");
            var next=files.FirstOrDefault(f=>f.Path=="3ds/3ds-library/boot-dispatcher.3dsx");
            if(next!=null&&File.Exists(helper)&&File.Exists(boot)&&HashFile(helper)==HashFile(boot)&&HashFile(helper)!=next.SHA256)
                throw new IOException("Autostart vor diesem Update deaktivieren / Disable autoboot before this update.");
            string transaction=DateTime.UtcNow.ToString("yyyyMMdd-HHmmss")+"-"+Guid.NewGuid().ToString("N");
            string backup=target.PathFor("Library-PC/install-backups/"+transaction),stage=target.PathFor("Library-PC/install-staging/"+transaction);
            var ops=files.Select(f=>new Operation{File=f,Target=target.PathFor(f.Path),Backup=target.PathFor("Library-PC/install-backups/"+transaction+"/"+f.Path),Staged=target.PathFor("Library-PC/install-staging/"+transaction+"/"+f.Path)}).ToList();
            foreach(var op in ops){target.Validate();Directory.CreateDirectory(Path.GetDirectoryName(op.Staged));File.WriteAllBytes(op.Staged,op.File.Data);if(HashFile(op.Staged)!=op.File.SHA256)throw new IOException("Staging-Prüfung fehlgeschlagen / Staging verification failed.");}
            Directory.CreateDirectory(backup);File.WriteAllText(Path.Combine(backup,"TRANSACTION.txt"),"VitarenewFlow installer\r\n"+String.Join("\r\n",files.Select(f=>f.Path+" "+f.SHA256)),Encoding.UTF8);
            try {
                foreach(var op in ops){
                    target.Validate();target.PathFor(op.File.Path);Directory.CreateDirectory(Path.GetDirectoryName(op.Target));
                    if(File.Exists(op.Target)&&HashFile(op.Target)==op.File.SHA256)continue;
                    if(File.Exists(op.Target)){Directory.CreateDirectory(Path.GetDirectoryName(op.Backup));string old=HashFile(op.Target);File.Move(op.Target,op.Backup);op.Moved=true;if(HashFile(op.Backup)!=old)throw new IOException("Backup-Prüfung fehlgeschlagen / Backup verification failed.");}
                    File.Move(op.Staged,op.Target);op.Committed=true;if(HashFile(op.Target)!=op.File.SHA256)throw new IOException("Zielprüfung fehlgeschlagen / Target verification failed.");
                }
                target.Validate();foreach(var file in files)if(HashFile(target.PathFor(file.Path))!=file.SHA256)throw new IOException("Abschlussprüfung fehlgeschlagen / Final verification failed.");
                foreach(string folder in new[]{"roms/nds","roms/gba","roms/gb","roms/n64","roms/snes","roms/nes","roms/vb","3ds/3ds-library/covers"})Directory.CreateDirectory(target.PathFor(folder));
                File.WriteAllText(Path.Combine(backup,"SUCCESS.txt"),"All package files SHA256 verified. Boot/config/games/saves excluded.\r\n",Encoding.UTF8);
                return new InstallResult{Backup=backup,Installed=ops.Count(o=>o.Committed),Components=Components(target)};
            }catch(Exception failure){
                var rollback=new List<string>();
                foreach(var op in ops.AsEnumerable().Reverse())try{
                    if(!op.Committed&&!op.Moved)continue;target.Validate();target.PathFor(op.File.Path);
                    if(File.Exists(op.Target)){if(HashFile(op.Target)!=op.File.SHA256)throw new IOException("Target changed externally");File.Delete(op.Target);}
                    if(op.Moved)File.Move(op.Backup,op.Target);
                }catch(Exception e){rollback.Add(op.File.Path+": "+e.Message);}
                throw new IOException(failure.Message+"\r\n"+(rollback.Count==0?"Zurückgesetzt / Rolled back.":"Sicherungen behalten / Backups retained: "+backup+"\r\n"+String.Join("\r\n",rollback)),failure);
            }
        }
        public static string[] Components(Target target){
            target.Validate();var list=new List<string>();
            foreach(string rel in new[]{"3ds/mgba-library/mgba-library.3dsx","3ds/n64-library/n64-library.3dsx","3ds/snes-library/snes-library.3dsx","3ds/nes-library/nes-library.3dsx","3ds/vb-library/vb-library.3dsx","luma/payloads/GodMode9.firm"})list.Add((File.Exists(target.PathFor(rel))?"OK      ":"FEHLT / MISSING  ")+rel);
            list.Add("CIA: nach Installation auf SD noch mit FBI installieren / Install the CIA with FBI on console.");
            return list.ToArray();
        }
    }
}
