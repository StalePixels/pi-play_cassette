#include <stdbool.h>

extern int tzx_amp;                    // Audio amplitude
extern bool tzx_silent;			// Do we actually make sound?

void tzx_play_audio(char amp, int len);