using System.Text.Json;
namespace System.Web.Script.Serialization {
    // Preserve the existing cover engine's dictionary/array contract on .NET.
    internal sealed class JavaScriptSerializer {
        public int MaxJsonLength { get; set; } = 16 * 1024 * 1024;
        public object DeserializeObject(string value) {
            if (value.Length > MaxJsonLength) throw new IOException("JSON response too large.");
            using var doc = JsonDocument.Parse(value, new JsonDocumentOptions { MaxDepth = 64 });
            return Convert(doc.RootElement);
        }
        static object Convert(JsonElement value) => value.ValueKind switch {
            JsonValueKind.Object => value.EnumerateObject().ToDictionary(p => p.Name, p => Convert(p.Value)),
            JsonValueKind.Array => value.EnumerateArray().Select(Convert).ToArray(),
            JsonValueKind.String => value.GetString(), JsonValueKind.True => true,
            JsonValueKind.False => false, JsonValueKind.Number => value.GetDouble(), _ => null
        };
        public string Serialize(object value) => JsonSerializer.Serialize(value, new JsonSerializerOptions { IncludeFields = true });
    }
}
