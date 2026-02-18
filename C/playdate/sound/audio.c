// Audio manager implementation for CrossUp (Playdate)

#include "audio.h"

AudioManager audioManager;

void InitAudioManager(AudioManager* manager) {
    if (manager == NULL || pd == NULL) return;

    memset(manager, 0, sizeof(AudioManager));

    manager->musicCount = 0;
    manager->soundCount = 0;
    manager->currentMusicIndex = -1;

    manager->masterVolume = 1.0f;
    manager->musicVolume = 1.0f;
    manager->sfxVolume = 1.0f;
    manager->voxVolume = 1.0f;
    manager->ambienceVolume = 1.0f;

    manager->isMusicPlaying = false;
    manager->isMusicEnabled = true;
    manager->isSfxEnabled = true;
    manager->isVoxEnabled = true;
    manager->isAmbienceEnabled = true;

    manager->isFadingOut = false;
    manager->fadeOutDuration = 0.0f;
    manager->fadeOutTimer = 0.0f;
    manager->fadeStartVolume = 1.0f;

    manager->pendingMusicPath[0] = '\0';
    manager->pendingMusicVolume = 1.0f;
    manager->pendingMusicLoop = false;
    manager->pendingMusicDelay = 0.0f;
    manager->pendingMusicDelayTimer = 0.0f;
    manager->isPendingMusicDelayed = false;

    InitPocketModPlayer(&manager->modPlayer);
    manager->modVolume = 1.0f;
    manager->isModEnabled = true;

    manager->initialized = true;
}

void UpdateAudioManager(AudioManager* manager, float deltaTime) {
    if (manager == NULL || pd == NULL) return;

    UpdatePocketModPlayer(&manager->modPlayer);

    // Music fading
    if (manager->isFadingOut && manager->currentMusicIndex >= 0) {
        manager->fadeOutTimer += deltaTime;
        float fadeProgress = manager->fadeOutTimer / manager->fadeOutDuration;

        if (fadeProgress >= 1.0f) {
            StopMusic(manager);
            manager->isFadingOut = false;
            manager->fadeOutTimer = 0.0f;

            if (manager->pendingMusicPath[0] != '\0') {
                PlayMusic(manager, manager->pendingMusicPath,
                          manager->pendingMusicVolume,
                          manager->pendingMusicLoop, 0.0f);
                manager->pendingMusicPath[0] = '\0';
            }
        } else {
            float newVolume = manager->fadeStartVolume * (1.0f - fadeProgress);
            MusicTrack* track = &manager->music[manager->currentMusicIndex];
            if (track->player != NULL) {
                float finalVol = newVolume * manager->musicVolume * manager->masterVolume;
                pd->sound->fileplayer->setVolume(track->player, finalVol, finalVol);
            }
        }
    }

    // Pending music delay
    if (manager->isPendingMusicDelayed) {
        manager->pendingMusicDelayTimer += deltaTime;
        if (manager->pendingMusicDelayTimer >= manager->pendingMusicDelay) {
            PlayMusic(manager, manager->pendingMusicPath,
                      manager->pendingMusicVolume,
                      manager->pendingMusicLoop, 0.0f);
            manager->pendingMusicPath[0] = '\0';
            manager->isPendingMusicDelayed = false;
        }
    }
}

void UnloadAudioManager(AudioManager* manager) {
    if (manager == NULL || pd == NULL) return;

    UnloadPocketModPlayer(&manager->modPlayer);

    for (size_t i = 0; i < manager->musicCount; i++) {
        if (manager->music[i].player != NULL) {
            pd->sound->fileplayer->stop(manager->music[i].player);
            pd->sound->fileplayer->freePlayer(manager->music[i].player);
            manager->music[i].player = NULL;
        }
    }

    for (size_t i = 0; i < manager->soundCount; i++) {
        if (manager->sounds[i].player != NULL) {
            pd->sound->sampleplayer->stop(manager->sounds[i].player);
            pd->sound->sampleplayer->freePlayer(manager->sounds[i].player);
            manager->sounds[i].player = NULL;
        }
        if (manager->sounds[i].sample != NULL) {
            pd->sound->sample->freeSample(manager->sounds[i].sample);
            manager->sounds[i].sample = NULL;
        }
    }

    manager->musicCount = 0;
    manager->soundCount = 0;
    manager->currentMusicIndex = -1;
    manager->initialized = false;
}

