#ifdef PROFILER_ENABLED

#include "profiler.h"
#include "../allFiles/library.h"   // pd, sW, sH, sW_L, sH_L
#include <string.h>

// ---------------------------------------------------------------------------
// Tuning constants
// ---------------------------------------------------------------------------

#define PROF_MAX_TIMERS    16       // hard cap on distinct timer names
#define PROF_NAME_LEN      16       // max chars per name (including '\0')
#define PROF_LOG_INTERVAL  60       // console dump every N frames (~2 s @ 30 fps)
#define PROF_AVG_ALPHA     0.1f     // EMA weight for the current frame
#define PROF_BUDGET_MS     33.33f   // one-frame budget at 30 FPS

// Bar chart geometry — sits just below the FPS counter in the top-right.
#define BAR_MAX_W   96   // pixels wide when avg == PROF_BUDGET_MS (100 %)
#define BAR_H        6   // height of a single bar in pixels
#define BAR_GAP      2   // vertical gap between bars
#define BAR_PAD      3   // horizontal padding inside the white panel
#define BAR_PANEL_X (sW - BAR_MAX_W - BAR_PAD * 2 - 2)
#define BAR_PANEL_Y  22  // just below the top-left FPS counter

// ---------------------------------------------------------------------------
// Internal state
// ---------------------------------------------------------------------------

typedef struct {
    char     name[PROF_NAME_LEN];
    uint32_t startMs;   // absolute ms when Profiler_Begin() was called
    float    frameMs;   // accumulated ms this frame
    float    avgMs;     // exponential moving average
    float    peakMs;    // all-time peak for this timer
} ProfTimer;

static ProfTimer s_timers[PROF_MAX_TIMERS];
static int       s_timerCount  = 0;
static uint32_t  s_frameStart  = 0;
static int       s_frameCount  = 0;
static float     s_frameAvgMs  = 0.0f;
static float     s_framePeakMs = 0.0f;

// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------

static ProfTimer* s_findOrCreate(const char* name) {
    for (int i = 0; i < s_timerCount; i++) {
        if (strncmp(s_timers[i].name, name, PROF_NAME_LEN - 1) == 0)
            return &s_timers[i];
    }
    if (s_timerCount >= PROF_MAX_TIMERS) return NULL;

    ProfTimer* t = &s_timers[s_timerCount++];
    strncpy(t->name, name, PROF_NAME_LEN - 1);
    t->name[PROF_NAME_LEN - 1] = '\0';
    t->startMs = 0;
    t->frameMs = 0.0f;
    t->avgMs   = 0.0f;
    t->peakMs  = 0.0f;
    return t;
}

// Clamp integer to [0, max].
static inline int s_clampI(int v, int max) { return v < 0 ? 0 : (v > max ? max : v); }

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

void Profiler_Init(void) {
    memset(s_timers, 0, sizeof(s_timers));
    s_timerCount  = 0;
    s_frameCount  = 0;
    s_frameAvgMs  = 0.0f;
    s_framePeakMs = 0.0f;
}

void Profiler_FrameBegin(void) {
    if (!pd) return;
    s_frameStart = pd->system->getCurrentTimeMilliseconds();
    for (int i = 0; i < s_timerCount; i++)
        s_timers[i].frameMs = 0.0f;
}

void Profiler_FrameEnd(void) {
    if (!pd) return;

    float frameMs = (float)(pd->system->getCurrentTimeMilliseconds() - s_frameStart);

    // Update frame-level EMA and peak.
    if (s_frameCount == 0) {
        s_frameAvgMs  = frameMs;
        s_framePeakMs = frameMs;
    } else {
        s_frameAvgMs = s_frameAvgMs * (1.0f - PROF_AVG_ALPHA) + frameMs * PROF_AVG_ALPHA;
        if (frameMs > s_framePeakMs) s_framePeakMs = frameMs;
    }

    // Update per-timer EMAs and peaks.
    for (int i = 0; i < s_timerCount; i++) {
        ProfTimer* t = &s_timers[i];
        if (s_frameCount == 0) {
            t->avgMs  = t->frameMs;
            t->peakMs = t->frameMs;
        } else {
            t->avgMs = t->avgMs * (1.0f - PROF_AVG_ALPHA) + t->frameMs * PROF_AVG_ALPHA;
            if (t->frameMs > t->peakMs) t->peakMs = t->frameMs;
        }
    }

    s_frameCount++;

    if ((s_frameCount % PROF_LOG_INTERVAL) == 0)
        Profiler_Log();
}

