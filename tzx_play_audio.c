#include "tzx_play_audio.h"
#include "tzx-pi.h"

int tzx_amp;		 // Audio amplitude
bool tzx_silent = false; // Do we actually make sound?


void tzx_play_audio(char amp, int len)
{
	static int16_t sample;
	if (!tzx_silent)
	{
		int frames;
		/* Puts amplitude amp to soundcard buffer for len time */
		while (len)
		{

			// sample = (INT16) (amp - 0x80) << 8;
			// // sample = (amp<<8)-0x7FFF;
			buffer[active_buffer][buf_index++] = 0;
			buffer[active_buffer][buf_index++] = amp-0x7F;
			buffer[active_buffer][buf_index++] = 0;
			buffer[active_buffer][buf_index++] = amp-0x7F;
			
			len--;

			if (buf_index == BUFFER_LEN)
			{

				//sending values to sound driver
				// if (VERBOSE)
				// {
				// 	printf("\n START : WRITING AUDIO FRAMES... ");
				// }
				frames = snd_pcm_writei(alsa_handle, buffer[active_buffer], BUFFER_LEN/4);

				if(frames<0)
				{
					printf("\n snd_pcm_recover - %d", frames);
					snd_pcm_recover(alsa_handle, frames, 0);
				}
				else {
					printf("\n snd_pcm_writei - %d", frames);
				}

				//sending values to sound driver
				// if (VERBOSE)
				// {
				// 	printf(" DONE - %d\n", frames);
				// }

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