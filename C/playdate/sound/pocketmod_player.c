// PocketMod-based Module Player for Playdate

#include "pocketmod_player.h"
#include "../../allFiles/library.h"
#include <stdlib.h>
#include <string.h>

#define POCKETMOD_IMPLEMENTATION
#include "pocketmod.h"

#define SAMPLE_RATE 44100

static float renderBuffer[512 * 2];
static int g_modDataSize = 0;

static int pocketmodAudioCallback(void* context, int16_t* left, int16_t* right, int len) {
    PocketModPlayer* player = (PocketModPlayer*)context;

    if (!player || !player->loaded || !player->playing || !player->pmodContext) {
        memset(left, 0, len * sizeof(int16_t));
        memset(right, 0, len * sizeof(int16_t));
        return 1;
    }

    pocketmod_context* ctx = (pocketmod_context*)player->pmodContext;

    int samplesRemaining = len;
    int outIndex = 0;

    while (samplesRemaining > 0) {
        int chunkSize = (samplesRemaining > 512) ? 512 : samplesRemaining;
        int bytesNeeded = chunkSize * sizeof(float) * 2;

        int bytesRendered = pocketmod_render(ctx, renderBuffer, bytesNeeded);
        int samplesRendered = bytesRendered / (sizeof(float) * 2);

        if (samplesRendered == 0) {
            for (int i = outIndex; i < len; i++) {
                left[i] = 0;
                right[i] = 0;
            }
            break;
        }

        for (int i = 0; i < samplesRendered; i++) {
            float l = renderBuffer[i * 2];
            float r = renderBuffer[i * 2 + 1];

            if (l > 1.0f) l = 1.0f;
            if (l < -1.0f) l = -1.0f;
            if (r > 1.0f) r = 1.0f;
            if (r < -1.0f) r = -1.0f;

            left[outIndex + i] = (int16_t)(l * 32767.0f);
            right[outIndex + i] = (int16_t)(r * 32767.0f);
        }

        outIndex += samplesRendered;
        samplesRemaining -= samplesRendered;
    }

    player->position = pocketmod_loop_count(ctx);
    return 1;
}

bool InitPocketModPlayer(PocketModPlayer* player) {
    if (!player || !pd) return false;

    memset(player, 0, sizeof(PocketModPlayer));
    player->state = POCKETMOD_STATE_STOPPED;
    player->sampleRate = SAMPLE_RATE;
    player->channels = 2;
    strcpy(player->loadStage, "Initialized");
    strcpy(player->moduleName, "No module loaded");

    pd->system->logToConsole("PocketMod: Player initialized");
    return true;
}

void UnloadPocketModPlayer(PocketModPlayer* player) {
    if (!player) return;

    if (player->soundSource) {
        pd->sound->removeSource(player->soundSource);
        player->soundSource = NULL;
    }

    if (player->pmodContext) {
        free(player->pmodContext);
        player->pmodContext = NULL;
    }

    if (player->modData) {
        free(player->modData);
        player->modData = NULL;
    }

    g_modDataSize = 0;
    player->loaded = false;
    player->playing = false;
    player->state = POCKETMOD_STATE_STOPPED;
    strcpy(player->loadStage, "Unloaded");

    pd->system->logToConsole("PocketMod: Player unloaded");
}

