#include "catalog_format.h"
#include <string.h>
bool catalog_is_application(uint64_t id)
{
    uint32_t category = (uint32_t)(id >> 32);
    return category == 0x00040000 || category == 0x00040002;
}
bool catalog_smdh_valid(const uint8_t *data, size_t size)
{
    return data && size == SMDH_SIZE && memcmp(data, "SMDH", 4) == 0;
}
/* ASCII console: transliterate German letters and reject control bytes. */
void catalog_text(char *out, size_t capacity, const uint8_t *text, size_t units)
{
    if (!capacity) return;
    size_t used = 0;
    for (size_t i = 0; i < units; ++i) {
        unsigned c = text[i*2] | ((unsigned)text[i*2+1] << 8);
        if (!c) break;
        const char *replacement = NULL;
        switch(c) {
            case 0xe4: replacement = "ae"; break;
            case 0xf6: replacement = "oe"; break;
            case 0xfc: replacement = "ue"; break;
            case 0xc4: replacement = "Ae"; break;
            case 0xd6: replacement = "Oe"; break;
            case 0xdc: replacement = "Ue"; break;
            case 0xdf: replacement = "ss"; break;
            default: break;
        }
        char single[2] = {(c >= 32 && c <= 126) ? (char)c : (c < 32 ? ' ' : '?'), 0};
        if (!replacement) replacement = single;
        size_t n = strlen(replacement);
        if (used + n >= capacity) break;
        memcpy(out + used, replacement, n);
        used += n;
    }
    while (used && out[used-1] == ' ') --used;
    out[used] = 0;
}
void catalog_name(char *out, size_t capacity, const uint8_t *smdh, unsigned language, bool publisher)
{
    unsigned choices[] = {language < 16 ? language : 1, 1, 0, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    if (!capacity) return;
    out[0] = 0;
    for (unsigned i = 0; i < sizeof(choices)/sizeof(choices[0]); ++i) {
        const uint8_t *text = smdh + 8 + choices[i]*0x200 + (publisher ? 0x180 : 0);
        catalog_text(out, capacity, text, 0x40);
        if (out[0]) return;
    }
}
unsigned catalog_icon_index(unsigned x, unsigned y)
{
    unsigned morton = 0;
    for (unsigned bit = 0; bit < 3; ++bit) {
        morton |= ((x >> bit) & 1) << (bit*2);
        morton |= ((y >> bit) & 1) << (bit*2+1);
    }
    return ((y / 8)*6 + x / 8)*64 + morton;
}
