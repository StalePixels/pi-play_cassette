
#include "tzx_toggle_amplitude.h"
#include "tzx_play_audio.h"

inline void tzx_toggle_amplitude()
{
    if (tzx_amp == LOAMP)
    {
        tzx_amp = HIAMP;
    }
    else
    {
        tzx_amp = LOAMP;
    }
}
