#include "tzx_finish_audio.h"
#include "tzx_play_audio.h"
#include "tzx-pi.h"

void tzx_finish_audio()
{
	if (!tzx_silent)
	{
		int frames;
		// Write the buffer
		if(buf_index) {
			printf("Writing final buffer (%d samples)... ", buf_index/4);
			frames = snd_pcm_writei(alsa_handle, buffer[active_buffer], buf_index/4);

            if(frames<0)
            {
                printf("\n snd_pcm_recover - %d", frames);
                snd_pcm_recover(alsa_handle, frames, 0);
            }
            else {
                printf("\n snd_pcm_writei - %d", frames);
            }

			printf(" DONE - %d\n", frames);
			// Reset the offset
			buf_index = 0;
		}
	}
}