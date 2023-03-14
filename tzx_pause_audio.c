#include "tzx_pause_audio.h"
#include "tzx-pi.h"
#include "tzx_play_audio.h"

void tzx_pause_audio(char amp, int delay)
{

/* Waits for tzx_pause milliseconds*/
  int p;

  p = (int) ((((float) delay) * samplefreq) / 1000.0);

  tzx_play_audio(amp, p);

}