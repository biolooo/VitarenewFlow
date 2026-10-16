using System;
using System.Collections.Generic;
using System.Drawing;
using System.Drawing.Imaging;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
namespace LibraryDesktop {
    internal static class CoverCompression {
        sealed class Bin { public int Key,N,R,G,B,A,Index; public int Value(int axis){return axis==0?R/N:axis==1?G/N:axis==2?B/N:A/N;} }
        sealed class Box { public List<Bin> Bins;public int Axis,Score;public Box(List<Bin> bins){Bins=bins;for(int a=0;a<4;a++){int range=bins.Max(x=>x.Value(a))-bins.Min(x=>x.Value(a));if(range>Score){Score=range;Axis=a;}}if(bins.Count<2)Score=-1;} }
        static int Key(byte[] pixels,int i){return pixels[i+3]==0?0:(pixels[i+2]>>3)|((pixels[i+1]>>3)<<5)|((pixels[i]>>3)<<10)|((pixels[i+3]>>4)<<15);}
        public static void Save(Bitmap bitmap,Stream output){
            byte[] rgba;using(var memory=new MemoryStream()){bitmap.Save(memory,ImageFormat.Png);rgba=memory.ToArray();}
            var data=bitmap.LockBits(new Rectangle(0,0,bitmap.Width,bitmap.Height),ImageLockMode.ReadOnly,PixelFormat.Format32bppArgb);
            byte[] pixels=new byte[data.Stride*bitmap.Height];try{Marshal.Copy(data.Scan0,pixels,0,pixels.Length);}finally{bitmap.UnlockBits(data);}
            var bins=new Dictionary<int,Bin>();bool transparent=false;
            for(int y=0;y<bitmap.Height;y++)for(int x=0;x<bitmap.Width;x++){int i=y*data.Stride+x*4,k=Key(pixels,i);if(k==0){transparent=true;continue;}Bin b;if(!bins.TryGetValue(k,out b)){b=new Bin{Key=k};bins.Add(k,b);}b.N++;b.B+=pixels[i];b.G+=pixels[i+1];b.R+=pixels[i+2];b.A+=pixels[i+3];}
            var boxes=new List<Box>();if(bins.Count>0)boxes.Add(new Box(bins.Values.ToList()));
            while(boxes.Count<(transparent?255:256)){
                var box=boxes.OrderByDescending(x=>x.Score).FirstOrDefault();if(box==null||box.Score<0)break;
                var sorted=box.Bins.OrderBy(x=>x.Value(box.Axis)).ToList();int total=sorted.Sum(x=>x.N),sum=0,split=1;
                for(int i=0;i<sorted.Count-1;i++){sum+=sorted[i].N;split=i+1;if(sum>=total/2)break;}
                boxes.Remove(box);boxes.Add(new Box(sorted.Take(split).ToList()));boxes.Add(new Box(sorted.Skip(split).ToList()));
            }
            using(var indexed=new Bitmap(bitmap.Width,bitmap.Height,PixelFormat.Format8bppIndexed)){
                var palette=indexed.Palette;int offset=transparent?1:0;if(transparent)palette.Entries[0]=Color.Transparent;
                for(int i=0;i<boxes.Count;i++){var list=boxes[i].Bins;int n=list.Sum(x=>x.N);palette.Entries[i+offset]=Color.FromArgb(list.Sum(x=>x.A)/n,list.Sum(x=>x.R)/n,list.Sum(x=>x.G)/n,list.Sum(x=>x.B)/n);foreach(var b in list)b.Index=i+offset;}
                indexed.Palette=palette;var target=indexed.LockBits(new Rectangle(0,0,indexed.Width,indexed.Height),ImageLockMode.WriteOnly,PixelFormat.Format8bppIndexed);byte[] indices=new byte[target.Stride*indexed.Height];
                for(int y=0;y<bitmap.Height;y++)for(int x=0;x<bitmap.Width;x++){int k=Key(pixels,y*data.Stride+x*4);indices[y*target.Stride+x]=(byte)(k==0?0:bins[k].Index);}
                try{Marshal.Copy(indices,0,target.Scan0,indices.Length);}finally{indexed.UnlockBits(target);}
                using(var memory=new MemoryStream()){indexed.Save(memory,ImageFormat.Png);byte[] packed=memory.ToArray();
                    // GDI+ platform variations must not discard transparency.
                    bool alphaOK=true;if(transparent){using(var verify=new Bitmap(new MemoryStream(packed))){for(int y=0;y<bitmap.Height&&alphaOK;y++)for(int x=0;x<bitmap.Width;x++)if(pixels[y*data.Stride+x*4+3]==0&&verify.GetPixel(x,y).A!=0){alphaOK=false;break;}}}
                    byte[] chosen=alphaOK&&packed.Length<rgba.Length?packed:rgba;output.Write(chosen,0,chosen.Length);
                }
            }
        }
    }
}
