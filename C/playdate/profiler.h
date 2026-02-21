// util-profiler.h — lightweight per-section frame profiler for Playdate
//
// Uses pd->system->getCurrentTimeMilliseconds() so it never interferes with
// the game's own getElapsedTime() / resetElapsedTime() delta-time tracking.
//
// Build with -DPROFILER_ENABLED (set PROFILER=1 in make) to activate.
// All macros compile to nothing when the flag is absent.
//
// Usage pattern:
//
//   // main.c update():
//   PROF_FRAME_BEGIN();
//   PROF_BEGIN("audio");   UpdateAudioManager(...); PROF_END("audio");
//   PROF_BEGIN("update");  UpdateScreenManager(...); PROF_END("update");
//   PROF_BEGIN("draw");    DrawScreenManager(...);  PROF_END("draw");
//   PROF_DRAW();           // bar-chart overlay in top-right corner
//   PROF_FRAME_END();      // computes averages; logs every 60 frames

#pragma once

#ifdef PROFILER_ENABLED

// Initialise — call once after pd is valid (e.g. first frame of update()).
void Profiler_Init(void);

// Call at the very start of each frame (before any subsystem work).
void Profiler_FrameBegin(void);

// Call at the very end of each frame.  Computes rolling averages and logs
// to console every PROF_LOG_INTERVAL frames.
void Profiler_FrameEnd(void);

// Start / stop a named timer.  Timers are created on first use (max 16).
// Accumulates if Begin/End are called multiple times in one frame.
void Profiler_Begin(const char* name);
void Profiler_End(const char* name);

// Draw a proportional bar chart in the top-right corner of the display.
// Bars represent the rolling-average time for each section as a fraction
// of the 33 ms frame budget.  Call after DrawScreenManager() completes.
void Profiler_Draw(void);

// Dump all averages + peaks to the simulator / device console.
void Profiler_Log(void);

#define PROF_INIT()         Profiler_Init()
#define PROF_FRAME_BEGIN()  Profiler_FrameBegin()
#define PROF_FRAME_END()    Profiler_FrameEnd()
#define PROF_BEGIN(name)    Profiler_Begin(name)
#define PROF_END(name)      Profiler_End(name)
#define PROF_DRAW()         Profiler_Draw()

#else  // PROFILER_ENABLED not defined — all macros are no-ops

#define PROF_INIT()
#define PROF_FRAME_BEGIN()
#define PROF_FRAME_END()
#define PROF_BEGIN(name)
#define PROF_END(name)
#define PROF_DRAW()

#endif // PROFILER_ENABLED