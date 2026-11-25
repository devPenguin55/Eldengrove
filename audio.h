#ifndef AUDIO_H
#define AUDIO_H

#include "miniaudio.h"

extern ma_engine audioEngine;

void audioInit();
void audioPlay(const char* path);
void audioShutdown();

#endif