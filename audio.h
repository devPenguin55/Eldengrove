#ifndef AUDIO_H
#define AUDIO_H

#include "miniaudio.h"

extern ma_engine audioEngine;

void initAudio();
void audioPlay(const char* path);
void audioShutdown();

#endif