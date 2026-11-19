/* SPDX-License-Identifier: GPL-3.0-only */
/* The DFD ACID voice (src/eng_dfdacid.c) through the firmware's audio path (voice.c, fx.c's mix): every one of the 19 sounds
 * (acid_tables.h, the web version's) sounds, never clips, ends; an accent is louder and brighter; the zap starts high; the
 * resonance rings; PUMP (the track LFO, 1/4 SAW FREE UNI on AMP) ducks every beat. OUT/acid_<n>.raw (int16 mono, FS) to listen. */
#define main hostsim_main
#include "hostsim.c"
#undef main
#include "acid_tables.h"

static void sound(track_t *t, uint32_t i)
{
    const acid_sound_t *d = &ACID_SND[i];
    host_preset(t, ENGI_DFDACID, 0);
    t->p[P_E0] = d->voice; t->p[P_E1] = d->cut; t->p[P_E2] = d->res; t->p[P_E3] = d->env; t->p[P_E4] = d->dec;
    t->p[P_E5] = d->acc; t->p[P_E6] = d->filth; t->p[P_E7] = d->zap; t->p[P_DETUNE] = d->width;
    t->p[P_DIST] = t->p[P_CHOR] = t->p[P_DLY] = t->p[P_REV] = 0;
    t->p[P_LSYNC] = 5; t->p[P_LWAVE] = 2; t->p[P_LTRIG] = 1; t->p[P_LPOL] = 1; t->p[P_M4SRC] = 1; t->p[P_M4DST] = 4; t->p[P_M4AMT] = d->pump / 2;
}
/* n blocks of the mix; a note on at block `on` (velocity vel), off at block `off`; rms, peak, the "brightness" (first difference) */
static void render(int16_t *buf, uint32_t nb, uint32_t note, uint32_t vel, uint32_t on, uint32_t off)
{
    uint32_t f, i;
    for (f = 0; f < nb; f++) {
        int32_t o[2 * CTL];
        if (f == on) trk_note_on(&trk[0], note, vel);
        if (f == off) trk_note_off(&trk[0], note);
        mix_block(o, CTL);
        for (i = 0; i < CTL; i++) buf[f * CTL + i] = (int16_t)clamp(o[2 * i], -32768, 32767);
    }
}
static double rms(const int16_t *b, uint32_t a, uint32_t z) { double s = 0; uint32_t i; for (i = a; i < z; i++) s += (double)b[i] * b[i]; return sqrt(s / (z - a)); }
static double bright(const int16_t *b, uint32_t a, uint32_t z) { double s = 0, e = 0; uint32_t i; for (i = a + 1; i < z; i++) { double d = b[i] - b[i - 1]; s += d * d; e += (double)b[i] * b[i]; } return e > 0 ? s / e : 0; }
static int crossings(const int16_t *b, uint32_t a, uint32_t z) { int c = 0; uint32_t i; for (i = a + 1; i < z; i++) c += (b[i - 1] < 0) != (b[i] < 0); return c; }

