// ines.h -- a .nes file's 16-byte header: iNES, and NES 2.0 where it says so.
//
//   0-3   "NES" $1A
//   4     PRG ROM, 16K units          (NES 2.0: byte 9's low nibble is the MSB;
//   5     CHR ROM, 8K units            $F there means 2^E * (2M + 1) bytes)
//   6     mapper D0-D3 << 4, 4-screen, trainer, battery, mirroring (1: vertical)
//   7     mapper D4-D7, NES 2.0 ($08 in bits 2-3), console type
//   8     NES 2.0: submapper << 4 | mapper D8-D11;  iNES: PRG RAM, 8K units
//   9     NES 2.0: CHR MSB << 4 | PRG MSB;          iNES: bit 0 PAL
//   10-11 NES 2.0: PRG RAM / NVRAM, CHR RAM / NVRAM (64 << n bytes, 0: none)
//   12    NES 2.0: timing (NTSC, PAL, either, Dendy)
//
// An iNES header with junk in bytes 12-15 (old tools wrote "DiskDude!"
// from byte 7) has its byte 7 ignored, as emulators do.
//
// Plain C: the plugin and the Mac tests share it.
#pragma once
#include <stdbool.h>
#include <stdint.h>

enum { INES_NTSC, INES_PAL, INES_MULTI, INES_DENDY };
enum { INES_H, INES_V, INES_FOUR };        // mirroring: horizontal, vertical, four-screen

typedef struct {
    bool nes2, dirty;                      // NES 2.0; an iNES header with junk in 12-15
    uint16_t mapper;
    uint8_t submapper;                     // NES 2.0 only
    uint64_t prg, chr;                     // ROM bytes (chr 0: the board has CHR RAM)
    uint32_t prg_ram, prg_nvram, chr_ram, chr_nvram;   // bytes (iNES: prg_ram as byte 8 says, 8K if 0)
    uint8_t mirroring;                     // INES_H/V/FOUR
    bool battery, trainer;
    uint8_t console;                       // 0 NES/Famicom, 1 Vs., 2 PlayChoice-10, 3 extended
    uint8_t timing;                        // INES_NTSC...
    uint64_t want;                         // the bytes the header accounts for: 16 + trainer + PRG + CHR
} ines_t;

// NULL, or why h isn't a .nes header
const char *ines_parse(const uint8_t h[16], ines_t *r);
// the mapper's usual name ("MMC1"), or NULL for one this table lacks
const char *ines_mapper_name(unsigned mapper);
const char *ines_timing_name(unsigned timing);
const char *ines_mirroring_name(unsigned mirroring);
// n bytes as "256K", "8K", "1.5M", "512" (out holds 12)
char *ines_size(uint64_t n, char out[12]);
