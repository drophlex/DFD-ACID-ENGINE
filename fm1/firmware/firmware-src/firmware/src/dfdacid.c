/* SPDX-License-Identifier: GPL-3.0-only
 * DFD ACID ENGINE for the M-VAVE FM-1 (DARK FUNK DURCAL DESIGN · DESARROLLADO EN POLARIS SOUND LAB · MÉRIDA, YUCATÁN, MÉXICO),
 * on Felucca 1.8.2's drivers (Hügelton Instruments): the keys, knobs, buttons, screen, audio, flash, USB and the sequencer are
 * Felucca's; what is seen and played is DFD ACID's own, as the web version: its screen replaces Felucca's pages (main.c calls
 * acid_input, acid_draw and acid_leds instead of ui_input, ui_draw and ui_leds).
 *
 * As on the web: one acid / dirty voice (eng_dfdacid.c) on track 1 plays a 16-step pattern from the web version's generator
 * (acid_tables.h, made from web/pattern.js: the same patterns, note for note), in 16 styles (10 + the DIRTY LAB's 6 riff
 * families), 3 scales, 12 keys and 24 seeds each; 19 sounds (each style's own and the DIRTY LAB's).
 *
 *   PRESETS knob  STYLE           K1..K4  SOUND page: CUT  RES  ENV  DEC     FX    SOUND / DIRTY page
 *   ALGORITHM     SOUND                   DIRTY page: FILTH PUMP ZAP WIDTH    SCL   the scale       ENV  the key
 *   SELECT        TEMPO           OCT+    GENERATE (the next seed)            LFO   DNA LOCK: OFF / NOTES / RHYTHM
 *   PLAY          play / stop     OCT-    MUTATE                              HOME  about (and back)   EDIT  English / Español
 *   The keys play the voice over the pattern. PUMP is Felucca's LFO through the matrix on AMP (SAW, 1/4, FREE, UNI): a sidechain duck.
 * What it remembers: the patterns and the sound are Felucca's (its autosave); the style, sound, scale, key, seed and lock are kept
 * in the silent track 4's parameters (acid_keep / acid_recover), so a power-on comes back as it was.
 * Integer maths, bounded loops; the steps are written in the main loop. */
#include "acid_tables.h"

#define ACID_MARK 17                      /* trk[3].p[P_E7]: "the settings below are DFD ACID's" */
static const char *const ACID_LOCK[3] = {"OFF", "NOTES", "RHYTHM"};
static const char *const ACID_KEY[12] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};

static struct {
    uint8_t sty, snd, scl, key, seed, lock, page, info, booted, mine;
    uint32_t hot_ms, hot_what, sig, rnd;
} acid = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ~0u, 0x1234567u};

static uint32_t acid_rand(void) { acid.rnd = acid.rnd * 1664525u + 1013904223u; return acid.rnd >> 8; }

