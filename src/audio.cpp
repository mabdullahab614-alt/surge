#include "audio.h"
#include "globals.h"
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <algorithm>

// Triangle wave (softer timbre than square)
static float triWave(float ph) {
    float p = fmodf(ph * (float)(1. / (2. * 3.14159265)), 1.f);
    if (p < 0.f) p += 1.f;
    if (p < 0.25f) return 4.f * p;
    if (p < 0.75f) return 2.f - 4.f * p;
    return 4.f * p - 4.f;
}

// Schroeder multi-tap comb filter reverb
static void applyReverb(short* d, int n) {
    const int t1 = 882, t2 = 1470, t3 = 2205;
    const float g1 = 0.38f, g2 = 0.26f, g3 = 0.16f;
    for (int i = t3; i < n; i++) {
        int v = (int)d[i] + (int)(d[i-t1]*g1) + (int)(d[i-t2]*g2) + (int)(d[i-t3]*g3);
        d[i] = (short)std::max(-32767, std::min(32767, v));
    }
}

// Pack raw 16-bit mono PCM into a WAV byte buffer and load as a looping Music stream.
// Avoids any file I/O: LoadMusicStreamFromMemory handles the in-memory WAV.
//
// The buffer is intentionally NOT freed: raylib's miniaudio/drwav backend retains a
// raw pointer into it for streaming (drwav_init_memory doesn't copy). It lives for
// the entire program lifetime and is reclaimed by the OS on exit.
static Music waveToLoopingMusic(short* pcm, int numSamples, int sampleRate = 44100) {
    const int dataBytes = numSamples * 2;
    const int fileSize  = 44 + dataBytes;
    unsigned char* wav  = (unsigned char*)malloc(fileSize);

    auto w16 = [&](int off, int16_t v)    { memcpy(&wav[off], &v, 2); };
    auto w32 = [&](int off, int32_t v)    { memcpy(&wav[off], &v, 4); };
    auto tag = [&](int off, const char* s) { memcpy(&wav[off], s, 4); };

    tag(0,  "RIFF"); w32(4, fileSize - 8); tag(8, "WAVE");
    tag(12, "fmt "); w32(16, 16);
    w16(20, 1);                       // PCM
    w16(22, 1);                       // mono
    w32(24, sampleRate);
    w32(28, sampleRate * 2);          // byte rate
    w16(32, 2);                       // block align
    w16(34, 16);                      // bits per sample
    tag(36, "data"); w32(40, dataBytes);
    memcpy(&wav[44], pcm, dataBytes);

    Music m = LoadMusicStreamFromMemory(".wav", wav, fileSize);
    m.looping = true;
    return m;
}

Sound mkSweep(float f0, float f1, float dur, short vol) {
    int sr = 44100, n = (int)(sr * dur);
    short* d = (short*)malloc(n * 2);
    float ph = 0;
    for (int i = 0; i < n; i++) {
        float t = (float)i / sr, fr = f0 + (f1 - f0) * (t / dur);
        ph += 2.f * PI * fr / sr;
        d[i] = (short)(vol * sinf(ph) * (1.f - t / dur));
    }
    Wave w = {(unsigned)n, (unsigned)sr, 16, 1, d};
    Sound s = LoadSoundFromWave(w);
    free(d);
    return s;
}

Sound mkTone(float f, float dur, short vol) {
    int sr = 44100, n = (int)(sr * dur);
    short* d = (short*)malloc(n * 2);
    for (int i = 0; i < n; i++) {
        float t = (float)i / sr, e = std::min({t / .01f, (dur - t) / .05f, 1.f});
        d[i] = (short)(vol * sinf(2.f * PI * f * t) * e);
    }
    Wave w = {(unsigned)n, (unsigned)sr, 16, 1, d};
    Sound s = LoadSoundFromWave(w);
    free(d);
    return s;
}

// Kill crunch — noise burst + low thump + mid crack; very short, percussive
Sound mkCrunch() {
    int sr = 44100;
    int n  = (int)(sr * 0.07f);
    short* d = (short*)malloc(n * 2);
    for (int i = 0; i < n; i++) {
        float t     = (float)i / sr;
        // white noise crack (very fast decay)
        float noise = ((float)rand() / (float)RAND_MAX * 2.f - 1.f);
        float crack = noise * expf(-t / 0.006f);
        // sub thump
        float thump = sinf(2.f * 3.14159f * 75.f * t) * expf(-t / 0.025f);
        // mid transient
        float snap  = sinf(2.f * 3.14159f * 320.f * t) * expf(-t / 0.009f);
        int v = (int)(crack * 9000 + thump * 14000 + snap * 5000);
        d[i] = (short)std::max(-32767, std::min(32767, v));
    }
    Wave w = {(unsigned)n, (unsigned)sr, 16, 1, d};
    Sound s = LoadSoundFromWave(w); free(d); return s;
}

