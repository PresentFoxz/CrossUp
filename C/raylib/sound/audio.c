// Audio manager implementation for CrossUp (Raylib)

#include "audio.h"

AudioManager audioManager;

void InitAudioManager(AudioManager* manager) {
    if (manager == NULL) return;

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

    manager->initialized = true;
}

void UpdateAudioManager(AudioManager* manager, float deltaTime) {
    if (manager == NULL) return;

    // Stream music must be pumped every frame
    if (manager->isMusicPlaying && manager->currentMusicIndex >= 0) {
        MusicTrack* track = &manager->music[manager->currentMusicIndex];
        if (track->loaded) {
            UpdateMusicStream(track->stream);
        }
    }

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
            if (track->loaded) {
                // Call raylib's SetMusicVolume directly (no name conflict here since
                // AudioSetMusicVolume is our wrapper and this resolves to raylib's)
                SetMusicVolume(track->stream, newVolume * manager->masterVolume);
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
    if (manager == NULL) return;

    for (size_t i = 0; i < manager->musicCount; i++) {
        if (manager->music[i].loaded) {
            StopMusicStream(manager->music[i].stream);
            UnloadMusicStream(manager->music[i].stream);
            manager->music[i].loaded = false;
        }
    }

    for (size_t i = 0; i < manager->soundCount; i++) {
        if (manager->sounds[i].loaded) {
            StopSound(manager->sounds[i].sound);
            UnloadSound(manager->sounds[i].sound);
            manager->sounds[i].loaded = false;
        }
    }

    manager->musicCount = 0;
    manager->soundCount = 0;
    manager->currentMusicIndex = -1;
    manager->initialized = false;
}

bool PlayMusic(AudioManager* manager, const char* path, float volume, bool loop, float delay) {
    if (manager == NULL || path == NULL) return false;
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

    int trackIndex = -1;
    if (manager->musicCount < MAX_MUSIC_TRACKS) {
        trackIndex = (int)manager->musicCount;
        manager->musicCount++;
    } else {
        trackIndex = 0;
        if (manager->music[0].loaded) {
            UnloadMusicStream(manager->music[0].stream);
            manager->music[0].loaded = false;
        }
    }

    Music stream = LoadMusicStream(path);
    if (!IsMusicValid(stream)) {
        TraceLog(LOG_ERROR, "Audio: Failed to load music: %s", path);
        return false;
    }

    stream.looping = loop;

    manager->music[trackIndex].stream = stream;
    manager->music[trackIndex].isLooping = loop;
    manager->music[trackIndex].isPlaying = true;
    manager->music[trackIndex].loaded = true;
    strncpy(manager->music[trackIndex].path, path, sizeof(manager->music[trackIndex].path) - 1);

    float finalVol = volume * manager->musicVolume * manager->masterVolume;
    SetMusicVolume(stream, finalVol);
    PlayMusicStream(stream);

    manager->currentMusicIndex = trackIndex;
    manager->isMusicPlaying = true;

    return true;
}

void StopMusic(AudioManager* manager) {
    if (manager == NULL) return;
    if (manager->currentMusicIndex < 0) return;

    MusicTrack* track = &manager->music[manager->currentMusicIndex];
    if (track->loaded) {
        StopMusicStream(track->stream);
        track->isPlaying = false;
    }

    manager->isMusicPlaying = false;
    manager->isFadingOut = false;
}

void PauseMusic(AudioManager* manager) {
    if (manager == NULL) return;
    if (manager->currentMusicIndex < 0 || !manager->isMusicPlaying) return;

    MusicTrack* track = &manager->music[manager->currentMusicIndex];
    if (track->loaded) {
        PauseMusicStream(track->stream);
    }
}

void ResumeMusic(AudioManager* manager) {
    if (manager == NULL) return;
    if (manager->currentMusicIndex < 0) return;

    MusicTrack* track = &manager->music[manager->currentMusicIndex];
    if (track->loaded) {
        ResumeMusicStream(track->stream);
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
    if (manager == NULL || path == NULL) return false;
    if (!manager->isSfxEnabled) return false;

    Sound sound = LoadSound(path);
    if (!IsSoundValid(sound)) {
        TraceLog(LOG_ERROR, "Audio: Failed to load sound: %s", path);
        return false;
    }

    float finalVol = volume * manager->sfxVolume * manager->masterVolume;
    SetSoundVolume(sound, finalVol);
    PlaySound(sound);

    if (manager->soundCount < MAX_SOUNDS) {
        manager->sounds[manager->soundCount].sound = sound;
        manager->sounds[manager->soundCount].isPlaying = true;
        manager->sounds[manager->soundCount].loaded = true;
        manager->soundCount++;
    }

    return true;
}

bool LoadSFX(AudioManager* manager, const char* path) {
    if (manager == NULL || path == NULL) return false;
    if (manager->soundCount >= MAX_SOUNDS) return false;

    Sound sound = LoadSound(path);
    if (!IsSoundValid(sound)) {
        TraceLog(LOG_ERROR, "Audio: Failed to preload sound: %s", path);
        return false;
    }

    manager->sounds[manager->soundCount].sound = sound;
    manager->sounds[manager->soundCount].isPlaying = false;
    manager->sounds[manager->soundCount].loaded = true;
    manager->soundCount++;

    return true;
}

void StopAllSFX(AudioManager* manager) {
    if (manager == NULL) return;

    for (size_t i = 0; i < manager->soundCount; i++) {
        if (manager->sounds[i].loaded) {
            StopSound(manager->sounds[i].sound);
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
        if (track->loaded) {
            SetMusicVolume(track->stream, volume * manager->masterVolume);
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

// Raylib natively handles MOD/XM/S3M/IT via LoadMusicStream
bool PlayModuleMusic(AudioManager* manager, const char* path) {
    if (manager == NULL || path == NULL) return false;
    return PlayMusic(manager, path, manager->musicVolume, true, 0.0f);
}

void StopModuleMusic(AudioManager* manager) {
    StopMusic(manager);
}

void PauseModuleMusic(AudioManager* manager) {
    PauseMusic(manager);
}

void ResumeModuleMusic(AudioManager* manager) {
    ResumeMusic(manager);
}

void RestartModuleMusic(AudioManager* manager) {
    if (manager == NULL) return;
    if (manager->currentMusicIndex < 0) return;

    MusicTrack* track = &manager->music[manager->currentMusicIndex];
    if (track->loaded) {
        SeekMusicStream(track->stream, 0.0f);
    }
}

bool IsModulePlaying(AudioManager* manager) {
    return IsMusicPlaying(manager);
}

void SetModuleVolume(AudioManager* manager, float volume) {
    AudioSetMusicVolume(manager, volume);
}

float GetModuleVolume(AudioManager* manager) {
    return GetMusicVolume(manager);
}