/* ------------------------------------------------------------------ the pattern --- */
static void acid_put(step_t *st, uint16_t code, int32_t root)    /* one of the web generator's steps (acid_tables.h) */
{
    const uint32_t f = code >> 8;
    step_clear(st);
    if (!(f & 1u)) return;
    if (f & 8u) { st->time = ST_TIE; return; }
    st->time = ST_NOTE;
    st->note[0] = (uint8_t)clamp(root + (int8_t)(code & 255u), 12, 108);
    st->n = 1;
    st->vel = (f & 2u) ? 127 : 100;
    st->flags = (uint8_t)((f & 2u ? SF_ACCENT : 0u) | (f & 4u ? SF_SLIDE : 0u));
    step_set_ratchet(st, ((f >> 4) & 3u) + 1u);
}
static int32_t acid_root(void) { return 36 + (int32_t)acid.key - (acid.key > 6u ? 12 : 0); }   /* C2 .. F#2, then G1 .. B1 */
/* the steps of (style, scale, seed) at the key; DNA LOCK keeps the notes (or the rhythm) of what plays now */
static void acid_write(int keep)
{
    track_t *t = &trk[0];
    const uint16_t *pat = ACID_PAT[acid.sty % ACID_NSTY][acid.scl % ACID_NSCL][acid.seed % ACID_NSEED];
    step_t old[16];
    uint32_t s;
    for (s = 0; s < 16u; s++) old[s] = t->step[s];
    t->p[P_SLEN] = 16;
    t->p[P_SDIV] = 2;                                             /* 1/16 */
    for (s = 0; s < NSTEP; s++) {
        step_t *st = &t->step[s];
        if (s >= 16u) { step_clear(st); continue; }
        acid_put(st, pat[s], acid_root());
        if (keep && acid.lock == 1u && old[s].time == ST_NOTE && st->time == ST_NOTE) st->note[0] = old[s].note[0];   /* NOTES */
        if (keep && acid.lock == 2u) {                            /* RHYTHM: the old steps' timing, the new notes */
            const uint8_t note = st->time == ST_NOTE ? st->note[0] : (uint8_t)acid_root();
            if (old[s].time == ST_NOTE) { *st = old[s]; st->note[0] = note; } else *st = old[s];
        }
    }
}
static void acid_mutate(void)                                     /* a few notes, accents and slides change; step 1 stays */
{
    track_t *t = &trk[0];
    uint32_t s, k;
    for (k = 0; k < 4u; k++) {
        s = 1u + acid_rand() % 15u;
        step_t *st = &t->step[s];
        const uint32_t what = acid_rand() % 4u;
        if (what == 0u && acid.lock != 1u) {                      /* a note from another step (stays in the key and the style) */
            const step_t *o = &t->step[acid_rand() % 16u];
            if (o->time == ST_NOTE && st->time == ST_NOTE) st->note[0] = o->note[0];
        } else if (what == 1u && st->time == ST_NOTE) st->flags ^= SF_ACCENT, st->vel = st->flags & SF_ACCENT ? 127 : 100;
        else if (what == 2u && st->time == ST_NOTE) st->flags ^= SF_SLIDE;
        else if (what == 3u && acid.lock != 2u) {                 /* a rest becomes a note, or a note a rest */
            if (st->time == ST_NOTE) step_clear(st);
            else { const step_t *o = &t->step[0]; *st = *o; st->flags &= (uint8_t)~SF_SLIDE; }
        }
    }
}

/* ------------------------------------------------------------------ the sound --- */
static void acid_pump(uint32_t amt)                               /* amt 0..63: Felucca's LFO through the matrix (slot 4) on AMP */
{
    track_t *t = &trk[0];
    t->p[P_LSYNC] = 5;                                            /* 1/4 */
    t->p[P_LWAVE] = 2;                                            /* SAW: up from each beat, so the level ducks on the beat */
    t->p[P_LTRIG] = 1;                                            /* FREE: in time with the bars from PLAY */
    t->p[P_LPOL] = 1;                                             /* UNI */
    t->p[P_LD_AMP] = 0;
    t->p[P_M4SRC] = 1; t->p[P_M4DST] = 4;                         /* LFO -> AMP */
    t->p[P_M4AMT] = (int16_t)clamp((int32_t)amt, 0, 63);
}
static void acid_sound(uint32_t i)
{
    const acid_sound_t *d = &ACID_SND[i % ACID_NSND];
    track_t *t = &trk[0];
    acid.snd = (uint8_t)(i % ACID_NSND);
    load_begin(t, UNDO_SOUND);
    if (t->eng_req != ENGI_DFDACID) set_engine_of(t, ENGI_DFDACID);
    apply_preset_to(t, 0);
    t->p[P_E0] = d->voice; t->p[P_E1] = d->cut; t->p[P_E2] = d->res; t->p[P_E3] = d->env;
    t->p[P_E4] = d->dec; t->p[P_E5] = d->acc; t->p[P_E6] = d->filth; t->p[P_E7] = d->zap;
    t->p[P_DETUNE] = d->width;
    t->p[P_VOICE] = V_LEGATO;                                     /* slides: a held note glides into the next */
    t->p[P_GLIDE] = 18;
    t->p[P_DLY] = (int16_t)(i == 7u ? 30 : 12);                   /* a dub echo on HYPNOTIC, a touch elsewhere */
    t->p[P_REV] = (int16_t)(d->voice == 3u ? 18 : 6);
    acid_pump(d->pump / 2u);
    load_end(t);
}
static void acid_keep(void)                                       /* the settings, in the silent track 4 */
{
    track_t *t = &trk[3];
    if (t->eng_req != ENGI_DFDACID) set_engine_of(t, ENGI_DFDACID);
    t->p[P_LEVEL] = 0;
    t->p[P_E1] = acid.sty; t->p[P_E2] = acid.snd; t->p[P_E3] = acid.scl; t->p[P_E4] = acid.key;
    t->p[P_E5] = acid.seed; t->p[P_E6] = (int16_t)(acid.lock | (acid.mine ? 4u : 0u)); t->p[P_E7] = ACID_MARK;
}
static int acid_recover(void)
{
    const track_t *t = &trk[3];
    if (trk[0].eng_req != ENGI_DFDACID || t->eng_req != ENGI_DFDACID || t->p[P_E7] != ACID_MARK) return 0;
    acid.sty = (uint8_t)(t->p[P_E1] % (int16_t)ACID_NSTY); acid.snd = (uint8_t)(t->p[P_E2] % (int16_t)ACID_NSND);
    acid.scl = (uint8_t)(t->p[P_E3] % 3); acid.key = (uint8_t)(t->p[P_E4] % 12); acid.seed = (uint8_t)(t->p[P_E5] % (int16_t)ACID_NSEED);
    acid.lock = (uint8_t)((t->p[P_E6] & 3) % 3); acid.mine = (uint8_t)((t->p[P_E6] >> 2) & 1);
    return 1;
}
static void acid_quiet_others(void)                               /* tracks 2..4 silent: one voice, as on the web */
{
    uint32_t v, s;
    for (v = 1; v < NTRK; v++) { trk[v].p[P_LEVEL] = 0; for (s = 0; s < NSTEP; s++) step_clear(&trk[v].step[s]); }
}
static void acid_style(uint32_t i, int sound)                     /* a style: its sound (if asked), its tempo, a pattern */
{
    uint32_t v;
    acid.sty = (uint8_t)(i % ACID_NSTY);
    for (v = 0; v < NTRK; v++) load_begin(&trk[v], (sound ? UNDO_SOUND : 0u) | UNDO_PAT);
    if (sound) { acid_sound(ACID_STY_SOUND[acid.sty]); song.g[G_BPM] = ACID_STY_BPM[acid.sty]; }
    acid_quiet_others();
    acid_write(0);
    acid.mine = 0;
    acid_keep();
    for (v = 0; v < NTRK; v++) load_end(&trk[v]);
    song.sel = 0;                                                 /* the keys play the acid voice */
}

