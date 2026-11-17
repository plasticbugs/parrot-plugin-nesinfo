// nesui.c -- see nesui.h.
#include "nesui.h"

#include <stdio.h>
#include <string.h>

#define X0      16
#define XV      96                  // values: label column 10 characters
#define W       224                 // to x 240
#define ROW0    62
#define ROW_H   12

static void commas(uint64_t n, char *out, size_t max) {
    char d[24];
    int len = snprintf(d, sizeof d, "%llu", (unsigned long long)n), o = 0;
    for (int i = 0; i < len && o + 1 < (int)max; i++) {
        if (i && (len - i) % 3 == 0 && o + 2 < (int)max) out[o++] = ',';
        out[o++] = d[i];
    }
    out[o] = 0;
}

static void row(canvas_t *c, int i, const char *label, const char *value, uint8_t colour) {
    int y = ROW0 + i * ROW_H;
    cvs_line(c, X0, y, XV - X0, label, C_GRAY, C_BLACK, 1);
    cvs_line(c, XV, y, X0 + W - XV, value, colour, C_BLACK, 1);
}

// s in lines of n characters, at most rows of them, from y; words kept whole where they can be
static void wrap(canvas_t *c, int y, const char *s, int rows, uint8_t colour) {
    int n = W / 8;
    for (int r = 0; r < rows; r++, y += 10) {
        char line[40] = "";
        size_t left = s ? strlen(s) : 0;
        if (left) {
            size_t take = left <= (size_t)n ? left : (size_t)n;
            if (take < left) {
                size_t sp = take;
                while (sp > 0 && s[sp] != ' ') sp--;
                if (sp > 0) take = sp;
            }
            memcpy(line, s, take);
            line[take] = 0;
            s += take;
            while (*s == ' ') s++;
        }
        cvs_line(c, X0, y, W, line, colour, C_BLACK, 1);
    }
}

void nesui_draw(canvas_t *c, const nesui_t *s) {
    char a[40], b[16];
    // all of it, every time: what changed is all that reaches the console
    // (vdp_list), and a layout that changes (reading -> the header) leaves nothing behind
    canvas_fill(c, 0, 0, VDP_W, VDP_H, C_BLACK);
    cvs_line(c, X0, 8, W, "NES INFO", C_LBLUE, C_BLACK, 2);
    wrap(c, 32, s->file ? s->file : "", 2, C_WHITE);
    canvas_fill(c, X0, 54, W, 1, C_GRAY);
    int rows = 9;
    if (!s->file || s->error || !s->have) {
        for (int i = 0; i < rows; i++) row(c, i, "", "", C_WHITE);
        if (!s->file) {
            wrap(c, ROW0, "Open a .nes file from the menu: pick it and press A.", 4, C_WHITE);
        } else if (s->error) {
            cvs_line(c, X0, ROW0, W, "Can't read it:", C_LRED, C_BLACK, 1);
            wrap(c, ROW0 + 12, s->error, 4, C_WHITE);
        } else {
            cvs_line(c, X0, ROW0, W, "Reading the card...", C_WHITE, C_BLACK, 1);
        }
    } else {
        const ines_t *h = &s->h;
        const char *name = ines_mapper_name(h->mapper);
        if (h->nes2 && h->submapper) snprintf(a, sizeof a, "%u.%u", h->mapper, h->submapper);
        else snprintf(a, sizeof a, "%u", h->mapper);
        row(c, 0, "Mapper", a, C_WHITE);
        if (name) cvs_line(c, XV + 8 * ((int)strlen(a) + 2), ROW0, X0 + W - XV - 8 * ((int)strlen(a) + 2), name, C_LYELLOW, C_BLACK, 1);
        row(c, 1, "PRG ROM", ines_size(h->prg, b), C_WHITE);
        if (h->chr) snprintf(a, sizeof a, "ROM %s", ines_size(h->chr, b));
        else if (h->chr_ram || h->chr_nvram) snprintf(a, sizeof a, "RAM %s", ines_size(h->chr_ram + h->chr_nvram, b));
        else snprintf(a, sizeof a, "none");
        row(c, 2, "CHR", a, C_WHITE);
        uint32_t ram = h->prg_ram + h->prg_nvram;
        if (!ram) snprintf(a, sizeof a, h->battery ? "none, battery" : "none");
        else snprintf(a, sizeof a, "%s%s", ines_size(ram, b), h->battery ? ", battery" : "");
        row(c, 3, "PRG RAM", a, C_WHITE);
        snprintf(a, sizeof a, "%s%s", ines_mirroring_name(h->mirroring), h->trainer ? ", trainer" : "");
        row(c, 4, "Mirroring", a, C_WHITE);
        static const char *console[] = {"", ", Vs.", ", PlayChoice", ", extended"};
        snprintf(a, sizeof a, "%s%s", ines_timing_name(h->timing), console[h->console & 3]);
        row(c, 5, "TV", a, C_WHITE);
        row(c, 6, "Header", h->nes2 ? "NES 2.0" : h->dirty ? "iNES, junk in 12-15" : "iNES", C_WHITE);
        commas(s->size, a, sizeof a);
        strncat(a, " bytes", sizeof a - strlen(a) - 1);
        uint8_t fc = C_WHITE;
        if (s->size < h->want) {
            commas(h->want - s->size, b, sizeof b);
            snprintf(a, sizeof a, "short by %s", b);
            fc = C_LRED;
        }
        row(c, 7, "File", a, fc);
        char t[12];
        if (s->crc_state < 0) a[0] = 0;
        else if (!s->crc_state) snprintf(a, sizeof a, "%s of %s", ines_size(s->crc_at & ~1023ull, b), ines_size(s->crc_len, t));
        else snprintf(a, sizeof a, "%08lX", (unsigned long)s->crc);
        row(c, 8, "CRC-32", a, s->crc_state > 0 ? C_LGREEN : C_WHITE);
    }
    cvs_line(c, X0, 176, W, s->note ? s->note : "", C_GRAY, C_BLACK, 1);
}
