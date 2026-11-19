/* SPDX-License-Identifier: GPL-3.0-only */
/* DFD ACID ENGINE (src/dfdacid.c) on the real UI sources (with the stubs of tests/ui_test.c): the patterns are the web version's
 * (web/pattern.js through acid_tables.h) in every style, scale and seed; the styles load their sound and tempo; every control does
 * what the screen says; the power-on recovery brings every setting back; the screen draws in every state. */
#define UI_TEST_NO_MAIN 1
#include "ui_test.c"

static void aframe(void) { fm1_ms += 16u; acid_input(); andro_poll(); acid_draw(); acid_leds(); }
static void atap(uint32_t label) { host_pressed |= 1u << panel.btn[label]; aframe(); }
static void aturn(uint32_t role, int32_t s) { host_enc[panel.enc[role]] += s * panel.dir[role]; aframe(); host_ticks += 200000u; }

static int acid_frame_ok(void) { return 1; }
static int same_as_table(uint32_t sty, uint32_t scl, uint32_t seed)
{
    const uint16_t *pat = ACID_PAT[sty][scl][seed];
    uint32_t s, ok = 1;
    for (s = 0; s < 16u; s++) {
        const step_t *st = &trk[0].step[s];
        const uint32_t f = pat[s] >> 8;
        if (!(f & 1u)) ok &= st->time == ST_REST || (st->time != ST_NOTE && st->time != ST_TIE);
        else if (f & 8u) ok &= st->time == ST_TIE;
        else {
            ok &= st->time == ST_NOTE && st->note[0] == (uint8_t)(acid_root() + (int8_t)(pat[s] & 255u));
            ok &= !!(st->flags & SF_ACCENT) == !!(f & 2u) && !!(st->flags & SF_SLIDE) == !!(f & 4u) && step_ratchet(st) == ((f >> 4) & 3u) + 1u;
            ok &= st->vel == ((f & 2u) ? 127 : 100);
        }
    }
    return (int)ok;
}
static int web_patterns(void)
{
    uint32_t a, b, k, ok = 1, n = 0;
    acid.key = 0; acid.lock = 0;
    for (a = 0; a < ACID_NSTY; a++)
        for (b = 0; b < ACID_NSCL; b++)
            for (k = 0; k < ACID_NSEED; k++) {
                acid.sty = (uint8_t)a; acid.scl = (uint8_t)b; acid.seed = (uint8_t)k;
                acid_write(0);
                ok &= same_as_table(a, b, k) && trk[0].p[P_SLEN] == 16 && trk[0].p[P_SDIV] == 2;
                ok &= trk[0].step[0].time == ST_NOTE && trk[0].step[0].note[0] % 12u == 0u;   /* the web's lines start on the root */
                if (!ok) { printf("  style %u scale %u seed %u\n", a, b, k); return check("web patterns", 0); }
                n++;
            }
    printf("  %u patterns checked against the web version\n", n);
    return check("every style x scale x seed: the web generator's notes, accents, slides, ties and ratchets; 1/16, 16 steps; on the root", (int)ok);
}
static int styles(void)
{
    uint32_t i, v, ok = 1;
    for (i = 0; i < ACID_NSTY; i++) {
        acid_style(i, 1);
        const acid_sound_t *d = &ACID_SND[ACID_STY_SOUND[i]];
        ok &= trk[0].eng_req == ENGI_DFDACID && trk[0].p[P_E0] == d->voice && trk[0].p[P_E1] == d->cut && trk[0].p[P_E6] == d->filth;
        ok &= trk[0].p[P_VOICE] == V_LEGATO && trk[0].p[P_M4AMT] == d->pump / 2 && trk[0].p[P_M4DST] == 4 && song.g[G_BPM] == ACID_STY_BPM[i];
        for (v = 1; v < NTRK; v++) ok &= trk[v].p[P_LEVEL] == 0;
        ok &= song.sel == 0 && !acid.mine;
        if (!ok) { printf("  style %s\n", ACID_STY_NAME[i]); break; }
    }
    return check("the 16 styles: the DFD ACID voice with the style's sound, its tempo, legato slides, the pump; tracks 2..4 silent", (int)ok);
}
static int controls(void)
{
    int bad = 0;
    uint32_t sty, snd, seed, s, ok;
    int16_t was;
    acid.scl = 0; acid.key = 0; acid.lock = 0; acid_style(0, 1); acid.booted = 1;
    sty = acid.sty; aturn(EN_PRESET, 1); bad += check("PRESETS: the next style", acid.sty == (sty + 1u) % ACID_NSTY && acid_frame_ok());
    snd = acid.snd; aturn(EN_ALGO, 1); bad += check("ALGORITHM: the next sound (the pattern stays)", acid.snd == (snd + 1u) % ACID_NSND);
    was = song.g[G_BPM]; aturn(EN_SELECT, 3); bad += check("SELECT: the tempo", song.g[G_BPM] == was + 3);
    seed = acid.seed; atap(B_OCTUP); bad += check("OCT+: GENERATE, another seed of the style, from the table", acid.seed != seed && same_as_table(acid.sty, acid.scl, acid.seed));
    atap(B_OCTDN); bad += check("OCT-: MUTATE, MY PATTERN, step 1 kept", acid.mine && trk[0].step[0].time == ST_NOTE);
    atap(B_SCL); bad += check("SCL: the next scale, the seed in it", acid.scl == 1u && same_as_table(acid.sty, 1, acid.seed));
    { uint8_t n0 = trk[0].step[0].note[0]; atap(B_ENV); bad += check("ENV: the key, the pattern moved with it", acid.key == 1u && trk[0].step[0].note[0] == n0 + 1u); }
    was = trk[0].p[P_E1]; aturn(EN_K1, 2); bad += check("SOUND page K1: CUT", trk[0].p[P_E1] == was + 4);
    atap(B_FX); bad += check("FX: the DIRTY page", acid.page == 1u);
    aturn(EN_K2, 5); bad += check("DIRTY page K2: PUMP is the LFO on AMP (1/4 SAW FREE UNI)", trk[0].p[P_M4SRC] == 1 && trk[0].p[P_LSYNC] == 5 && trk[0].p[P_LWAVE] == 2 && trk[0].p[P_LTRIG] == 1 && trk[0].p[P_LPOL] == 1);
    was = trk[0].p[P_E7]; aturn(EN_K3, 30); bad += check("DIRTY page K3: ZAP, up to 24 semitones", trk[0].p[P_E7] == 24 && was <= 24);
    atap(B_LFO); { step_t keep[16]; memcpy(keep, trk[0].step, sizeof keep); atap(B_OCTUP); ok = 1;
        for (s = 0; s < 16u; s++) if (keep[s].time == ST_NOTE && trk[0].step[s].time == ST_NOTE) ok &= keep[s].note[0] == trk[0].step[s].note[0];
        bad += check("LFO: DNA LOCK NOTES, a new seed keeps the notes where both play", acid.lock == 1u && ok); }
    atap(B_LFO); atap(B_LFO); bad += check("  .. LFO twice more: RHYTHM, then OFF", acid.lock == 0u);
    atap(B_PLAY); bad += check("PLAY: the transport", transport_req == 1u); transport_req = 0;
    atap(B_HOME); bad += check("HOME: about (and back)", acid.info == 1u); atap(B_HOME);
    atap(B_EDIT); bad += check("EDIT: Español", lang_es == 1u); atap(B_EDIT);
    return bad;
}
static int recovery(void)
{
    int bad = 0;
    uint32_t i, ok = 1;
    for (i = 0; i < 200u && ok; i++) {
        const uint8_t sty = (uint8_t)(i % ACID_NSTY), snd = (uint8_t)((i * 7u) % ACID_NSND), scl = (uint8_t)(i % 3u), key = (uint8_t)((i * 5u) % 12u), seed = (uint8_t)((i * 11u) % ACID_NSEED), lock = (uint8_t)(i % 3u);
        acid_style(sty, 1); acid_sound(snd); acid.scl = scl; acid.key = key; acid.seed = seed; acid.lock = lock; acid.mine = (uint8_t)(i & 1u); acid_keep();
        memset(&acid, 0, sizeof acid);
        ok &= acid_recover() && acid.sty == sty && acid.snd == snd && acid.scl == scl && acid.key == key && acid.seed == seed && acid.lock == lock && acid.mine == (i & 1u);
        if (!ok) printf("  case %u\n", i);
    }
    bad += check("power-on: 200 settings (style, sound, scale, key, seed, lock, mine) come back from track 4", (int)ok);
    for (i = 0; i < NTRK; i++) set_engine_of(&trk[i], 0);
    memset(&acid, 0, sizeof acid); ok = !acid_recover();
    bad += check("  .. other music (not DFD ACID's): not recovered, the first style loads instead", (int)ok);
    return bad;
}
static int screens(void)
{
    uint32_t i, ok = 1;
    acid_style(8, 1); song.playing = 1;
    for (i = 0; i < 40u; i++) {
        acid.page = (uint8_t)(i & 1u); acid.info = (uint8_t)(i % 7u == 6u); lang_es = (uint8_t)((i >> 1) & 1u);
        trk[0].seq_idx = (uint16_t)i; acid.hot_ms = i % 3u ? fm1_ms | 1u : 0; acid.hot_what = i % 18u;
        acid.sig = ~0u; ui.force = 1; acid_draw();
    }
    song.playing = 0; lang_es = 0; acid.info = 0; acid.hot_ms = 0;
    return check("the screen in 40 states (pages, hot values, info, Spanish, playing): drawn without leaving the canvas", (int)ok);
}
int main(void)
{
    int bad = 0;
    setvbuf(stdout, NULL, _IONBF, 0);
    panel = PANEL_DEFAULT; panel_init();
    bad += web_patterns();
    bad += styles();
    bad += controls();
    bad += recovery();
    bad += screens();
    printf("%s\n", bad ? "DFD ACID TEST FAILED" : "dfd acid test passed");
    return bad != 0;
}