// Stage-clear stinger — 5-note ascending arpeggio, bright and triumphant
Sound mkStageWin() {
    static const float NOTES[] = {523.3f, 659.3f, 784.f, 1046.5f, 1318.5f};
    float noteDur = 0.10f;
    int sr = 44100;
    int n  = (int)(sr * noteDur * 5);
    short* d = (short*)calloc(n, 2);
    for (int ni = 0; ni < 5; ni++) {
        float fr = NOTES[ni];
        int st = (int)(sr * noteDur * ni);
        int ed = (int)(sr * noteDur * (ni + 1));
        for (int i = st; i < ed && i < n; i++) {
            float t   = (float)(i - st) / sr;
            float env = (t < 0.01f) ? t / 0.01f : 1.f - (t - 0.01f) / (noteDur - 0.01f);
            float mel = triWave(2.f * 3.14159f * fr * t) * 0.7f
                      + sinf(2.f * 3.14159f * fr * t) * 0.3f;
            int v = (int)(14000 * mel * env);
            d[i] = (short)std::max(-32767, std::min(32767, v));
        }
    }
    Wave w = {(unsigned)n, (unsigned)sr, 16, 1, d};
    Sound s = LoadSoundFromWave(w); free(d); return s;
}

// ── BGM generators ────────────────────────────────────────────────────────────
// Each synthesises a melodic loop and returns it as a looping Music stream.

// Level 1 — C major, cheerful; 32 notes with bass voice
Music mkBGM() {
    static const int NM = 32;
    float ns[] = {261.6f,329.6f,392.f,523.3f,392.f,329.6f,349.2f,392.f,
                  523.3f,659.3f,523.3f,392.f,329.6f,261.6f,293.7f,261.6f,
                  392.f,523.3f,659.3f,784.f,659.3f,523.3f,440.f,392.f,
                  329.6f,392.f,523.3f,392.f,349.2f,329.6f,293.7f,261.6f};
    float bs[] = {.2f,.2f,.2f,.4f,.2f,.2f,.2f,.4f,.2f,.2f,.2f,.2f,.2f,.4f,.2f,.4f,
                  .15f,.15f,.15f,.3f,.15f,.15f,.15f,.3f,.2f,.2f,.2f,.2f,.2f,.3f,.2f,.4f};
    float bn[] = {130.8f,130.8f,196.f,130.8f,196.f,130.8f,174.6f,196.f,
                  130.8f,196.f,130.8f,196.f,130.8f,130.8f,196.f,130.8f,
                  196.f,130.8f,196.f,196.f,196.f,130.8f,220.f,196.f,
                  130.8f,196.f,130.8f,196.f,174.6f,130.8f,196.f,130.8f};
    float tot = 0; for (int i = 0; i < NM; i++) tot += bs[i];
    int sr = 44100, n = (int)(sr * tot); short* d = (short*)calloc(n, 2); float t = 0;
    for (int ni = 0; ni < NM; ni++) {
        float fr = ns[ni], bfr = bn[ni], dur = bs[ni] * .85f;
        int st = (int)(t * sr), ed = (int)((t + dur) * sr);
        for (int i = st; i < ed && i < n; i++) {
            float ti = (float)(i - st) / sr, env = 1.f - ti / dur;
            float ph = 2.f * PI * fr * ti, bph = 2.f * PI * bfr * ti;
            float mel = (sinf(ph) > 0 ? 1.f : -1.f) * .6f + triWave(ph) * .4f;
            float bas = (sinf(bph) > 0 ? 1.f : -1.f) * .6f + triWave(bph) * .4f;
            int v = (int)(5000 * mel * env) + (int)(2000 * bas * env * .75f);
            d[i] = (short)std::max(-32767, std::min(32767, v));
        }
        t += bs[ni];
    }
    applyReverb(d, n);
    Music m = waveToLoopingMusic(d, n, sr); free(d); return m;
}