int main(int argc, char **argv)
{
    const char *dir = argc > 1 ? argv[1] : ".";
    static int16_t buf[FS * 3];
    const uint32_t NB = FS * 2u / CTL;
    uint32_t i, k, ok = 1, peakmax = 0;
    int bad = 0;
    for (i = 0; i < ACID_NSND; i++) {
        uint32_t pk = 0, endpk = 0;
        host_tracks_init(); song.g[G_BPM] = 120;
        for (k = 1; k < NTRK; k++) trk[k].p[P_LEVEL] = 0;
        sound(&trk[0], i);
        render(buf, NB, 36, 100, 1, NB / 2);
        for (k = 0; k < NB * CTL; k++) { uint32_t a = (uint32_t)abs(buf[k]); if (a > pk) pk = a; if (k > NB * CTL - FS / 8u && a > endpk) endpk = a; }
        if (pk > peakmax) peakmax = pk;
        {
            char path[512]; FILE *w; snprintf(path, sizeof path, "%s/acid_%02u.raw", dir, (unsigned)i);
            if ((w = fopen(path, "wb"))) { fwrite(buf, 2, NB * CTL, w); fclose(w); }
        }
        printf("  %-15s peak %5u  rms %6.0f  end %4u  voices %u\n", ACID_SND[i].name, pk, rms(buf, CTL * 8, FS / 2), endpk, (unsigned)busy_now());
        ok &= pk > 2000u && pk < 32767u && endpk < 300u;
    }
    bad += !ok; printf("dfd acid voice: the 19 sounds sound, never clip, end after the release %s\n", ok ? "ok" : "FAIL");
    /* accent: louder and brighter than the same note at velocity 100 */
    {
        double r1, r2, b1, b2;
        host_tracks_init(); for (k = 1; k < NTRK; k++) trk[k].p[P_LEVEL] = 0; sound(&trk[0], 0); trk[0].p[P_M4AMT] = 0;
        render(buf, 300, 36, 100, 1, 200); r1 = rms(buf, CTL * 4, CTL * 60); b1 = bright(buf, CTL * 4, CTL * 60);
        host_tracks_init(); for (k = 1; k < NTRK; k++) trk[k].p[P_LEVEL] = 0; sound(&trk[0], 0); trk[0].p[P_M4AMT] = 0;
        render(buf, 300, 36, 127, 1, 200); r2 = rms(buf, CTL * 4, CTL * 60); b2 = bright(buf, CTL * 4, CTL * 60);
        ok = r2 > r1 && b2 > b1 * 1.15;
        bad += !ok; printf("dfd acid voice: an accent is louder (%.0f vs %.0f) and brighter (%.4f vs %.4f) %s\n", r2, r1, b2, b1, ok ? "ok" : "FAIL");
    }
    /* zap: the note starts higher */
    {
        int c0, c1;
        host_tracks_init(); for (k = 1; k < NTRK; k++) trk[k].p[P_LEVEL] = 0; sound(&trk[0], 0); trk[0].p[P_M4AMT] = 0;
        trk[0].p[P_E7] = 12; trk[0].p[P_E1] = 127; trk[0].p[P_E2] = 0; trk[0].p[P_E3] = 0;
        render(buf, FS / CTL, 45, 100, 1, FS / CTL - 2);
        c0 = crossings(buf, CTL * 2, CTL * 2 + 1500); c1 = crossings(buf, FS / 2, FS / 2 + 1500);
        ok = c0 > c1 * 13 / 10;
        bad += !ok; printf("dfd acid voice: ZAP 12 starts higher and falls (%d vs %d crossings) %s\n", c0, c1, ok ? "ok" : "FAIL");
    }
    /* pump: at 120 BPM a beat is FS / 2 samples; the start of each beat is quieter than its end */
    {
        double a, z;
        host_tracks_init(); song.g[G_BPM] = 120; for (k = 1; k < NTRK; k++) trk[k].p[P_LEVEL] = 0; sound(&trk[0], 16); trk[0].p[P_M4AMT] = 63;
        trk[0].p[P_E4] = 127; trk[0].p[P_SUS] = 127;
        seq_start();
        render(buf, FS * 3u / CTL, 36, 100, 1, FS * 3u / CTL - 1);
        a = rms(buf, FS + 200, FS + 2400); z = rms(buf, FS + FS / 2 - 4000, FS + FS / 2 - 400);
        ok = a < z * 0.5;
        bad += !ok; printf("dfd acid voice: PUMP ducks with the beat (%.0f at a beat's start vs %.0f at its end) %s\n", a, z, ok ? "ok" : "FAIL");
    }
    printf("%s (loudest peak %u)\n", bad ? "DFD ACID VOICE FAILED" : "dfd acid voice passed", peakmax);
    return bad != 0;
}
