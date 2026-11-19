/* SPDX-License-Identifier: GPL-3.0-only
 * DFD ACID: the web version's voice (engine/dfd_acid.h) for DFD ACID ENGINE on the FM-1 (Dark Funk Durcal design, Polaris Sound Lab,
 * Mérida), in Felucca's integer terms. A monophonic bass / lead voice:
 *   VOICE   ACID SAW · ACID SQUARE · DIRTY (three detuned saws and a sub) · FIDGET (two detuned pulses and a saw an octave up)
 *   CUT RES the cutoff and resonance of a 24 dB low-pass: two trapezoidal SVFs in a row (the first resonant), the bass the
 *           resonance takes away given back (as the web's ladder compensates)
 *   ENV DEC the filter's own envelope, restarted by every note that is not slid into, and its decay (30 ms .. 2.5 s); an accent
 *           (velocity >= 120, the step's ACC) shortens it, opens the filter further and climbs over accents in a row
 *   ACC     how far an accent jumps
 *   FILTH   drive before the filter and a sine wavefold after it (the DIRTY LAB's FILTH)
 *   ZAP     each new note starts up to two octaves high and falls (the electro "zap"), in 70 ms
 * WIDTH (the detune of DIRTY / FIDGET) is the track's DTUNE. Slides are Felucca's: LEGATO + GLIDE, a slid step holds its note.
 * Integer maths; the filter's coefficients once per block (CTL), its envelope per sample.
 * State: s[0] s[1] and s[4] s[5] the two filters, s[2] the filter envelope (Q15), s[3] the zap (Q15), s[6] the accent's
 * climb (Q15), s[7] the sub's flip-flop; ph[0..2] the oscillators. */

static const char *const N_DFD_VOICE[] = {"ACID SAW", "ACID SQR", "DIRTY", "FIDGET"};
enum { DV_SAW, DV_SQR, DV_DIRTY, DV_FIDGET };

static void dfdv_note_on(track_t *t, voice_t *v)
{
    (void)t;
    if (!v->ph[1]) { v->ph[1] = 0x55555555u; v->ph[2] = 0xAAAAAAAAu; }   /* free-running, apart: no phasing at the start */
    v->s[2] = 32767;                                             /* the filter's envelope restarts */
    v->s[3] = 32767;                                             /* the zap */
    if (v->vel >= 120u) v->s[6] += (32767 - v->s[6]) * 9 / 16;   /* accents in a row climb (the web's capacitor) */
    else v->s[6] = v->s[6] * 3 / 4;
}

/* the decay of DEC (0..127: 30 ms .. 2.5 s) as a Q16 factor per sample: 1 - 1 / (tau FS), tau growing as 30 ms x 83^(d / 127) */
static uint32_t dfdv_dec(int32_t d)
{
    int32_t ms = 30, k;
    for (k = 0; k < d; k += 16) ms = ms * (k + 16 <= d ? 174 : 100 + (174 - 100) * (d - k) / 16) / 100;   /* x1.74 per 16 steps */
    if (ms > 2500) ms = 2500;
    return 65536u - (uint32_t)(65536u * 1000u / ((uint32_t)ms * (FS / 1000u) + 1u));
}