// Level 2 — A minor, descending, darker
Music mkBGM2() {
    static const int NM = 32;
    float ns[] = {440.f,392.f,349.f,330.f,294.f,262.f,247.f,220.f,
                  262.f,294.f,330.f,349.f,330.f,294.f,247.f,220.f,
                  330.f,349.f,392.f,440.f,392.f,349.f,330.f,294.f,
                  262.f,294.f,330.f,220.f,247.f,262.f,294.f,220.f};
    float bs[] = {.2f,.2f,.2f,.3f,.2f,.2f,.2f,.4f,.2f,.2f,.3f,.2f,.2f,.2f,.3f,.4f,
                  .15f,.15f,.15f,.25f,.15f,.15f,.15f,.3f,.2f,.15f,.2f,.3f,.15f,.15f,.25f,.4f};
    float bn[] = {110.f,82.4f,87.3f,82.4f,73.4f,65.4f,82.4f,110.f,
                  65.4f,98.f,110.f,87.3f,110.f,98.f,82.4f,110.f,
                  82.4f,87.3f,98.f,110.f,82.4f,87.3f,82.4f,73.4f,
                  110.f,98.f,82.4f,110.f,98.f,87.3f,82.4f,110.f};
    float tot = 0; for (int i = 0; i < NM; i++) tot += bs[i];
    int sr = 44100, n = (int)(sr * tot); short* d = (short*)calloc(n, 2); float t = 0;
    for (int ni = 0; ni < NM; ni++) {
        float fr = ns[ni], bfr = bn[ni], dur = bs[ni] * .85f;
        int st = (int)(t * sr), ed = (int)((t + dur) * sr);
        for (int i = st; i < ed && i < n; i++) {
            float ti = (float)(i - st) / sr, env = 1.f - ti / dur;
            float ph = 2.f * PI * fr * ti, bph = 2.f * PI * bfr * ti;
            float mel = (sinf(ph) > 0 ? 1.f : -1.f) * .6f + triWave(ph) * .4f;
            float bas = (sinf(bph) > 0 ? 1.f : -1.f) * .6f + triWave(bph) * .4f;
            int v = (int)(5000 * mel * env) + (int)(2000 * bas * env * .75f);
            d[i] = (short)std::max(-32767, std::min(32767, v));
        }
        t += bs[ni];
    }
    applyReverb(d, n);
    Music m = waveToLoopingMusic(d, n, sr); free(d); return m;
}

// Level 3 — fast, urgent
Music mkBGM3() {
    static const int NM = 32;
    float ns[] = {330.f,392.f,440.f,523.f,440.f,392.f,330.f,294.f,
                  349.f,415.f,494.f,523.f,494.f,415.f,349.f,330.f,
                  262.f,294.f,330.f,370.f,415.f,440.f,494.f,523.f,
                  494.f,440.f,415.f,370.f,330.f,294.f,262.f,247.f};
    float bs[] = {.15f,.15f,.15f,.25f,.15f,.15f,.15f,.25f,.15f,.15f,.15f,.25f,.15f,.15f,.15f,.3f,
                  .1f,.1f,.1f,.2f,.1f,.1f,.1f,.2f,.1f,.1f,.1f,.2f,.1f,.1f,.1f,.3f};
    float bn[] = {82.4f,98.f,110.f,130.8f,110.f,98.f,82.4f,73.4f,
                  87.3f,103.8f,123.5f,130.8f,123.5f,103.8f,87.3f,82.4f,
                  65.4f,73.4f,82.4f,92.5f,103.8f,110.f,123.5f,130.8f,
                  123.5f,110.f,103.8f,92.5f,82.4f,73.4f,65.4f,61.7f};
    float tot = 0; for (int i = 0; i < NM; i++) tot += bs[i];
    int sr = 44100, n = (int)(sr * tot); short* d = (short*)calloc(n, 2); float t = 0;
    for (int ni = 0; ni < NM; ni++) {
        float fr = ns[ni], bfr = bn[ni], dur = bs[ni] * .8f;
        int st = (int)(t * sr), ed = (int)((t + dur) * sr);
        for (int i = st; i < ed && i < n; i++) {
            float ti = (float)(i - st) / sr, env = 1.f - ti / dur;
            float ph = 2.f * PI * fr * ti, bph = 2.f * PI * bfr * ti;
            float mel = (sinf(ph) > 0 ? 1.f : -1.f) * .6f + triWave(ph) * .4f;
            float bas = (sinf(bph) > 0 ? 1.f : -1.f) * .6f + triWave(bph) * .4f;
            int v = (int)(5200 * mel * env) + (int)(2500 * bas * env * .8f);
            d[i] = (short)std::max(-32767, std::min(32767, v));
        }
        t += bs[ni];
    }
    applyReverb(d, n);
    Music m = waveToLoopingMusic(d, n, sr); free(d); return m;
}

