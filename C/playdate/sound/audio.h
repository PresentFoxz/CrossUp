// Audio manager for CrossUp (Playdate)

#ifndef AUDIO_H
#define AUDIO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "pocketmod_player.h"

#define MAX_SOUNDS 128
#define MAX_MUSIC_TRACKS 8

typedef enum {
    AUDIO_MUSIC,
    AUDIO_SFX,
    AUDIO_VOX,
    AUDIO_AMBIENCE
} AudioType;

typedef struct {
    FilePlayer* player;
    bool isLooping;
    bool isPlaying;
    char path[256];
} MusicTrack;

typedef struct {
    SamplePlayer* player;
    AudioSample* sample;
    bool isPlaying;
} SFXResource;

typedef struct {
    // Music
    MusicTrack music[MAX_MUSIC_TRACKS];
    int currentMusicIndex;
    size_t musicCount;

    // Sound effects
    SFXResource sounds[MAX_SOUNDS];
    size_t soundCount;

    // Volume settings
    float masterVolume;
    float musicVolume;
    float sfxVolume;
    float voxVolume;
    float ambienceVolume;

    // Audio enabled flags
    bool isMusicPlaying;
    bool isMusicEnabled;
    bool isSfxEnabled;
    bool isVoxEnabled;
    bool isAmbienceEnabled;

    // Music fading
    bool isFadingOut;
    float fadeOutDuration;
    float fadeOutTimer;
    float fadeStartVolume;

    // Pending music
    char pendingMusicPath[256];
    float pendingMusicVolume;
    bool pendingMusicLoop;
    float pendingMusicDelay;
    float pendingMusicDelayTimer;
    bool isPendingMusicDelayed;

    // Module player (MOD/XM/S3M/IT via PocketMod)
    PocketModPlayer modPlayer;
    float modVolume;
    bool isModEnabled;

    bool initialized;
} AudioManager;

extern AudioManager audioManager;

// Init/Update/Unload
void InitAudioManager(AudioManager* manager);
void UpdateAudioManager(AudioManager* manager, float deltaTime);
void UnloadAudioManager(AudioManager* manager);

// Music
bool PlayMusic(AudioManager* manager, const char* path, float volume, bool loop, float delay);
void StopMusic(AudioManager* manager);
void PauseMusic(AudioManager* manager);
void ResumeMusic(AudioManager* manager);
void FadeOutMusic(AudioManager* manager, float duration);
bool IsMusicPlaying(AudioManager* manager);

// SFX
bool PlaySFX(AudioManager* manager, const char* path, float volume);
bool LoadSFX(AudioManager* manager, const char* path);
void StopAllSFX(AudioManager* manager);

// Volume
void AudioSetMasterVolume(AudioManager* manager, float volume);
void AudioSetMusicVolume(AudioManager* manager, float volume);
void SetSFXVolume(AudioManager* manager, float volume);
void SetVoxVolume(AudioManager* manager, float volume);
void SetAmbienceVolume(AudioManager* manager, float volume);

float AudioGetMasterVolume(AudioManager* manager);
float GetMusicVolume(AudioManager* manager);
float GetSFXVolume(AudioManager* manager);
float GetVoxVolume(AudioManager* manager);
float GetAmbienceVolume(AudioManager* manager);

// Enable/disable
void SetMusicEnabled(AudioManager* manager, bool enabled);
void SetSFXEnabled(AudioManager* manager, bool enabled);

// Module (tracker) music
bool PlayModuleMusic(AudioManager* manager, const char* path);
void StopModuleMusic(AudioManager* manager);
void PauseModuleMusic(AudioManager* manager);
void ResumeModuleMusic(AudioManager* manager);
void RestartModuleMusic(AudioManager* manager);
bool IsModulePlaying(AudioManager* manager);
void SetModuleVolume(AudioManager* manager, float volume);
float GetModuleVolume(AudioManager* manager);
const char* GetModuleName(AudioManager* manager);
int GetModuleChannels(AudioManager* manager);

#endif // AUDIO_H