static void dfdv_render(track_t *t, voice_t *v, int32_t *out, uint32_t n, const vmod_t *m)
{
    const int16_t *p = t->p;
    const uint32_t voice = (uint32_t)clamp(p[P_E0], 0, 3), acc = v->vel >= 120u;
    const int32_t res = p[P_E2], envamt = p[P_E3], accamt = p[P_E5], filth = p[P_E6], zap = p[P_E7];
    int32_t env = v->s[2], zp = v->s[3], ic1 = v->s[0], ic2 = v->s[1], jc1 = v->s[4], jc2 = v->s[5], sub = v->s[7];
    const uint32_t ek = acc ? dfdv_dec(p[P_E4] / 3) : dfdv_dec(p[P_E4]);
    uint32_t ph0 = v->ph[0], ph1 = v->ph[1], ph2 = v->ph[2], inc, inc1, inc2, i;
    tsvf_t f1, f2;
    int32_t cut, width = p[P_DETUNE], drive = 32768 + filth * 640, fold = filth * 200, comp;
    /* pitch: the zap lifts the note by ZAP semitones x the zap envelope, falling in ~70 ms (once per block) */
    {
        int32_t up = zap * 16 * zp >> 15, pitch = clamp(m->pitch16 + up, 0, 2047);
        inc = zap ? pitch_inc((uint32_t)pitch) : m->inc;
        zp -= zp * CTL / (int32_t)(FS * 7u / 100u) + 1; if (zp < 0) zp = 0;
    }
    /* the stack: DIRTY +- 0.25 .. 0.6 semitone (WIDTH), FIDGET +- 0.5 and an octave up */
    {
        uint32_t d = (inc >> 10) * (uint32_t)(voice == DV_FIDGET ? 12 + width / 4 : 4 + width / 6);   /* ~0.1 % of the pitch per unit */
        inc1 = inc + d; inc2 = voice == DV_FIDGET ? inc * 2u : inc - d;
    }
    /* the filter: CUT + the envelope (ENV, more on an accent, climbing over accents in a row) + Felucca's modulation */
    {
        int32_t e = env * (envamt + (acc ? accamt / 2 : 0)) >> 15;              /* 0..190 */
        cut = (p[P_E1] << 8) + e * 6 * 8 + (acc ? (accamt * v->s[6] >> 15) * 3 * 8 : 0) + m->cutoff;
        tsvf_coef(&f1, cut, res - (acc ? accamt / 8 : 0));
        tsvf_coef(&f2, cut + 768, 0);
        comp = 32768 + res * 150;                                               /* the bass resonance takes, given back */
    }
    for (i = 0; i < n; i++) {
        int32_t s, y;
        switch (voice) {
        case DV_SQR: s = osc_pulse(ph0, inc, 0x80000000u); break;
        case DV_DIRTY: s = (osc_saw(ph0, inc) + osc_saw(ph1, inc1) + osc_saw(ph2, inc2)) * 11 >> 5; break;
        case DV_FIDGET: s = (osc_pulse(ph0, inc, 0x80000000u) + osc_pulse(ph1, inc1, 0x60000000u)) * 9 >> 4; s += osc_saw(ph2, inc2) * 5 >> 4; break;
        default: s = osc_saw(ph0, inc); break;
        }
        if (ph0 + inc < ph0) sub = !sub;                                          /* a square an octave down */
        if (voice == DV_DIRTY) s += sub ? 9000 : -9000;
        ph0 += inc; ph1 += inc1; ph2 += inc2;
        if (filth) s = softclip(((s >> 2) * (drive >> 2)) >> 11);              /* 1x .. 3.5x into the filter */
        s = (s >> 1) * (comp >> 4) >> 11;
        y = tsvf_lp(&f1, s >> 1, &ic1, &ic2);
        { int32_t a = y < 0 ? -y : y; if (a > 16000) { a = 16000 + (softclip((a - 16000) * 2) >> 1); y = y < 0 ? -a : a; } }
        y = tsvf_lp(&f2, y, &jc1, &jc2);
        if (fold) { int32_t z = sine_i((uint32_t)(y * (8 + (fold >> 9))) << 14); y += (z - y) * fold >> 15; }   /* the fold */
        out[i] += voice_amp(clamp(y * 2, -32767, 32767), m, i) * 2;
        env -= (int32_t)(((uint32_t)env * (65536u - ek)) >> 16) + (env > 0);
    }
    v->ph[0] = ph0; v->ph[1] = ph1; v->ph[2] = ph2;
    v->s[0] = ic1; v->s[1] = ic2; v->s[4] = jc1; v->s[5] = jc2; v->s[2] = env < 0 ? 0 : env; v->s[3] = zp; v->s[7] = sub;
}

/* the web version's sounds (acid_tables.h ACID_SND, from web/pattern.js) start here; these are the engine's own presets */
static const preset_t DFDV_PRESETS[] = {
    {"ACID SAW", {0, 53, 102, 79, 69, 83, 12, 0}, {0, 70, 100, 12}, 0, 1, FX(0, 0, 0, 0), PAT(0)},
    {"ACID SQR", {1, 60, 96, 72, 60, 83, 20, 0}, {0, 70, 100, 12}, 0, 1, FX(0, 0, 0, 0), PAT(0)},
    {"DIRTY", {2, 85, 44, 70, 52, 83, 95, 5}, {0, 70, 100, 12}, 0, 1, FX(0, 0, 0, 0), PAT(0)},
    {"FIDGET", {3, 81, 70, 76, 45, 83, 41, 12}, {0, 70, 100, 12}, 0, 1, FX(0, 0, 0, 0), PAT(0)},
};

static const engine_t ENG_DFDACID = {
    .name = "DFD ACID",
    .page_title = {"VOICE", "DIRT"},
    .edit = {
        {"VOICE", F_ENUM, 0, 3, 0, N_DFD_VOICE, 0},
        {"CUT", F_CUTOFF, 0, 127, 53, 0, 0},
        {"RES", F_PCT, 0, 127, 100, 0, 0},
        {"ENV", F_PCT, 0, 127, 80, 0, 0},
        {"DEC", F_PCT, 0, 127, 69, 0, 0},
        {"ACC", F_PCT, 0, 127, 83, 0, 0},
        {"FILTH", F_PCT, 0, 127, 0, 0, 0},
        {"ZAP", F_INT, 0, 24, 0, 0, "st"},
    },
    .presets = DFDV_PRESETS,
    .npresets = NELEM(DFDV_PRESETS),
    .note_on = dfdv_note_on,
    .render = dfdv_render,
    .knob = {P_E1, P_E2, P_E3, P_E4},
    .poly = 1,
    .keep = 0xF3u,                       /* a retrigger keeps the filters and the accent's climb */
    .mem = 0,
};
