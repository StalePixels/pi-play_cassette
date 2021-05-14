#include "tzx_finish_audio.h"
#include "tzx_play_audio.h"
#include "tzx-pi.h"

void tzx_finish_audio()
{
	if (!tzx_silent)
	{	
		int err = snd_pcm_drain(alsa_handle);
		if (err < 0)
		{
		printf("snd_pcm_drain failed: %s\n", snd_strerror(err));
		}
	}
}