void Profiler_Begin(const char* name) {
    if (!pd) return;
    ProfTimer* t = s_findOrCreate(name);
    if (t) t->startMs = pd->system->getCurrentTimeMilliseconds();
}

void Profiler_End(const char* name) {
    if (!pd) return;
    uint32_t now = pd->system->getCurrentTimeMilliseconds();
    for (int i = 0; i < s_timerCount; i++) {
        if (strncmp(s_timers[i].name, name, PROF_NAME_LEN - 1) == 0) {
            s_timers[i].frameMs += (float)(now - s_timers[i].startMs);
            return;
        }
    }
}

void Profiler_Log(void) {
    if (!pd) return;

    pd->system->logToConsole("-- PROF [frame %d]  budget %.1fms --",
                             s_frameCount, PROF_BUDGET_MS);
    pd->system->logToConsole("  FRAME      avg=%5.1fms  peak=%5.1fms  (%3.0f%%)",
                             s_frameAvgMs, s_framePeakMs,
                             s_frameAvgMs / PROF_BUDGET_MS * 100.0f);
    for (int i = 0; i < s_timerCount; i++) {
        ProfTimer* t = &s_timers[i];
        pd->system->logToConsole("  %-10s avg=%5.1fms  peak=%5.1fms  (%3.0f%%)",
                                 t->name,
                                 t->avgMs, t->peakMs,
                                 t->avgMs / PROF_BUDGET_MS * 100.0f);
    }
    pd->system->logToConsole("----");
}

// ---------------------------------------------------------------------------
// Visual overlay — proportional bar chart, top-right corner, no font needed.
//
// Layout (top-right):
//   ┌──────────────────────┐
//   │ FRAME  [████████░░░] │   ← full-frame avg
//   │ audio  [█░░░░░░░░░░] │
//   │ update [███░░░░░░░░] │
//   │ draw   [████████░░░] │
//   │   sort [█████░░░░░░] │   ← sub-timers indented by 2px
//   │  raster[████████░░░] │
//   │ submit [████░░░░░░░] │
//   │ upscale[███░░░░░░░░] │
//   └──────────────────────┘
//
// Full-width bar = PROF_BUDGET_MS.  Overbudget bars are clamped to full width.
// A white panel is drawn first so bars are legible over the 3D scene.
// ---------------------------------------------------------------------------

void Profiler_Draw(void) {
    if (!pd) return;

    int rows   = 1 + s_timerCount;  // frame row + one per named timer
    int panelH = rows * (BAR_H + BAR_GAP) + BAR_GAP;
    int panelW = BAR_MAX_W + BAR_PAD * 2;

    // White background panel.
    pd->graphics->fillRect(BAR_PANEL_X, BAR_PANEL_Y, panelW, panelH, kColorWhite);

    int x = BAR_PANEL_X + BAR_PAD;
    int y = BAR_PANEL_Y + BAR_GAP;

    // Frame bar (solid black).
    int fw = s_clampI((int)(s_frameAvgMs / PROF_BUDGET_MS * BAR_MAX_W + 0.5f), BAR_MAX_W);
    if (fw > 0) pd->graphics->fillRect(x, y, fw, BAR_H, kColorBlack);
    y += BAR_H + BAR_GAP;

    // Per-timer bars.
    for (int i = 0; i < s_timerCount; i++) {
        int bw = s_clampI((int)(s_timers[i].avgMs / PROF_BUDGET_MS * BAR_MAX_W + 0.5f), BAR_MAX_W);
        if (bw > 0) pd->graphics->fillRect(x, y, bw, BAR_H, kColorBlack);
        y += BAR_H + BAR_GAP;
    }

    // Draw a thin black border around the panel.
    pd->graphics->drawRect(BAR_PANEL_X, BAR_PANEL_Y, panelW, panelH, kColorBlack);

    // Mark the 50 % line (half budget) with a 1-pixel-wide vertical grey line.
    int halfX = x + (BAR_MAX_W / 2);
    pd->graphics->drawLine(halfX, BAR_PANEL_Y, halfX, BAR_PANEL_Y + panelH, 1, kColorXOR);
}

#endif // PROFILER_ENABLED