// nesread.h -- NES Info's reading: the open file's header, then the CRC-32
// of what follows it (the trainer too is skipped), a few sectors at a time
// so the plugin's loop keeps serving the console. Into a nesui_t, which the
// screen shows.
//
// Plain C: the plugin and the Mac tests (test/host_test.c, on FAT and exFAT
// card images) run the same code.
#pragma once
#include <stdbool.h>
#include <stdint.h>

#include "../sdplayer/fsro.h"
#include "nesui.h"

typedef struct {
    fs_t *fs;                               // the file open on it (fs_open)
    bool (*read)(uint32_t lba, uint8_t *buf);
    nesui_t *ui;
    uint32_t crc, sec, end_sec, skip;
    uint8_t buf[512];
    char err[64];
} nesread_t;

// the header (sector 0) into ui->h and ui->have, or ui->error; then the CRC
// is started (ui->crc_state 0). size: the file's bytes.
void nesread_header(nesread_t *r, uint64_t size);

// up to n more sectors of the CRC: true once it's over, ui->crc_state 1 with
// ui->crc, or -1 with ui->error (a read failed)
bool nesread_step(nesread_t *r, int n);