/* ------------------------------------------------------------------ the hooks Felucca's pages call (none shown) --- */
static int an_page(uint32_t g) { return g >= GR_AN_ALIGN && g <= GR_AN_VISION; }
static void an_knob(uint32_t g, uint32_t slot, int32_t steps) { (void)g; (void)slot; (void)steps; }
static void an_oct(void) {}
static const char *an_act_name(void) { return ""; }
static void an_graph(uint32_t g) { (void)g; }
static void an_columns(uint32_t g) { uint32_t c; (void)g; for (c = 0; c < 4u; c++) draw_column(c, "-", "", "", VAL(c), -1, ICON_AUTO); }
static uint32_t an_sig(void) { return 0; }
static void an_home_vision(void) {}

/* ------------------------------------------------------------------ the main loop --- */
static void andro_poll(void)
{
    lang_es = (uint8_t)(ui_idle >> 7);
    if (!acid.booted) {
        acid.booted = 1;
        acid.rnd ^= fm1_ms * 2654435761u;
        if (!acid_recover()) acid_style(0, 1);
    }
    an_live_fx = 0;
}

static void acid_hot(uint32_t what) { acid.hot_ms = fm1_ms | 1u; acid.hot_what = what; }
/* K1..K4 on a page: the track parameter it moves, its range */
static int16_t *acid_knob(uint32_t page, uint32_t k, int32_t *lo, int32_t *hi)
{
    static const uint8_t SOUND[4] = {P_E1, P_E2, P_E3, P_E4}, DIRTY[4] = {P_E6, P_M4AMT, P_E7, P_DETUNE};
    const uint8_t id = page ? DIRTY[k & 3u] : SOUND[k & 3u];
    *lo = 0; *hi = id == P_E7 ? 24 : id == P_M4AMT ? 63 : 127;
    return &trk[0].p[id];
}
static void acid_input(void)
{
    uint32_t pressed = fm1_input_edges(0), id, b;
    int32_t s, k, lo, hi;
    midi_take();
    midi_pc_poll();
    fm6_poll();
    settings_later_poll();
    if ((s = panel_enc(EN_PRESET)) != 0) {                        /* STYLE */
        acid_style((uint32_t)((int32_t)acid.sty + (s > 0 ? 1 : (int32_t)ACID_NSTY - 1)) % ACID_NSTY, 1);
        acid_hot(10);
    }
    if ((s = panel_enc(EN_ALGO)) != 0) {                          /* SOUND */
        acid_sound((acid.snd + (s > 0 ? 1u : ACID_NSND - 1u)) % ACID_NSND); acid_keep(); acid_hot(11);
    }
    if ((s = panel_enc(EN_SELECT)) != 0) {                        /* TEMPO */
        song.g[G_BPM] = (int16_t)clamp(song.g[G_BPM] + s, 40, 220);
        acid_hot(12);
    }
    for (k = 0; k < 4; k++) {
        if ((s = panel_enc(EN_K1 + (uint32_t)k)) == 0) continue;
        int16_t *p = acid_knob(acid.page, (uint32_t)k, &lo, &hi);
        *p = (int16_t)clamp(*p + s * (hi > 63 ? 2 : 1), lo, hi);
        if (acid.page && k == 1) acid_pump((uint32_t)*p);
        acid_hot((uint32_t)k + (acid.page ? 4u : 0u));
    }
    for (id = 0; id < 14u; id++) {
        if (!((pressed >> id) & 1u)) continue;
        b = panel_btn_of(id);
        switch (b) {
        case B_PLAY: transport_req = song.playing || chain_busy() || seq_counting() ? 2u : 1u; break;
        case B_OCTUP:                                             /* GENERATE: the next seed of the style */
            acid.seed = (uint8_t)((acid.seed + 1u + acid_rand() % 5u) % ACID_NSEED);
            load_begin(&trk[0], UNDO_PAT); acid_write(acid.lock != 0u); load_end(&trk[0]);
            acid.mine = 0; acid_keep(); acid_hot(13); break;
        case B_OCTDN: load_begin(&trk[0], UNDO_PAT); acid_mutate(); load_end(&trk[0]); acid.mine = 1; acid_keep(); acid_hot(14); break;
        case B_FX: acid.page ^= 1u; break;
        case B_SCL: acid.scl = (uint8_t)((acid.scl + 1u) % ACID_NSCL); load_begin(&trk[0], UNDO_PAT); acid_write(0); load_end(&trk[0]); acid_keep(); acid_hot(15); break;
        case B_ENV: {                                             /* the key: the pattern moves with it */
            const int32_t before = acid_root();
            uint32_t s2;
            acid.key = (uint8_t)((acid.key + 1u) % 12u);
            for (s2 = 0; s2 < 16u; s2++) if (trk[0].step[s2].time == ST_NOTE) trk[0].step[s2].note[0] = (uint8_t)clamp(trk[0].step[s2].note[0] + acid_root() - before, 12, 108);
            acid_keep(); acid_hot(16); break;
        }
        case B_LFO: acid.lock = (uint8_t)((acid.lock + 1u) % 3u); acid_keep(); acid_hot(17); break;
        case B_EDIT:
            lang_es ^= 1u;
            ui_idle = (uint8_t)(lang_es ? ui_idle | 0x80u : ui_idle & 0x7Fu);
            settings_later();
            break;
        case B_HOME: acid.info ^= 1u; break;
        default: break;
        }
        acid.sig = ~0u;
    }
}

