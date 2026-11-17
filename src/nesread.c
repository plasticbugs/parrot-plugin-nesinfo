// nesread.c -- see nesread.h.
#include "nesread.h"

#include <stdio.h>
#include <string.h>

static uint32_t tab[256];

static void tab_init(void) {
    if (tab[1]) return;
    for (uint32_t i = 0; i < 256; i++) {
        uint32_t c = i;
        for (int k = 0; k < 8; k++) c = c >> 1 ^ (0xEDB88320u & (0u - (c & 1)));
        tab[i] = c;
    }
}

void nesread_header(nesread_t *r, uint64_t size) {
    nesui_t *ui = r->ui;
    ui->size = size;
    ui->have = false;
    ui->crc_state = -1;
    uint32_t lba = fs_lba(r->fs, 0);
    if (!lba || !r->read(lba, r->buf)) {
        ui->error = "the card didn't answer";
        return;
    }
    const char *why = size < 16 ? "too short for a NES file" : ines_parse(r->buf, &ui->h);
    if (why) {
        ui->error = why;
        return;
    }
    ui->have = true;
    tab_init();
    uint32_t skip = 16 + (ui->h.trainer ? 512 : 0);
    r->crc = 0xFFFFFFFFu;
    r->sec = skip / 512;
    r->skip = skip % 512;
    r->end_sec = (uint32_t)((size + 511) / 512);
    ui->crc_len = size > skip ? size - skip : 0;
    ui->crc_at = 0;
    ui->crc_state = 0;
}

bool nesread_step(nesread_t *r, int n) {
    nesui_t *ui = r->ui;
    if (ui->crc_state) return true;
    for (int i = 0; i < n; i++) {
        if (r->sec >= r->end_sec) {
            ui->crc = ~r->crc;
            ui->crc_state = 1;
            return true;
        }
        uint32_t lba = fs_lba(r->fs, r->sec);
        if (!lba || !r->read(lba, r->buf)) {
            snprintf(r->err, sizeof r->err, "a read failed at %luK", (unsigned long)(r->sec / 2));
            ui->error = r->err;
            ui->crc_state = -1;
            return true;
        }
        uint32_t to = 512;
        if ((uint64_t)r->sec * 512 + 512 > ui->size) to = (uint32_t)(ui->size - (uint64_t)r->sec * 512);
        if (to < r->skip) to = r->skip;     // it ends inside what's skipped
        uint32_t c = r->crc;
        for (uint32_t k = r->skip; k < to; k++) c = c >> 8 ^ tab[(c ^ r->buf[k]) & 0xFF];
        r->crc = c;
        ui->crc_at += to - r->skip;
        r->skip = 0;
        r->sec++;
    }
    return false;
}