bool PlayMusic(AudioManager* manager, const char* path, float volume, bool loop, float delay) {
    if (manager == NULL || pd == NULL || path == NULL) return false;
    if (!manager->isMusicEnabled) return false;

    if (delay > 0.0f) {
        strncpy(manager->pendingMusicPath, path, sizeof(manager->pendingMusicPath) - 1);
        manager->pendingMusicPath[sizeof(manager->pendingMusicPath) - 1] = '\0';
        manager->pendingMusicVolume = volume;
        manager->pendingMusicLoop = loop;
        manager->pendingMusicDelay = delay;
        manager->pendingMusicDelayTimer = 0.0f;
        manager->isPendingMusicDelayed = true;
        return true;
    }

    if (manager->isMusicPlaying) {
        StopMusic(manager);
    }

    FilePlayer* player = pd->sound->fileplayer->newPlayer();
    if (player == NULL) {
        pd->system->logToConsole("Audio: Failed to create file player");
        return false;
    }

    if (pd->sound->fileplayer->loadIntoPlayer(player, path) == 0) {
        pd->system->logToConsole("Audio: Failed to load music: %s", path);
        pd->sound->fileplayer->freePlayer(player);
        return false;
    }

    int trackIndex = -1;
    if (manager->musicCount < MAX_MUSIC_TRACKS) {
        trackIndex = (int)manager->musicCount;
        manager->musicCount++;
    } else {
        trackIndex = 0;
        if (manager->music[0].player != NULL) {
            pd->sound->fileplayer->freePlayer(manager->music[0].player);
        }
    }

    manager->music[trackIndex].player = player;
    manager->music[trackIndex].isLooping = loop;
    manager->music[trackIndex].isPlaying = true;
    strncpy(manager->music[trackIndex].path, path, sizeof(manager->music[trackIndex].path) - 1);

    float finalVol = volume * manager->musicVolume * manager->masterVolume;
    pd->sound->fileplayer->setVolume(player, finalVol, finalVol);
    pd->sound->fileplayer->play(player, loop ? 0 : 1);

    manager->currentMusicIndex = trackIndex;
    manager->isMusicPlaying = true;

    return true;
}

void StopMusic(AudioManager* manager) {
    if (manager == NULL || pd == NULL) return;
    if (manager->currentMusicIndex < 0) return;

    MusicTrack* track = &manager->music[manager->currentMusicIndex];
    if (track->player != NULL) {
        pd->sound->fileplayer->stop(track->player);
        track->isPlaying = false;
    }

    manager->isMusicPlaying = false;
    manager->isFadingOut = false;
}

void PauseMusic(AudioManager* manager) {
    if (manager == NULL || pd == NULL) return;
    if (manager->currentMusicIndex < 0 || !manager->isMusicPlaying) return;

    MusicTrack* track = &manager->music[manager->currentMusicIndex];
    if (track->player != NULL) {
        pd->sound->fileplayer->pause(track->player);
    }
}

void ResumeMusic(AudioManager* manager) {
    if (manager == NULL || pd == NULL) return;
    if (manager->currentMusicIndex < 0) return;

    MusicTrack* track = &manager->music[manager->currentMusicIndex];
    if (track->player != NULL) {
        pd->sound->fileplayer->play(track->player, track->isLooping ? 0 : 1);
    }
}

void FadeOutMusic(AudioManager* manager, float duration) {
    if (manager == NULL || !manager->isMusicPlaying) return;
    if (manager->currentMusicIndex < 0) return;

    manager->isFadingOut = true;
    manager->fadeOutDuration = duration;
    manager->fadeOutTimer = 0.0f;
    manager->fadeStartVolume = manager->musicVolume;
}

bool IsMusicPlaying(AudioManager* manager) {
    if (manager == NULL) return false;
    return manager->isMusicPlaying;
}

bool PlaySFX(AudioManager* manager, const char* path, float volume) {
    if (manager == NULL || pd == NULL || path == NULL) return false;
    if (!manager->isSfxEnabled) return false;

    AudioSample* sample = pd->sound->sample->load(path);
    if (sample == NULL) {
        pd->system->logToConsole("Audio: Failed to load sound: %s", path);
        return false;
    }

    SamplePlayer* player = pd->sound->sampleplayer->newPlayer();
    if (player == NULL) {
        pd->sound->sample->freeSample(sample);
        return false;
    }

    pd->sound->sampleplayer->setSample(player, sample);

    float finalVol = volume * manager->sfxVolume * manager->masterVolume;
    pd->sound->sampleplayer->setVolume(player, finalVol, finalVol);
    pd->sound->sampleplayer->play(player, 1, 1.0f);

    if (manager->soundCount < MAX_SOUNDS) {
        manager->sounds[manager->soundCount].player = player;
        manager->sounds[manager->soundCount].sample = sample;
        manager->sounds[manager->soundCount].isPlaying = true;
        manager->soundCount++;
    }

    return true;
}

