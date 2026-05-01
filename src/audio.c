#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "audio.h"

#define LO  -28000
#define HI   28000

int audio_open(struct audio *a, unsigned rate)
{
	int err;

	memset(a, 0, sizeof *a);
	a->rate = rate;
	a->cap  = rate;                 /* one second's worth */
	a->buf  = calloc(a->cap, sizeof *a->buf);
	if (!a->buf)
		return -1;

	err = snd_pcm_open(&a->pcm, "default", SND_PCM_STREAM_PLAYBACK, 0);
	if (err < 0) {
		fprintf(stderr, "snd_pcm_open: %s\n", snd_strerror(err));
		free(a->buf);
		return -1;
	}

	err = snd_pcm_set_params(a->pcm,
	                         SND_PCM_FORMAT_S16_LE,
	                         SND_PCM_ACCESS_RW_INTERLEAVED,
	                         1, rate, 0, 500000);
	if (err < 0) {
		fprintf(stderr, "snd_pcm_set_params: %s\n", snd_strerror(err));
		snd_pcm_close(a->pcm);
		free(a->buf);
		return -1;
	}

	return 0;
}

void audio_close(struct audio *a)
{
	if (a->pcm) {
		snd_pcm_drain(a->pcm);
		snd_pcm_close(a->pcm);
	}
	free(a->buf);
	memset(a, 0, sizeof *a);
}

static int flush(struct audio *a)
{
	int16_t *p = a->buf;
	size_t   n = a->used;

	while (n) {
		snd_pcm_sframes_t w = snd_pcm_writei(a->pcm, p, n);
		if (w == -EAGAIN)
			continue;
		if (w < 0) {
			if (snd_pcm_recover(a->pcm, w, 1) < 0) {
				fprintf(stderr, "alsa recover failed\n");
				return -1;
			}
			continue;
		}
		p += w;
		n -= w;
	}

	a->used = 0;
	return 0;
}

int audio_emit(struct audio *a, int high, long samples)
{
	int16_t v = high ? HI : LO;

	while (samples > 0) {
		size_t room = a->cap - a->used;
		size_t take = (size_t)samples < room ? (size_t)samples : room;

		for (size_t i = 0; i < take; i++)
			a->buf[a->used + i] = v;

		a->used   += take;
		samples   -= take;

		if (a->used == a->cap && flush(a) < 0)
			return -1;
	}

	return 0;
}

int audio_drain(struct audio *a)
{
	if (a->used && flush(a) < 0)
		return -1;
	if (a->pcm) {
		int err = snd_pcm_drain(a->pcm);
		if (err < 0) {
			fprintf(stderr, "snd_pcm_drain: %s\n", snd_strerror(err));
			return -1;
		}
	}
	return 0;
}