/* ------------------------------------------------------------------ the screen: as the web version --- */
#define AC_INK RGB(17, 18, 22)
#define AC_CARBON RGB(27, 28, 34)
#define AC_LINE RGB(52, 54, 62)
#define AC_COPY RGB(237, 240, 233)
#define AC_MUTED RGB(150, 153, 156)
#define AC_LIME RGB(193, 255, 58)
#define AC_ORANGE RGB(255, 113, 71)
#define AC_VIOLET RGB(167, 153, 255)
#define AC_BLUE RGB(140, 188, 245)

static void acid_mark(int32_t x, int32_t y, uint16_t bg)          /* the DFD mark: three lime blocks */
{
    static const char *const L[3] = {"D", "F", "D"};
    int32_t i;
    for (i = 0; i < 3; i++) {
        cv_rect(x + i * 15, y, 14, 18, AC_LIME);
        cv_text_on(x + i * 15 + (14 - text_w(&AF_S, L[i])) / 2, y + 4, &AF_S, L[i], AC_INK, AC_LIME);
    }
    (void)bg;
}
/* the pattern as a roll: 16 columns, the pitch as height, accents orange, slides violet, ties blue, the playhead lime */
static void acid_roll(int32_t y0, int32_t h, int32_t cur)
{
    const track_t *t = &trk[0];
    int32_t s, lo = 127, hi = 0, prevy = -1;
    for (s = 0; s < 16; s++) if (t->step[s].time == ST_NOTE) { lo = t->step[s].note[0] < lo ? t->step[s].note[0] : lo; hi = t->step[s].note[0] > hi ? t->step[s].note[0] : hi; }
    if (hi < lo) { lo = 36; hi = 48; }
    if (hi - lo < 12) { lo -= (12 - (hi - lo)) / 2; hi = lo + 12; }
    for (s = 0; s < 16; s++) {
        const step_t *st = &t->step[s];
        const int32_t x = 8 + s * 14, on = st->time == ST_NOTE, tie = st->time == ST_TIE;
        cv_rect(x, y0, 13, h, s == cur ? ux_mix(AC_CARBON, AC_LIME, 22) : (s % 4 == 0 ? RGB(36, 38, 46) : AC_CARBON));
        if (on || tie) {
            const step_t *ref = st;
            int32_t k = s;
            while (ref->time == ST_TIE && k > 0) ref = &t->step[--k];
            {
                const int32_t note = ref->time == ST_NOTE ? ref->note[0] : lo, y = y0 + h - 8 - (note - lo) * (h - 16) / (hi - lo);
                const uint16_t c = tie ? AC_BLUE : (st->flags & SF_ACCENT) ? AC_ORANGE : AC_LIME;
                cv_rect(x + 1, y, 11, 6, c);
                if (on && (st->flags & SF_SLIDE) && prevy >= 0) an_line(x - 2, prevy + 3, x + 1, y + 3, 2, AC_VIOLET);
                if (on && step_ratchet(st) > 1u) { int32_t r; for (r = 1; r < (int32_t)step_ratchet(st); r++) cv_rect(x + 1 + r * 3, y + 8, 2, 2, c); }
                prevy = y;
            }
        } else prevy = -1;
        if (s == cur) cv_rect(x, y0 + h - 2, 13, 2, AC_LIME);
    }
}
static void acid_val(char *b, uint32_t page, uint32_t k)          /* a knob's value as the web shows it */
{
    int32_t lo, hi, v = *acid_knob(page, k, &lo, &hi);
    if (!page && k == 0u) { const uint32_t hz = ACID_CUT_HZ[v & 127]; if (hz >= 1000u) { fmt_int(b, hz / 1000u); str_cpy(b + str_len(b), ".", 2); fmt_int(b + str_len(b), hz / 100u % 10u); str_cpy(b + str_len(b), "K", 2); } else fmt_int(b, hz); return; }
    if (page && k == 2u) { if (!v) { str_cpy(b, "OFF", 4); return; } fmt_int(b, (uint32_t)v); str_cpy(b + str_len(b), " ST", 4); return; }
    fmt_int(b, (uint32_t)(v * 100 / 127));
}
static const char *acid_knob_name(uint32_t page, uint32_t k)
{
    static const char *const N[2][4] = {{"CUT", "RES", "ENV", "DEC"}, {"FILTH", "PUMP", "ZAP", "WIDTH"}};
    return N[page & 1u][k & 3u];
}
static void acid_panel(int32_t y)                                 /* the page: four knobs, their bars */
{
    char b[16];
    uint32_t k;
    const uint16_t hl = acid.page ? AC_ORANGE : AC_LIME;
    for (k = 0; k < 4u; k++) {
        int32_t lo, hi, v = *acid_knob(acid.page, k, &lo, &hi), x = 6 + (int32_t)k * 58;
        cv_text_on(x, y, &AF_S, acid_knob_name(acid.page, k), AC_MUTED, AC_INK);
        acid_val(b, acid.page, k);
        cv_text_on(x, y + 12, &AF_M, b, AC_COPY, AC_INK);
        cv_rect(x, y + 34, 52, 3, AC_LINE);
        cv_rect(x, y + 34, 52 * (v - lo) / (hi - lo), 3, hl);
    }
}
static void acid_status(int32_t y)                                /* what was just turned, large; else the page */
{
    char b[24];
    const char *label = 0;
    b[0] = 0;
    if (acid.hot_ms && fm1_ms - (acid.hot_ms & ~1u) < 1500u) {
        uint32_t w = acid.hot_what;
        if (w < 8u) { label = acid_knob_name(w >= 4u, w & 3u); acid_val(b, w >= 4u, w & 3u); }
        else switch (w) {
        case 10: label = "STYLE"; str_cpy(b, ACID_STY_NAME[acid.sty], sizeof b); break;
        case 11: label = "SOUND"; str_cpy(b, ACID_SND[acid.snd].name, sizeof b); break;
        case 12: label = "TEMPO"; fmt_int(b, (uint32_t)song.g[G_BPM]); break;
        case 13: label = "GENERATE"; str_cpy(b, "SEED ", 6); fmt_int(b + 5, acid.seed + 1u); break;
        case 14: label = "MUTATE"; str_cpy(b, "MY PATTERN", sizeof b); break;
        case 15: label = "SCALE"; str_cpy(b, ACID_SCL_NAME[acid.scl % 3u], sizeof b); break;
        case 16: label = "KEY"; str_cpy(b, ACID_KEY[acid.key % 12u], sizeof b); break;
        default: label = "DNA LOCK"; str_cpy(b, ACID_LOCK[acid.lock % 3u], sizeof b); break;
        }
        cv_text_on(6, y, &AF_S, label, acid.page ? AC_ORANGE : AC_LIME, AC_INK);
        cv_text_on(6, y + 14, &AF_L, b, AC_COPY, AC_INK);
        return;
    }
    acid_panel(y);
}
static void acid_info_screen(void)                                /* HOME: the controls, the credit */
{
    static const char *const LINES[9] = {"PRESETS  STYLE", "ALGORITHM  SOUND", "SELECT  TEMPO", "OCT+ GENERATE  OCT- MUTATE",
                                         "FX  SOUND / DIRTY PAGE", "SCL SCALE  ENV KEY", "LFO  DNA LOCK", "EDIT  LANGUAGE", "HOME  BACK"};
    int32_t i;
    cv_begin(240, 120, AC_INK);
    acid_mark(8, 6, AC_INK);
    cv_text_on(60, 8, &AF_M, "ACID ENGINE", AC_COPY, AC_INK);
    for (i = 0; i < 6; i++) cv_text_on(8, 32 + i * 14, &AF_S, LINES[i], AC_COPY, AC_INK);
    cv_blit(0, 0);
    cv_begin(240, 120, AC_INK);
    for (i = 6; i < 9; i++) cv_text_on(8, 2 + (i - 6) * 14, &AF_S, LINES[i], AC_COPY, AC_INK);
    cv_rect(8, 46, 224, 1, AC_LINE);
    cv_text_on(8, 52, &AF_S, "DARK FUNK DURCAL DESIGN", AC_LIME, AC_INK);
    cv_text_on(8, 66, &AF_S, "DESARROLLADO EN POLARIS SOUND LAB", AC_COPY, AC_INK);
    cv_text_on(8, 80, &AF_S, "MERIDA, YUCATAN, MEXICO", AC_COPY, AC_INK);
    cv_text_on(8, 98, &AF_S, "drophlex.github.io/DFD-ACID-ENGINE", AC_MUTED, AC_INK);
    cv_blit(0, 120);
}
static void acid_draw(void)
{
    const int32_t cur = song.playing ? (int32_t)(trk[0].seq_idx % 16u) : -1;
    const int hot = acid.hot_ms && fm1_ms - (acid.hot_ms & ~1u) < 1500u;
    uint32_t sig = (uint32_t)(cur + 1) * 3u + acid.sty * 7u + acid.snd * 11u + acid.scl * 13u + acid.key * 17u + acid.seed * 19u + acid.lock * 23u +
                   acid.page * 29u + acid.info * 31u + (uint32_t)song.g[G_BPM] * 37u + lang_es * 41u + (uint32_t)hot * 43u + acid.hot_what * 47u +
                   (uint32_t)song.playing * 53u + acid.mine * 59u, s;
    for (s = 0; s < 16u; s++) sig = sig * 31u + trk[0].step[s].note[0] + trk[0].step[s].time * 7u + trk[0].step[s].flags * 13u;
    for (s = 0; s < 8u; s++) sig = sig * 31u + (uint32_t)trk[0].p[P_E0 + s];
    sig = sig * 31u + (uint32_t)trk[0].p[P_M4AMT] + (uint32_t)trk[0].p[P_DETUNE] * 3u;
    char b[24];
    if (!scr_frame()) return;
    if (sig == acid.sig && !ui.force) return;
    acid.sig = sig;
    ui.force = 0;
    if (acid.info) { acid_info_screen(); scr_shown(); return; }
    /* band 1 (y 0..41): the mark, the name, the tempo and the transport */
    cv_begin(240, 42, AC_INK);
    acid_mark(6, 6, AC_INK);
    cv_text_on(58, 4, &AF_M, "ACID ENGINE", AC_COPY, AC_INK);
    cv_text_on(58, 24, &AF_S, acid.mine ? "MY PATTERN" : ACID_STY_NAME[acid.sty], acid.page ? AC_ORANGE : AC_LIME, AC_INK);
    cv_rrect(186, 8, 48, 24, 3, song.playing ? AC_ORANGE : AC_CARBON, AC_INK);
    fmt_int(b, (uint32_t)song.g[G_BPM]);
    cv_text_on(186 + (48 - text_w(&AF_S, b)) / 2, 14, &AF_S, b, song.playing ? AC_INK : AC_COPY, song.playing ? AC_ORANGE : AC_CARBON);
    cv_rect(0, 41, 240, 1, AC_LINE);
    cv_blit(0, 0);
    /* band 2 (y 42..141): the pattern */
    cv_begin(240, 100, AC_INK);
    acid_roll(14, 84, cur);
    cv_text_on(8, 1, &AF_S, ACID_KEY[acid.key % 12u], AC_COPY, AC_INK);
    cv_text_on(8 + text_w(&AF_S, ACID_KEY[acid.key % 12u]) + 5, 1, &AF_S, ACID_SCL_NAME[acid.scl % 3u], AC_MUTED, AC_INK);
    if (acid.lock) { const char *lk = acid.lock == 1u ? "LOCK NOTES" : "LOCK RHYTHM"; cv_text_on(232 - text_w(&AF_S, lk), 1, &AF_S, lk, AC_VIOLET, AC_INK); }
    cv_blit(0, 42);
    /* band 3 (y 142..239): the page (SOUND lime, DIRTY orange with its hazard stripe), the sound's name */
    cv_begin(240, 98, AC_INK);
    if (acid.page) { int32_t x; for (x = 0; x < 240; x += 12) cv_rect(x, 0, 6, 4, AC_ORANGE); }
    else cv_rect(0, 0, 240, 2, AC_LIME);
    cv_text_on(6, 8, &AF_S, acid.page ? "DIRTY LAB" : "SOUND", acid.page ? AC_ORANGE : AC_LIME, AC_INK);
    cv_text_on(232 - text_w(&AF_S, ACID_SND[acid.snd].name), 8, &AF_S, ACID_SND[acid.snd].name, AC_COPY, AC_INK);
    acid_status(28);
    {
        const char *hint = "OCT+ NEW  OCT- MUTATE  FX PAGE";
        cv_text_on((240 - text_w(&AF_S, hint)) / 2, 82, &AF_S, hint, AC_MUTED, AC_INK);
    }
    cv_blit(0, 142);
    scr_shown();
}
static void acid_leds(void)                                       /* PLAY green while playing; FX lit on the DIRTY page; HOME on its page */
{
    uint8_t nl[FM1_NCOL] = {0}, nd[FM1_NCOL] = {0};
    uint32_t c, k;
    static uint8_t ready;
    if (fm1_led_anim_on()) return;
    if (!ready) { led_pos_init(); ready = 1; }
    led_put(nl, panel.btn[B_FX], acid.page);
    led_put(nl, panel.btn[B_LFO], acid.lock != 0u);
    led_put(nl, panel.btn[B_HOME], acid.info);
    for (k = 0; k < NB; k++) led_put(nd, panel.btn[k], 1);
    if (song.playing) { led_clear(nd, panel.btn[B_PLAY]); nl[LED_PLAY_GREEN >> 3] |= (uint8_t)(1u << (LED_PLAY_GREEN & 7u)); }
    for (c = 0; c < FM1_NCOL; c++) { fm1_led_dim[c] = nd[c]; fm1_led_breath[c] = 0; fm1_led_mid[c] = 0; fm1_led[c] = nl[c]; }
}
