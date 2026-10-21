// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 VitarenewFlow contributors
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Web.Script.Serialization;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using System.Windows.Media.Imaging;

namespace VitarenewSetup {
    public sealed class ManifestFile { public string Path;public string Resource;public string SHA256; }
    public sealed class Manifest { public string Version;public ManifestFile[] Files; }
    public sealed class SetupWindow : Window {
        readonly ComboBox drives=new ComboBox(),language=new ComboBox();
        readonly TextBlock heading=new TextBlock(),intro=new TextBlock(),note=new TextBlock();
        readonly TextBox details=new TextBox();readonly Button scan=new Button(),install=new Button();
        readonly IList<PayloadFile> payload;readonly string version;bool busy;bool English {get{return language.SelectedIndex==1;}}
        string T(string de,string en){return English?en:de;}
        static Brush Color(string value){return (Brush)new BrushConverter().ConvertFromString(value);}
        static void Add(Panel panel,UIElement element){panel.Children.Add(element);}
        public SetupWindow(string selectedDrive) {
            var manifest=new JavaScriptSerializer().Deserialize<Manifest>(ReadText("payload.manifest"));version=manifest.Version;payload=new List<PayloadFile>();
            foreach(var f in manifest.Files){byte[] data=ReadBytes(f.Resource);if(Installer.Hash(data)!=f.SHA256)throw new IOException("Embedded package checksum failed: "+f.Path);payload.Add(new PayloadFile{Path=f.Path,Data=data,SHA256=f.SHA256});}
            foreach(string required in new[]{"3ds/3ds-library/3ds-library.3dsx","3ds/3ds-library/3ds-library.smdh","3ds/3ds-library/boot-dispatcher.3dsx","cias/VitarenewFlow.cia","gm9/scripts/VitarenewFlow-Copy.gm9","3DS Library PC.exe"})if(!payload.Any(f=>f.Path==required))throw new IOException("Incomplete embedded package");
            byte[] self=File.ReadAllBytes(Assembly.GetExecutingAssembly().Location);payload.Add(new PayloadFile{Path="VitarenewFlow Setup.exe",Data=self,SHA256=Installer.Hash(self)});
            Title="VitarenewFlow · Setup";Width=820;Height=710;MinWidth=700;MinHeight=620;WindowStartupLocation=WindowStartupLocation.CenterScreen;Background=Color("#101A21");Foreground=Color("#F2F7F8");FontFamily=new FontFamily("Segoe UI");FontSize=14;
            var panel=new StackPanel{Margin=new Thickness(30)};Content=new ScrollViewer{Content=panel,Background=Background,VerticalScrollBarVisibility=ScrollBarVisibility.Auto};
            var top=new DockPanel();language.Items.Add("Deutsch");language.Items.Add("English");language.SelectedIndex=0;language.Width=120;language.HorizontalAlignment=HorizontalAlignment.Right;DockPanel.SetDock(language,Dock.Right);top.Children.Add(language);top.Children.Add(new TextBlock{Text="VITARENEWFLOW",FontSize=16,FontWeight=FontWeights.Bold,Foreground=Color("#45CDB5")});Add(panel,top);
            heading.FontSize=30;heading.FontWeight=FontWeights.SemiBold;heading.Margin=new Thickness(0,24,0,14);Add(panel,heading);intro.TextWrapping=TextWrapping.Wrap;intro.Margin=new Thickness(0,0,0,20);Add(panel,intro);
            var row=new DockPanel();scan.Width=145;scan.Padding=new Thickness(10);scan.Margin=new Thickness(12,0,0,0);DockPanel.SetDock(scan,Dock.Right);row.Children.Add(scan);drives.MinHeight=40;row.Children.Add(drives);Add(panel,row);
            note.TextWrapping=TextWrapping.Wrap;note.Foreground=Color("#ADC0C7");note.Margin=new Thickness(0,16,0,16);Add(panel,note);
            details.Height=245;details.IsReadOnly=true;details.TextWrapping=TextWrapping.Wrap;details.VerticalScrollBarVisibility=ScrollBarVisibility.Auto;details.Background=Color("#1B2A33");details.Foreground=Foreground;details.BorderBrush=Color("#30414B");details.Padding=new Thickness(12);Add(panel,details);
            install.Padding=new Thickness(16,12,16,12);install.Margin=new Thickness(0,20,0,0);install.Background=Color("#45CDB5");install.Foreground=Color("#102B28");install.FontWeight=FontWeights.Bold;Add(panel,install);
            language.SelectionChanged+=delegate{RefreshText();};scan.Click+=delegate{FindDrives(selectedDrive);};drives.SelectionChanged+=delegate{Inspect();};install.Click+=delegate{Install();};Closing+=delegate(object sender,CancelEventArgs e){if(busy)e.Cancel=true;};
            RefreshText();FindDrives(selectedDrive);
        }
        static byte[] ReadBytes(string name){using(var stream=Assembly.GetExecutingAssembly().GetManifestResourceStream(name)){if(stream==null)throw new IOException("Missing resource: "+name);using(var copy=new MemoryStream()){stream.CopyTo(copy);return copy.ToArray();}}}
        static string ReadText(string name){return System.Text.Encoding.UTF8.GetString(ReadBytes(name));}
        void RefreshText(){heading.Text=T("Bereit für deine SD-Karte.","Ready for your SD card.");intro.Text=T("Launcher installieren, aktualisieren oder reparieren. Voraussetzung: Ein bereits eingerichteter 3DS mit Luma-CFW. Dieses Setup installiert keine CFW.","Install, update or repair the launcher. Requires a 3DS already running Luma CFW. This setup does not install CFW.");scan.Content=T("Neu suchen","Scan again");note.Text=T("Paket ","Package ")+version+T(" · Testversion, keine finale 1.0. Spiele, Saves, Cover, boot.firm und Luma-Einstellungen werden nicht ersetzt. Retro-Emulatoradapter werden separat eingerichtet."," · Test build, not final 1.0. Games, saves, covers, boot.firm and Luma settings are preserved. Retro emulator adapters are set up separately.");install.Content=T("Installieren / Reparieren","Install / Repair");}
        void FindDrives(string preferred){
            drives.Items.Clear();foreach(var drive in DriveInfo.GetDrives())try{if(drive.IsReady&&drive.DriveFormat=="FAT32"&&Directory.Exists(Path.Combine(drive.RootDirectory.FullName,"Nintendo 3DS"))&&File.Exists(Path.Combine(drive.RootDirectory.FullName,"luma/config.ini")))drives.Items.Add(drive.RootDirectory.FullName);}catch(IOException){}
            if(preferred!=null){string root=Path.GetPathRoot(Path.GetFullPath(preferred.EndsWith(":")?preferred+"\\":preferred));if(drives.Items.Contains(root))drives.SelectedItem=root;}
            if(drives.SelectedIndex<0&&drives.Items.Count==1)drives.SelectedIndex=0;
            Inspect();
        }
        void Inspect(){install.IsEnabled=!busy&&drives.SelectedItem!=null;if(busy)return;if(drives.SelectedItem==null){details.Text=T("Keine passende SD gewählt. FAT32-Karte mit Nintendo 3DS und luma/config.ini einstecken. Bei mehreren Karten bitte das richtige Laufwerk wählen.","No supported SD selected. Insert a FAT32 card containing Nintendo 3DS and luma/config.ini. If several cards are listed, select the correct drive.");return;}
            try{var target=new Target((string)drives.SelectedItem);details.Text=T("Ziel: ","Target: ")+target.Root+"\r\n\r\n"+String.Join("\r\n",Installer.Components(target));}catch(Exception e){details.Text=e.Message;install.IsEnabled=false;}
        }
        void Install(){
            if(busy||drives.SelectedItem==null)return;Target target;try{target=new Target((string)drives.SelectedItem);}catch(Exception e){details.Text=e.Message;return;}
            string current=Path.GetFullPath(Assembly.GetExecutingAssembly().Location);var files=payload.Where(f=>!(f.Path=="VitarenewFlow Setup.exe"&&Path.GetFullPath(target.PathFor(f.Path)).Equals(current,StringComparison.OrdinalIgnoreCase))).ToList();
            busy=true;install.IsEnabled=scan.IsEnabled=drives.IsEnabled=language.IsEnabled=false;details.Text=T("Paket prüfen, Sicherungen anlegen und Dateien übertragen … SD nicht entfernen.","Verifying package, backing up and transferring files … Do not remove the SD.");
            var worker=new BackgroundWorker();worker.DoWork+=delegate(object sender,DoWorkEventArgs e){e.Result=Installer.Install(target,files);};worker.RunWorkerCompleted+=delegate(object sender,RunWorkerCompletedEventArgs e){busy=false;scan.IsEnabled=drives.IsEnabled=language.IsEnabled=true;install.IsEnabled=true;
                if(e.Error!=null){details.Text=e.Error.Message;return;}var result=(InstallResult)e.Result;details.Text=T("Fertig. Alle Paketdateien per SHA256 geprüft.\r\n\r\nAuf dem 3DS noch /cias/VitarenewFlow.cia mit FBI installieren.\r\n\r\nSicherung: ","Done. All package files SHA256 verified.\r\n\r\nOn the 3DS, install /cias/VitarenewFlow.cia with FBI.\r\n\r\nBackup: ")+result.Backup+"\r\n\r\n"+String.Join("\r\n",result.Components);};worker.RunWorkerAsync();
        }
        public void Render(string path){Width=820;Height=810;WindowStartupLocation=WindowStartupLocation.Manual;Left=-10000;Top=-10000;Show();UpdateLayout();var content=(FrameworkElement)Content;var bitmap=new RenderTargetBitmap((int)content.ActualWidth,(int)content.ActualHeight,96,96,PixelFormats.Pbgra32);bitmap.Render(content);var encoder=new PngBitmapEncoder();encoder.Frames.Add(BitmapFrame.Create(bitmap));using(var output=File.Create(path))encoder.Save(output);Close();}
    }
    public static class SetupProgram {
        [STAThread] public static int Main(string[] args){try{string drive=args.Length==2&&args[0]=="--sd"?args[1]:null;var app=new Application();if(args.Length==2&&args[0]=="--preview"){new SetupWindow(null).Render(args[1]);app.Shutdown();return 0;}app.Run(new SetupWindow(drive));return 0;}catch(Exception e){if(args.Length==2&&args[0]=="--preview"){File.WriteAllText(args[1]+".error.txt",e.ToString());return 1;}MessageBox.Show(e.Message,"VitarenewFlow Setup",MessageBoxButton.OK,MessageBoxImage.Error);return 1;}}
    }
}