bool LoadSFX(AudioManager* manager, const char* path) {
    if (manager == NULL || pd == NULL || path == NULL) return false;
    if (manager->soundCount >= MAX_SOUNDS) return false;

    AudioSample* sample = pd->sound->sample->load(path);
    if (sample == NULL) {
        pd->system->logToConsole("Audio: Failed to preload sound: %s", path);
        return false;
    }

    SamplePlayer* player = pd->sound->sampleplayer->newPlayer();
    if (player == NULL) {
        pd->sound->sample->freeSample(sample);
        return false;
    }

    pd->sound->sampleplayer->setSample(player, sample);

    manager->sounds[manager->soundCount].player = player;
    manager->sounds[manager->soundCount].sample = sample;
    manager->sounds[manager->soundCount].isPlaying = false;
    manager->soundCount++;

    return true;
}

void StopAllSFX(AudioManager* manager) {
    if (manager == NULL || pd == NULL) return;

    for (size_t i = 0; i < manager->soundCount; i++) {
        if (manager->sounds[i].player != NULL) {
            pd->sound->sampleplayer->stop(manager->sounds[i].player);
            manager->sounds[i].isPlaying = false;
        }
    }
}

void AudioSetMasterVolume(AudioManager* manager, float volume) {
    if (manager == NULL) return;
    manager->masterVolume = volume;
}

void AudioSetMusicVolume(AudioManager* manager, float volume) {
    if (manager == NULL) return;
    manager->musicVolume = volume;

    if (manager->currentMusicIndex >= 0 && manager->isMusicPlaying) {
        MusicTrack* track = &manager->music[manager->currentMusicIndex];
        if (track->player != NULL) {
            float finalVol = volume * manager->masterVolume;
            pd->sound->fileplayer->setVolume(track->player, finalVol, finalVol);
        }
    }
}

void SetSFXVolume(AudioManager* manager, float volume) {
    if (manager == NULL) return;
    manager->sfxVolume = volume;
}

void SetVoxVolume(AudioManager* manager, float volume) {
    if (manager == NULL) return;
    manager->voxVolume = volume;
}

void SetAmbienceVolume(AudioManager* manager, float volume) {
    if (manager == NULL) return;
    manager->ambienceVolume = volume;
}

float AudioGetMasterVolume(AudioManager* manager) {
    return manager ? manager->masterVolume : 0.0f;
}

float GetMusicVolume(AudioManager* manager) {
    return manager ? manager->musicVolume : 0.0f;
}

float GetSFXVolume(AudioManager* manager) {
    return manager ? manager->sfxVolume : 0.0f;
}

float GetVoxVolume(AudioManager* manager) {
    return manager ? manager->voxVolume : 0.0f;
}

float GetAmbienceVolume(AudioManager* manager) {
    return manager ? manager->ambienceVolume : 0.0f;
}

void SetMusicEnabled(AudioManager* manager, bool enabled) {
    if (manager == NULL) return;
    manager->isMusicEnabled = enabled;
    if (!enabled && manager->isMusicPlaying) {
        StopMusic(manager);
    }
}

void SetSFXEnabled(AudioManager* manager, bool enabled) {
    if (manager == NULL) return;
    manager->isSfxEnabled = enabled;
    if (!enabled) {
        StopAllSFX(manager);
    }
}

bool PlayModuleMusic(AudioManager* manager, const char* path) {
    if (manager == NULL || path == NULL) return false;
    if (!manager->isModEnabled) return false;

    if (!LoadModFromPath(&manager->modPlayer, path)) return false;
    PlayMod(&manager->modPlayer);
    return true;
}

void StopModuleMusic(AudioManager* manager) {
    if (manager == NULL) return;
    UnloadPocketModPlayer(&manager->modPlayer);
    InitPocketModPlayer(&manager->modPlayer);
}

void PauseModuleMusic(AudioManager* manager) {
    if (manager == NULL) return;
    PauseMod(&manager->modPlayer);
}

void ResumeModuleMusic(AudioManager* manager) {
    if (manager == NULL) return;
    ResumeMod(&manager->modPlayer);
}

void RestartModuleMusic(AudioManager* manager) {
    if (manager == NULL) return;
    RestartMod(&manager->modPlayer);
}

bool IsModulePlaying(AudioManager* manager) {
    if (manager == NULL) return false;
    return IsModPlaying(&manager->modPlayer);
}

void SetModuleVolume(AudioManager* manager, float volume) {
    if (manager == NULL) return;
    manager->modVolume = volume;
}

float GetModuleVolume(AudioManager* manager) {
    return manager ? manager->modVolume : 0.0f;
}

const char* GetModuleName(AudioManager* manager) {
    if (manager == NULL) return "Unknown";
    return GetModName(&manager->modPlayer);
}

int GetModuleChannels(AudioManager* manager) {
    if (manager == NULL) return 0;
    return GetModChannelCount(&manager->modPlayer);
}
