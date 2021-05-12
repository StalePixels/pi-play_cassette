#include <ctype.h>
#include <unistd.h>

#include "tzx-pi.h"
#include "usage.h"
#include "shutdown.h"
#include "tzx_file.h"
#include "tzx_load.h"

#define BUFFER_LEN 48000

static char *device = "default"; /* playback device */
snd_output_t *output = NULL;
float buffer[BUFFER_LEN];

const char TZXTapeHeaderID[] = "ZXTape!";
const char TAPTapeHeaderID[] = "TAPtap.";


bool DEBUG = false;

int main(int argc, char *argv[])
{
    int err;

    int i;

    int freq = 440;       //frequency
    int samplefq = 48000; //sampling frequency

    snd_pcm_t *handle;
    snd_pcm_sframes_t frames;

//************************************************************************************************//
// 
// SETTING UP SOUND CARD
// 
//************************************************************************************************//
    opterr = 0;

    int c;
    while ((c = getopt (argc, argv, "dht:")) != -1) {
        switch (c)
        {
        case 'd':
            DEBUG = true;
            break;
        case 'h':
            usage(EXIT_SUCCESS);
        case 't':
            tzx_filename = optarg;
            break;
        case '?':
            if (optopt == 'c')
                fprintf (stderr, "Option -%t requires an filename.\n", optopt);
            else if (isprint (optopt))
                fprintf (stderr, "Unknown option `-%c'.\n", optopt);
                else
            fprintf (stderr,
                    "Unknown option character `\\x%x'.\n",
                    optopt);
            return 1;
        default:
            abort ();
        }
    }

    // Leftover option parsing
    for (int index = optind; index < argc; index++)
    {
        if(tzx_filename==NULL) 
        {
            tzx_filename = argv[index];
        }
        else if(DEBUG)
        {
            printf ("Ignoring option: \"%s\"\n", argv[index]);
        }
    }
    
    // Errorchecking
    if(tzx_filename==NULL)
    {
        usage(EXIT_NO_FILENAME);
    }

    uint8_t load_error = tzx_load(tzx_filename);
    if(load_error) {
        shutdown(load_error, tzx_filename);
    }
    
    if(DEBUG) {
        printf("\tDEBUG   \t:\tENABLED\n");
        printf("\tDATAFILE\t:\t\"%s\"\n", tzx_filename);
        printf("\tFILE VER.\t:\t%d.%02d\n", tzx_version_major, tzx_version_minor);
    }
    
//************************************************************************************************//
// 
// SETTING UP SOUND CARD
// 
//************************************************************************************************//
    if ((err = snd_pcm_open(&handle, device, SND_PCM_STREAM_PLAYBACK, 0)) < 0)
    {
        shutdown(EXIT_NO_SOUNDCARD, snd_strerror(err));
    }

    if ((err = snd_pcm_set_params(handle,
                                  SND_PCM_FORMAT_FLOAT,
                                  SND_PCM_ACCESS_RW_INTERLEAVED,
                                  1,
                                  48000,
                                  1,
                                  500000)) < 0)
    { /* 0.5sec */
        shutdown(EXIT_PARAMS_SOUNDCARD, snd_strerror(err));
    }

    printf("\ncmdline args count=%d\n", argc);
    
    

    return 0;
    
    
    
    
    // // SINE WAVE
    // printf("Sine tone at %dHz \n", freq);

    // for (i = 0; i < BUFFER_LEN; i++)
    // {
    //     buffer[i] = (sin(((2 * M_PI * freq) / samplefq) * i));
    // }

    // frames = snd_pcm_writei(handle, buffer, BUFFER_LEN);  //sending values to sound driver

    // err = snd_pcm_drain(handle);
    // if (err < 0)
    // {
    //     printf("snd_pcm_drain failed: %s\n", snd_strerror(err));
    // }
    // snd_pcm_close(handle);
    // return 0;
}