// Level 4 — D minor, volcanic, driving 8th-note pulse
Music mkBGM4() {
    static const int NM = 32;
    float ns[] = {293.7f,329.6f,349.2f,392.f,440.f,392.f,349.2f,329.6f,
                  293.7f,261.6f,233.1f,220.f,196.f,220.f,233.1f,261.6f,
                  293.7f,349.2f,440.f,523.3f,466.2f,392.f,349.2f,329.6f,
                  293.7f,329.6f,349.2f,392.f,440.f,466.2f,523.3f,293.7f};
    float bs[] = {.12f,.12f,.12f,.12f,.12f,.12f,.12f,.24f,
                  .12f,.12f,.12f,.12f,.12f,.12f,.12f,.24f,
                  .10f,.10f,.10f,.20f,.10f,.10f,.10f,.20f,
                  .10f,.10f,.10f,.10f,.10f,.10f,.20f,.40f};
    float bn[] = {73.4f,73.4f,110.f,73.4f,98.f,73.4f,87.3f,73.4f,
                  65.4f,65.4f,87.3f,65.4f,82.4f,65.4f,87.3f,65.4f,
                  73.4f,87.3f,110.f,73.4f,87.3f,98.f,87.3f,73.4f,
                  65.4f,73.4f,87.3f,98.f,87.3f,73.4f,65.4f,73.4f};
    float tot = 0; for (int i = 0; i < NM; i++) tot += bs[i];
    int sr = 44100, n = (int)(sr * tot); short* d = (short*)calloc(n, 2); float t = 0;
    for (int ni = 0; ni < NM; ni++) {
        float fr = ns[ni], bfr = bn[ni], dur = bs[ni] * .82f;
        int st = (int)(t * sr), ed = (int)((t + dur) * sr);
        for (int i = st; i < ed && i < n; i++) {
            float ti = (float)(i - st) / sr, env = 1.f - ti / dur;
            float ph = 2.f * PI * fr * ti, bph = 2.f * PI * bfr * ti;
            float mel = (sinf(ph) > 0 ? 1.f : -1.f) * .6f + triWave(ph) * .4f;
            float bas = (sinf(bph) > 0 ? 1.f : -1.f) * .6f + triWave(bph) * .4f;
            int v = (int)(5200 * mel * env) + (int)(2400 * bas * env * .8f);
            d[i] = (short)std::max(-32767, std::min(32767, v));
        }
        t += bs[ni];
    }
    applyReverb(d, n);
    Music m = waveToLoopingMusic(d, n, sr); free(d); return m;
}

// Level 5 — A major, crystalline, flowing arpeggios
Music mkBGM5() {
    static const int NM = 32;
    float ns[] = {440.f,554.4f,659.3f,880.f,830.6f,659.3f,554.4f,440.f,
                  493.9f,587.3f,740.f,880.f,740.f,587.3f,493.9f,440.f,
                  523.3f,659.3f,784.f,1046.5f,880.f,784.f,659.3f,523.3f,
                  440.f,523.3f,659.3f,784.f,880.f,784.f,659.3f,440.f};
    float bs[] = {.18f,.18f,.18f,.36f,.18f,.18f,.18f,.36f,
                  .15f,.15f,.15f,.30f,.15f,.15f,.15f,.30f,
                  .15f,.15f,.15f,.25f,.15f,.15f,.15f,.25f,
                  .20f,.15f,.15f,.15f,.25f,.15f,.15f,.40f};
    float bn[] = {55.f,82.4f,110.f,55.f,82.4f,110.f,55.f,82.4f,
                  61.7f,92.5f,123.5f,61.7f,92.5f,123.5f,61.7f,92.5f,
                  65.4f,82.4f,110.f,65.4f,82.4f,110.f,65.4f,82.4f,
                  55.f,69.3f,82.4f,110.f,82.4f,69.3f,55.f,55.f};
    float tot = 0; for (int i = 0; i < NM; i++) tot += bs[i];
    int sr = 44100, n = (int)(sr * tot); short* d = (short*)calloc(n, 2); float t = 0;
    for (int ni = 0; ni < NM; ni++) {
        float fr = ns[ni], bfr = bn[ni], dur = bs[ni] * .88f;
        int st = (int)(t * sr), ed = (int)((t + dur) * sr);
        for (int i = st; i < ed && i < n; i++) {
            float ti = (float)(i - st) / sr, env = ti/dur < .04f ? ti/(dur*.04f) : 1.f - ti/dur;
            float ph = 2.f * PI * fr * ti, bph = 2.f * PI * bfr * ti;
            float mel = triWave(ph) * .7f + (sinf(ph) > 0 ? 1.f : -1.f) * .3f;
            float bas = triWave(bph) * .65f + (sinf(bph) > 0 ? 1.f : -1.f) * .35f;
            int v = (int)(4800 * mel * env) + (int)(2200 * bas * env * .8f);
            d[i] = (short)std::max(-32767, std::min(32767, v));
        }
        t += bs[ni];
    }
    applyReverb(d, n);
    Music m = waveToLoopingMusic(d, n, sr); free(d); return m;
}

