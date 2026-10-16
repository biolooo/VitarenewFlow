using System;
using System.IO;
using System.Text;
namespace LibraryDesktop {
    internal static class CiaMetadata {
        static long Align(long value) { return checked((value + 63) & ~63L); }
        static ulong Big(byte[] data, int offset, int count) { ulong value = 0; for (int i = 0; i < count; i++) value = (value << 8) | data[offset + i]; return value; }
        public static bool Read(string file, out string id, out string name) {
            id = null; name = Card.CleanTitle(file);
            using (var stream = File.OpenRead(file)) using (var reader = new BinaryReader(stream)) {
                if (stream.Length < 0x2020 || stream.Length >= 4294967296L) return false;
                uint header = reader.ReadUInt32(); reader.ReadUInt32();
                uint cert = reader.ReadUInt32(), ticket = reader.ReadUInt32(), tmd = reader.ReadUInt32(), meta = reader.ReadUInt32();
                ulong content = reader.ReadUInt64();
                if (header < 0x2020 || header > 0x40000 || cert > 0x1000000 || ticket > 0x100000 || tmd > 0x800000 || tmd < 0x120 || meta > 0x1000000 || content > (ulong)stream.Length) return false;
                long tmdOffset = Align(Align(Align(header) + cert) + ticket), contentOffset = Align(tmdOffset + tmd), metaOffset = Align(contentOffset + (long)content);
                if (tmdOffset + tmd > stream.Length || contentOffset + (long)content > stream.Length || (meta > 0 && metaOffset + meta > stream.Length)) return false;
                stream.Position = tmdOffset; byte[] data = reader.ReadBytes((int)tmd);
                uint signature = (uint)Big(data, 0, 4);
                int skip = signature == 0x10000 || signature == 0x10003 ? 0x240 : signature == 0x10001 || signature == 0x10004 ? 0x140 : signature == 0x10002 || signature == 0x10005 ? 0x80 : -1;
                if (skip < 0 || data.Length < skip + 0xa0) return false;
                ulong title = Big(data, skip + 0x4c, 8), group = title >> 32;
                if (group != 0x00040000 && group != 0x00040002) return false;
                id = title.ToString("X16");
                if (meta >= 0x3ac0) {
                    stream.Position = metaOffset + 0x400; byte[] smdh = reader.ReadBytes(0x36c0);
                    if (smdh.Length == 0x36c0 && smdh[0] == 'S' && smdh[1] == 'M' && smdh[2] == 'D' && smdh[3] == 'H') {
                        foreach (int language in new[] { 2, 1, 0, 3, 4, 5 }) {
                            string label = Encoding.Unicode.GetString(smdh, 8 + language * 0x200, 128).Split('\0')[0].Trim();
                            if (label.Length > 0) { name = label; break; }
                        }
                    }
                }
                return true;
            }
        }
    }
}


