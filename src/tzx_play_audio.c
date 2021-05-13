#include "tzx_play_audio.h"
#include "tzx-pi.h"

int tzx_amp;                    // Audio amplitude

void tzx_play_audio(char amp, int len)
{
  int frames;
/* Puts amplitude amp to soundcard buffer for len time */
  while (len) {
    buffer[active_buffer][buf_index++] = amp;

    len--;

    if (buf_index == BUFFER_LEN) {	/* Is the buffer full ? */
      // flushSBBuffer();
      
      printf("WRITING FRAMES\n");
      frames = snd_pcm_writei(alsa_handle, buffer[active_buffer], BUFFER_LEN);  //sending values to sound driver
      printf("FRAMES WRITTEN: %d\n", frames);
      active_buffer = !active_buffer;
      buf_index = 0;
    }
  }

}