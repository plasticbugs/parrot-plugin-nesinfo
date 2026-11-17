// nesui.h -- NES Info's screen, drawn on the cart into a canvas
// (../midihello/vdp.h):
//
//   NES INFO                                   (twice the size)
//   MEGA MAN 2 (USA).NES
//   ------------------------------------------
//   MAPPER     1  MMC1
//   PRG ROM    256K
//   CHR        RAM 8K
//   PRG RAM    8K
//   MIRRORING  HORIZONTAL
//   TV         NTSC
//   HEADER     INES
//   FILE       262,160 BYTES
//   CRC-32     5E268761                    (of the ROM after the header)
//
//   RESET: BACK TO THE MENU
//
// Plain C with no Pico dependencies: the plugin and the Mac tests share it.
#pragma once
#include <stdbool.h>
#include <stdint.h>

#include "cvscreen.h"
#include "ines.h"

typedef struct {
    const char *file;           // the file's name; NULL: not started with one
    const char *error;          // why it couldn't be read; NULL: none
    bool have;                  // h holds the file's header
    ines_t h;
    uint64_t size;              // the file's bytes
    int crc_state;              // CRC-32: -1 not started, 0 reading, 1 done
    uint64_t crc_at, crc_len;   // bytes read so far, of
    uint32_t crc;               // the file after the 16-byte header (and the trainer)
    const char *note;           // the bottom line
} nesui_t;

// the whole screen, from black; only what changed reaches the console (vdp_list)
void nesui_draw(canvas_t *c, const nesui_t *s);
