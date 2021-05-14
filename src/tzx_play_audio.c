#include "tzx_play_audio.h"
#include "tzx-pi.h"

int tzx_amp;		 // Audio amplitude
bool tzx_silent = false; // Do we actually make sound?

void tzx_play_audio(char amp, int len)
{
	if (!tzx_silent)
	{
		int frames;
		/* Puts amplitude amp to soundcard buffer for len time */
		while (len)
		{
			buffer[active_buffer][buf_index++] = amp;

			len--;

			if (buf_index == BUFFER_LEN)
			{

				//sending values to sound driver
				if (VERBOSE)
				{
					printf("\n START : WRITING AUDIO FRAMES... ");
				}
				frames = snd_pcm_writei(alsa_handle, buffer[active_buffer], BUFFER_LEN);

				//sending values to sound driver
				if (VERBOSE)
				{
					printf(" DONE - %d\n", frames);
				}

				// Flip the buffers
				active_buffer = !active_buffer;

				// Rewind the insert pointer
				buf_index = 0;
			}
		}
	}
	else if (VERBOSE)
	{

		printf("\n SKIPPING: %d FRAMES... ", len);
	}
}