using SkiaSharp;
namespace LibraryDesktop;
internal static class PortableImages {
    static SKBitmap Render(string source) {
        if (new FileInfo(source).Length > 32L * 1024 * 1024) throw new IOException("Bitte ein Bild unter 32 MB verwenden.");
        using var input = File.OpenRead(source);
        using var codec = SKCodec.Create(input) ?? throw new IOException("Kein lesbares PNG-/JPG-Bild.");
        var info = codec.Info;
        if (info.Width <= 0 || info.Height <= 0 || (long)info.Width * info.Height > 40000000)
            throw new IOException("Bitte ein Bild mit höchstens 40 Megapixeln verwenden.");
        using var decoded = SKBitmap.Decode(codec) ?? throw new IOException("Bild konnte nicht dekodiert werden.");
        var origin = codec.EncodedOrigin;
        bool swap = origin is SKEncodedOrigin.LeftTop or SKEncodedOrigin.RightTop or SKEncodedOrigin.RightBottom or SKEncodedOrigin.LeftBottom;
        int orientedWidth = swap ? decoded.Height : decoded.Width, orientedHeight = swap ? decoded.Width : decoded.Height;
        double scale = Math.Min(1, 256d / Math.Max(orientedWidth, orientedHeight));
        int width = Math.Max(1, (int)Math.Round(orientedWidth * scale)), height = Math.Max(1, (int)Math.Round(orientedHeight * scale));
        var result = new SKBitmap(new SKImageInfo(width, height, SKColorType.Rgba8888, SKAlphaType.Premul));
        try { using var canvas = new SKCanvas(result);
            canvas.Clear(SKColors.Transparent); canvas.Scale((float)width / orientedWidth, (float)height / orientedHeight);
            switch (origin) {
                case SKEncodedOrigin.TopRight: canvas.Translate(decoded.Width, 0); canvas.Scale(-1, 1); break;
                case SKEncodedOrigin.BottomRight: canvas.Translate(decoded.Width, decoded.Height); canvas.RotateDegrees(180); break;
                case SKEncodedOrigin.BottomLeft: canvas.Translate(0, decoded.Height); canvas.Scale(1, -1); break;
                case SKEncodedOrigin.LeftTop: canvas.RotateDegrees(90); canvas.Scale(1, -1); break;
                case SKEncodedOrigin.RightTop: canvas.Translate(decoded.Height, 0); canvas.RotateDegrees(90); break;
                case SKEncodedOrigin.RightBottom: canvas.Translate(decoded.Height, decoded.Width); canvas.RotateDegrees(90); canvas.Scale(-1, 1); break;
                case SKEncodedOrigin.LeftBottom: canvas.Translate(0, decoded.Width); canvas.RotateDegrees(270); break;
            }
            using var decodedImage = SKImage.FromBitmap(decoded);
            canvas.DrawImage(decodedImage, SKRect.Create(decoded.Width, decoded.Height), new SKSamplingOptions(SKCubicResampler.Mitchell));
        }
        catch { result.Dispose(); throw; }
        return result;
    }
    static byte[] Encode(string source) {
        using var result = Render(source); using var image = SKImage.FromBitmap(result);
        using var png = image.Encode(SKEncodedImageFormat.Png, 100); return png.ToArray();
    }
    public static Avalonia.Media.Imaging.Bitmap LoadPreview(string source) {
        using var memory = new MemoryStream(Encode(source)); return new Avalonia.Media.Imaging.Bitmap(memory);
    }
    public static void Convert(string source, string target) {
        byte[] png = Encode(source);
        using (var output = new FileStream(target, FileMode.CreateNew, FileAccess.Write, FileShare.None)) { output.Write(png); output.Flush(true); }
        using var verify = SKCodec.Create(target);
        if (verify == null || verify.Info.Width > 256 || verify.Info.Height > 256) throw new IOException("Cover-Prüfung fehlgeschlagen.");
    }
}
