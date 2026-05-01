#ifndef PLAY_CASSETTE_AUDIO_H
#define PLAY_CASSETTE_AUDIO_H

#include <stdint.h>
#include <alsa/asoundlib.h>

struct audio {
	snd_pcm_t *pcm;
	unsigned rate;
	int16_t *buf;
	size_t cap;
	size_t used;
};

int  audio_open(struct audio *a, unsigned rate);
void audio_close(struct audio *a);

int  audio_emit(struct audio *a, int high, long samples);
int  audio_drain(struct audio *a);

#endif
