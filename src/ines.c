// ines.c -- see ines.h.
#include "ines.h"

#include <stdio.h>
#include <string.h>

// NES 2.0's ROM size: lsb units of `unit` bytes, or (msb $F) 2^E * (2M + 1)
static uint64_t rom_size(uint8_t lsb, uint8_t msb, uint64_t unit) {
    if (msb != 0xF) return ((uint64_t)msb << 8 | lsb) * unit;
    unsigned e = lsb >> 2, m = lsb & 3;
    return e > 61 ? 0 : (1ull << e) * (2 * m + 1);
}

static uint32_t ram_size(uint8_t shift) { return shift ? 64u << shift : 0; }

const char *ines_parse(const uint8_t h[16], ines_t *r) {
    memset(r, 0, sizeof *r);
    if (memcmp(h, "NES\x1a", 4)) return "not a NES file (no NES header)";
    r->nes2 = (h[7] & 0x0C) == 0x08;
    r->dirty = !r->nes2 && (h[12] | h[13] | h[14] | h[15]);
    uint8_t h7 = r->dirty ? 0 : h[7];
    r->mapper = (uint16_t)(h[6] >> 4 | (h7 & 0xF0));
    r->mirroring = h[6] & 8 ? INES_FOUR : h[6] & 1 ? INES_V : INES_H;
    r->battery = h[6] & 2;
    r->trainer = h[6] & 4;
    r->console = h7 & 3;
    if (r->nes2) {
        r->mapper |= (uint16_t)(h[8] & 0x0F) << 8;
        r->submapper = h[8] >> 4;
        r->prg = rom_size(h[4], h[9] & 0x0F, 16384);
        r->chr = rom_size(h[5], h[9] >> 4, 8192);
        r->prg_ram = ram_size(h[10] & 0x0F);
        r->prg_nvram = ram_size(h[10] >> 4);
        r->chr_ram = ram_size(h[11] & 0x0F);
        r->chr_nvram = ram_size(h[11] >> 4);
        r->timing = h[12] & 3;
    } else {
        r->prg = (uint64_t)h[4] * 16384;
        r->chr = (uint64_t)h[5] * 8192;
        r->prg_ram = (r->dirty || !h[8] ? 1 : h[8]) * 8192u;
        r->chr_ram = r->chr ? 0 : 8192;
        r->timing = !r->dirty && (h[9] & 1) ? INES_PAL : INES_NTSC;
    }
    r->want = 16 + (r->trainer ? 512 : 0) + r->prg + r->chr;
    if (!r->prg) return "no PRG ROM in the header";
    return NULL;
}

const char *ines_mapper_name(unsigned m) {
    static const struct { uint16_t n; const char *name; } t[] = {
        {0, "NROM"}, {1, "MMC1"}, {2, "UxROM"}, {3, "CNROM"}, {4, "MMC3"}, {5, "MMC5"},
        {7, "AxROM"}, {9, "MMC2"}, {10, "MMC4"}, {11, "Color Dreams"}, {13, "CPROM"},
        {16, "Bandai FCG"}, {18, "Jaleco SS88006"}, {19, "Namco 163"}, {21, "VRC4"},
        {22, "VRC2"}, {23, "VRC2/VRC4"}, {24, "VRC6"}, {25, "VRC4"}, {26, "VRC6"},
        {34, "BNROM/NINA-001"}, {64, "RAMBO-1"}, {66, "GxROM"}, {68, "Sunsoft-4"},
        {69, "FME-7"}, {71, "Camerica"}, {73, "VRC3"}, {75, "VRC1"}, {79, "NINA-03/06"},
        {85, "VRC7"}, {87, "Jaleco J87"}, {94, "UN1ROM"}, {105, "NES-EVENT"},
        {118, "TxSROM"}, {119, "TQROM"}, {140, "Jaleco JF-11"}, {180, "UNROM (AND)"},
        {206, "Namco 118"}, {210, "Namco 175/340"},
    };
    for (unsigned i = 0; i < sizeof t / sizeof t[0]; i++)
        if (t[i].n == m) return t[i].name;
    return NULL;
}

const char *ines_timing_name(unsigned t) {
    static const char *n[] = {"NTSC", "PAL", "NTSC or PAL", "Dendy"};
    return n[t & 3];
}

const char *ines_mirroring_name(unsigned m) {
    return m == INES_FOUR ? "four-screen" : m == INES_V ? "vertical" : "horizontal";
}

char *ines_size(uint64_t n, char out[12]) {
    if (n >= 1u << 20 && n % (1u << 20) == 0) snprintf(out, 12, "%luM", (unsigned long)(n >> 20));
    else if (n >= 1u << 20) snprintf(out, 12, "%lu.%luM", (unsigned long)(n >> 20), (unsigned long)((n % (1u << 20)) * 10 >> 20));
    else if (n >= 1024 && n % 1024 == 0) snprintf(out, 12, "%luK", (unsigned long)(n >> 10));
    else snprintf(out, 12, "%lu", (unsigned long)n);
    return out;
}