bool LoadModFromPath(PocketModPlayer* player, const char* path) {
    if (!player || !path) return false;

    strcpy(player->loadStage, "Loading file");
    pd->system->logToConsole("PocketMod: Loading %s", path);

    if (player->loaded) {
        UnloadPocketModPlayer(player);
        InitPocketModPlayer(player);
    }

    SDFile* file = pd->file->open(path, kFileRead);
    if (!file) {
        strcpy(player->loadStage, "File open failed");
        pd->system->logToConsole("PocketMod: Failed to open %s", path);
        return false;
    }

    pd->file->seek(file, 0, SEEK_END);
    int fileSize = pd->file->tell(file);
    pd->file->seek(file, 0, SEEK_SET);

    pd->system->logToConsole("PocketMod: File size = %d bytes", fileSize);

    player->modData = pd_malloc(fileSize);
    if (!player->modData) {
        strcpy(player->loadStage, "Memory allocation failed");
        pd->file->close(file);
        return false;
    }

    int bytesRead = pd->file->read(file, player->modData, fileSize);
    pd->file->close(file);

    if (bytesRead != fileSize) {
        strcpy(player->loadStage, "File read failed");
        free(player->modData);
        player->modData = NULL;
        return false;
    }

    g_modDataSize = fileSize;
    strcpy(player->loadStage, "Initializing pocketmod");

    player->pmodContext = pd_malloc(sizeof(pocketmod_context));
    if (!player->pmodContext) {
        strcpy(player->loadStage, "Context allocation failed");
        free(player->modData);
        player->modData = NULL;
        return false;
    }

    pocketmod_context* ctx = (pocketmod_context*)player->pmodContext;
    if (!pocketmod_init(ctx, player->modData, fileSize, SAMPLE_RATE)) {
        strcpy(player->loadStage, "Pocketmod init failed");
        pd->system->logToConsole("PocketMod: pocketmod_init failed!");
        free(player->pmodContext);
        free(player->modData);
        player->pmodContext = NULL;
        player->modData = NULL;
        return false;
    }

    strcpy(player->loadStage, "Creating audio source");

    player->soundSource = pd->sound->addSource(pocketmodAudioCallback, player, 1);
    if (!player->soundSource) {
        strcpy(player->loadStage, "Audio source creation failed");
        free(player->pmodContext);
        free(player->modData);
        player->pmodContext = NULL;
        player->modData = NULL;
        return false;
    }

    const char* filename = strrchr(path, '/');
    if (!filename) filename = path;
    else filename++;

    strncpy(player->moduleName, filename, sizeof(player->moduleName) - 1);
    player->moduleName[sizeof(player->moduleName) - 1] = '\0';

    char* ext = strrchr(player->moduleName, '.');
    if (ext) *ext = '\0';

    player->loaded = true;
    player->state = POCKETMOD_STATE_STOPPED;
    player->totalTimeMs = 120000;
    strcpy(player->loadStage, "Loaded successfully");

    pd->system->logToConsole("PocketMod: Successfully loaded %s", player->moduleName);
    return true;
}

void PlayMod(PocketModPlayer* player) {
    if (!player || !player->loaded) return;
    player->playing = true;
    player->state = POCKETMOD_STATE_PLAYING;
    pd->system->logToConsole("PocketMod: Playing %s", player->moduleName);
}

void PauseMod(PocketModPlayer* player) {
    if (!player || !player->loaded) return;
    player->playing = false;
    player->state = POCKETMOD_STATE_PAUSED;
    pd->system->logToConsole("PocketMod: Paused");
}

void ResumeMod(PocketModPlayer* player) {
    if (!player || !player->loaded) return;
    player->playing = true;
    player->state = POCKETMOD_STATE_PLAYING;
    pd->system->logToConsole("PocketMod: Resumed");
}

void RestartMod(PocketModPlayer* player) {
    if (!player || !player->loaded || !player->pmodContext || !player->modData) return;
    pocketmod_context* ctx = (pocketmod_context*)player->pmodContext;
    pocketmod_init(ctx, player->modData, g_modDataSize, SAMPLE_RATE);
    player->position = 0;
    player->row = 0;
    player->timeMs = 0;
    pd->system->logToConsole("PocketMod: Restarted");
}

void UpdatePocketModPlayer(PocketModPlayer* player) {
    if (!player || !player->loaded) return;
    if (player->playing) {
        static uint32_t lastTicks = 0;
        uint32_t currentTicks = pd->system->getCurrentTimeMilliseconds();
        if (lastTicks != 0) {
            player->timeMs += currentTicks - lastTicks;
        }
        lastTicks = currentTicks;
    }
}

bool IsModLoaded(PocketModPlayer* player) {
    return player && player->loaded;
}

bool IsModPlaying(PocketModPlayer* player) {
    return player && player->playing && player->state == POCKETMOD_STATE_PLAYING;
}

bool IsModPaused(PocketModPlayer* player) {
    return player && player->state == POCKETMOD_STATE_PAUSED;
}

PocketModState GetModState(PocketModPlayer* player) {
    return player ? player->state : POCKETMOD_STATE_STOPPED;
}

const char* GetModName(PocketModPlayer* player) {
    return player ? player->moduleName : "Unknown";
}

const char* GetModType(PocketModPlayer* player) {
    return player && player->loaded ? "MOD (PocketMod)" : "Unknown";
}

int GetModChannelCount(PocketModPlayer* player) {
    if (player && player->loaded && player->pmodContext) {
        pocketmod_context* ctx = (pocketmod_context*)player->pmodContext;
        return ctx->num_channels;
    }
    return 0;
}

int GetModCurrentTime(PocketModPlayer* player) {
    return player ? player->timeMs : 0;
}

int GetModTotalTime(PocketModPlayer* player) {
    return player ? player->totalTimeMs : 0;
}

int GetModPosition(PocketModPlayer* player) {
    return player ? player->position : 0;
}

int GetModRow(PocketModPlayer* player) {
    if (player && player->pmodContext) {
        pocketmod_context* ctx = (pocketmod_context*)player->pmodContext;
        return ctx->line;
    }
    return 0;
}

const char* GetModLoadStage(PocketModPlayer* player) {
    return player ? player->loadStage : "Not initialized";
}
