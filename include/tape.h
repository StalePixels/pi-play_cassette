#ifndef PLAY_CASSETTE_TAPE_H
#define PLAY_CASSETTE_TAPE_H

#include <libspectrum.h>

#include "audio.h"

int  tape_open(const char *path, libspectrum_tape **out);
void tape_close(libspectrum_tape *tape);

int  tape_play(libspectrum_tape *tape, struct audio *out);

#endif
