// nesinfo -- NES Info, the first plugin that opens a file: pick a .nes on the
// card in the menu, press A, and the TV shows what its header says (mapper,
// PRG and CHR sizes, RAM and battery, mirroring, TV system) and the CRC-32
// of the ROM after the header, read through the whole file from the card.
// RESET goes back to the menu, which comes back on the same file, with the
// mapper and CRC on its bottom line.
//
// It is also the SDK's example (../sdk): the console side is PSG Synth's
// kernel (cvkernel.h), the screen a canvas (cvscreen.h), the card cvsd.h,
// and the build cvplugin.cmake's.
//
//   core 1  serves the kernel (cvkernel.c)
//   core 0  this loop: cvk_task() (power, RESET back to the menu), the
//           kernel's reports, the card, the screen
#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"

#include "../midihello/mailbox.h"
#include "cvkernel.h"
#include "cvscreen.h"
#include "cvsd.h"
#include "handoff.h"
#include "nesread.h"
#include "nesui.h"
#include "rom_image.h"

#define UI_US       20000           // the screen is brought up to date this often (at most)
#define OPEN_US     20000           // serve the console this long before the card's (blocking) start
#define CRC_SECS    8               // sectors read a pass

static cvs_t scr;
static uint8_t list[CVS_LIST_MAX];
static fs_t fs;
static fs_file_t file;
static nesui_t ui = {.crc_state = -1};
static nesread_t rd = {.fs = &fs, .read = cvsd_read, .ui = &ui};
static char note[33];

static void draw(canvas_t *c, bool all, void *ctx) {
    (void)all;                              // nesui_draw draws it all each time
    nesui_draw(c, ctx);
}

static const char *fs_why(int r) {
    switch (r) {
    case FS_EIO:     return cvsd_card_in() ? "the card didn't answer" : "no card in the slot";
    case FS_ENOFS:   return "no FAT or exFAT partition on the card";
    case FS_ENOFILE: return "the file is empty or gone";
    case FS_ECHAIN:  return "its clusters are broken (check the card)";
    case FS_EFRAG:   return "it's in too many pieces on the card";
    default:         return "the card's file system is odd";
    }
}

// the menu's bottom line once we're back: what was found
static void set_note(void) {
    if (ui.crc_state > 0) {
        const char *m = ines_mapper_name(ui.h.mapper);
        snprintf(note, sizeof note, "Mapper %u%s%s, CRC %08lX", ui.h.mapper, m ? " " : "", m ? m : "",
                 (unsigned long)ui.crc);
        if (strlen(note) > 28) snprintf(note, sizeof note, "Mapper %u, CRC %08lX", ui.h.mapper, (unsigned long)ui.crc);
        cvk_note(note);
    } else if (ui.error) {
        cvk_note(ui.have ? "NES Info: a card read failed" : "NES Info couldn't read the file");
    }
}

int main(void) {
    static const cvk_config_t cfg = {rom_image, addr_line, data_bit, handoff_bin, HANDOFF_BIN_LEN};
    cvk_start(&cfg);
    cvs_init(&scr);
    const cvl_t *b = cvk_launch();
    static char name[FS_NAME];
    if (b && b->path[0]) {
        const char *slash = strrchr(b->path, '/');
        strncpy(name, slash ? slash + 1 : b->path, sizeof name - 1);
        ui.file = name;
    }
    ui.note = "RESET: back to the menu";

    uint32_t t0 = time_us_32(), t_ui = t0;
    bool opened = false;
    for (;;) {
        uint32_t t = time_us_32();
        cvk_task(t);
        int k;
        uint8_t bits;
        while (cvk_report(t, &k, &bits))
            if (k == REPORT_KIND_ACK || k == REPORT_KIND_HELLO) cvs_report(&scr, k == REPORT_KIND_HELLO, bits & 31, t);
        if (!opened && ui.file && t - t0 > OPEN_US) {
            opened = true;
            int r = cvsd_open_launched(&fs, b, false, &file);
            if (r == FS_OK) nesread_header(&rd, file.size);
            else ui.error = fs_why(r);
            set_note();
        } else if (ui.crc_state == 0 && nesread_step(&rd, CRC_SECS)) {
            set_note();
        }
        if (t - t_ui >= UI_US) {
            t_ui = t;
            size_t n = cvs_update(&scr, draw, &ui, list, sizeof list, t);
            if (n) cvk_list(list, n, scr.out_full, scr.seq);
        }
    }
}
