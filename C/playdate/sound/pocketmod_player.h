// PocketMod-based Module Player for Playdate
// Ultra-lightweight MOD player with minimal CPU usage

#ifndef POCKETMOD_PLAYER_H
#define POCKETMOD_PLAYER_H

#include <stdbool.h>
#include <stdint.h>
#include "../../allFiles/library.h"

typedef struct PocketModPlayer PocketModPlayer;

typedef enum {
    POCKETMOD_STATE_STOPPED,
    POCKETMOD_STATE_PLAYING,
    POCKETMOD_STATE_PAUSED
} PocketModState;

bool InitPocketModPlayer(PocketModPlayer* player);
void UnloadPocketModPlayer(PocketModPlayer* player);
bool LoadModFromPath(PocketModPlayer* player, const char* path);
void PlayMod(PocketModPlayer* player);
void PauseMod(PocketModPlayer* player);
void ResumeMod(PocketModPlayer* player);
void RestartMod(PocketModPlayer* player);
void UpdatePocketModPlayer(PocketModPlayer* player);

bool IsModLoaded(PocketModPlayer* player);
bool IsModPlaying(PocketModPlayer* player);
bool IsModPaused(PocketModPlayer* player);
PocketModState GetModState(PocketModPlayer* player);

const char* GetModName(PocketModPlayer* player);
const char* GetModType(PocketModPlayer* player);
int GetModChannelCount(PocketModPlayer* player);
int GetModCurrentTime(PocketModPlayer* player);
int GetModTotalTime(PocketModPlayer* player);
int GetModPosition(PocketModPlayer* player);
int GetModRow(PocketModPlayer* player);
const char* GetModLoadStage(PocketModPlayer* player);

struct PocketModPlayer {
    void* modData;
    void* pmodContext;
    SoundSource* soundSource;
    bool loaded;
    bool playing;
    PocketModState state;
    char moduleName[32];
    char loadStage[32];
    int sampleRate;
    int channels;
    int position;
    int row;
    uint32_t timeMs;
    uint32_t totalTimeMs;
};

#endif // POCKETMOD_PLAYER_H
