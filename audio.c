#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include "audio.h"

ma_engine audioEngine;
ma_sound currentMusic;

void audioInit() {
    ma_result result = ma_engine_init(NULL, &audioEngine);
    if (result != MA_SUCCESS) {
        printf("Failed to initialize audio engine\n");
        return;
    }
}

void audioPlay(const char* path) {
    ma_engine_play_sound(&audioEngine, path, NULL);
}


void audioPlayMusic(const char* path) {
    ma_sound_uninit(&currentMusic);

    ma_sound_init_from_file(
        &audioEngine,
        path,
        MA_SOUND_FLAG_STREAM,
        NULL,
        NULL,
        &currentMusic
    );

    ma_sound_set_looping(&currentMusic, MA_TRUE);
    ma_sound_start(&currentMusic);
}

void audioShutdown() {
    ma_engine_uninit(&audioEngine);
}