// Level 6 — B minor, epic final, powerful and urgent
Music mkBGM6() {
    static const int NM = 32;
    float ns[] = {493.9f,740.f,659.3f,587.3f,554.4f,493.9f,440.f,392.f,
                  370.f,493.9f,554.4f,587.3f,659.3f,740.f,987.8f,880.f,
                  740.f,659.3f,587.3f,493.9f,440.f,370.f,330.f,293.7f,
                  440.f,493.9f,587.3f,659.3f,740.f,880.f,987.8f,493.9f};
    float bs[] = {.18f,.18f,.18f,.18f,.18f,.18f,.18f,.36f,
                  .18f,.18f,.18f,.18f,.18f,.18f,.18f,.36f,
                  .14f,.14f,.14f,.14f,.14f,.14f,.14f,.28f,
                  .14f,.14f,.14f,.14f,.14f,.14f,.18f,.42f};
    float bn[] = {61.7f,61.7f,92.5f,61.7f,61.7f,73.4f,55.f,61.7f,
                  73.4f,61.7f,73.4f,82.4f,61.7f,73.4f,92.5f,61.7f,
                  55.f,61.7f,73.4f,55.f,61.7f,73.4f,55.f,61.7f,
                  55.f,61.7f,73.4f,82.4f,73.4f,61.7f,55.f,61.7f};
    float tot = 0; for (int i = 0; i < NM; i++) tot += bs[i];
    int sr = 44100, n = (int)(sr * tot); short* d = (short*)calloc(n, 2); float t = 0;
    for (int ni = 0; ni < NM; ni++) {
        float fr = ns[ni], bfr = bn[ni], dur = bs[ni] * .80f;
        int st = (int)(t * sr), ed = (int)((t + dur) * sr);
        for (int i = st; i < ed && i < n; i++) {
            float ti = (float)(i - st) / sr, env = 1.f - ti / dur;
            float ph = 2.f * PI * fr * ti, bph = 2.f * PI * bfr * ti;
            float mel = (sinf(ph) > 0 ? 1.f : -1.f) * .55f + triWave(ph) * .45f;
            float bas = (sinf(bph) > 0 ? 1.f : -1.f) * .55f + triWave(bph) * .45f;
            int v = (int)(5500 * mel * env) + (int)(2800 * bas * env * .85f);
            d[i] = (short)std::max(-32767, std::min(32767, v));
        }
        t += bs[ni];
    }
    applyReverb(d, n);
    Music m = waveToLoopingMusic(d, n, sr); free(d); return m;
}

// ── Playback control ──────────────────────────────────────────────────────────

void playSFX(Sound s, float pitchVar) {
    SetSoundPitch(s, 1.f + GetRandomValue(-100, 100) / 100.f * pitchVar);
    PlaySound(s);
}

void playLevelBGM() {
    if (!g.audioReady) return;
    for (int i = 1; i <= 6; i++) StopMusicStream(g.bgm[i]);
    PlayMusicStream(g.bgm[g.currentLevel]);
}

bool levelBGMPlaying() {
    if (!g.audioReady) return false;
    return IsMusicStreamPlaying(g.bgm[g.currentLevel]);
}

void stopAllBGM() {
    if (!g.audioReady) return;
    for (int i = 1; i <= 6; i++) StopMusicStream(g.bgm[i]);
}

void pauseAllBGM() {
    if (!g.audioReady) return;
    for (int i = 1; i <= 6; i++) PauseMusicStream(g.bgm[i]);
}

void resumeAllBGM() {
    if (!g.audioReady) return;
    for (int i = 1; i <= 6; i++) ResumeMusicStream(g.bgm[i]);
}

// Must be called every frame to stream whichever BGM tracks are playing
void updateActiveBGM() {
    if (!g.audioReady) return;
    for (int i = 1; i <= 6; i++)
        if (IsMusicStreamPlaying(g.bgm[i])) UpdateMusicStream(g.bgm[i]);
}
