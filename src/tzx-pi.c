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


// Conversion functions to get 2,3 and 4 byte words ...
int get_uint16(char *ptr)
{
    return(ptr[0] + (ptr[1]*256));
}

int get_uint24(char *ptr)
{
    return(get_uint16(ptr) + (ptr[2]*256*256));

}

int get_uint32(char *ptr)
{
    return(get_uint24(ptr) + (ptr[3]*256*256*256));
}

int main(int argc, char *argv[])
{
    int err;

    int i;

    int sinefreq = 440;
    
    // int playfreq = 44100;
    
    int samplefreq = 44100; //sampling frequency

    snd_pcm_t *handle;
    snd_pcm_sframes_t frames;

//************************************************************************************************//
// 
// PARSE OPTIONS
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
    

//************************************************************************************************//
// 
// LOAD TZX DATA
// 
//************************************************************************************************//
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
// LOAD TZX DATA
// 
//************************************************************************************************//
    int unrecognised_block = 0;
    int numblocks=0;
    int tzx_data_offset=0;
    int tzx_block_offsets[2048];            // Array of offsets into the data that a block starts
    int tzx_starting_block = 1;
    int tzx_ending_block = 0;
    int tzx_current_block;
    
    while(tzx_data_offset<tzx_filesize-10)
    {
        tzx_block_offsets[numblocks]=tzx_data_offset;
        tzx_data_offset++;
        switch(tzx_data[tzx_data_offset-1])
        {
            case 0x10:
                tzx_data_offset += get_uint16(&tzx_data[tzx_data_offset+0x02])+0x04;
                break;
            case 0x11:
                tzx_data_offset += get_uint24(&tzx_data[tzx_data_offset+0x0F])+0x12;
                break;
            case 0x12:
                tzx_data_offset += 0x04;
                break;
            case 0x13:
                tzx_data_offset += (tzx_data[tzx_data_offset+0x00]*0x02)+0x01;
                break;
            case 0x14:
                tzx_data_offset += get_uint24(&tzx_data[tzx_data_offset+0x07])+0x0A;
                break;
            case 0x15:  
                tzx_data_offset += get_uint24(&tzx_data[tzx_data_offset+0x05])+0x08;
                break;

            case 0x16:
                tzx_data_offset += get_uint32(&tzx_data[tzx_data_offset+0x00])+0x04;
                break;
            case 0x17:
                tzx_data_offset += get_uint32(&tzx_data[tzx_data_offset+0x00])+0x04;
                break;

            case 0x20:
                tzx_data_offset += 0x02;
                break;
            case 0x21:
                tzx_data_offset += tzx_data[tzx_data_offset+0x00]+0x01;
                break;
            case 0x22:
                break;
            case 0x23:
                tzx_data_offset += 0x02;
                break;
            case 0x24:
                tzx_data_offset += 0x02;
                break;
            case 0x25:
                break;
            case 0x26:
                tzx_data_offset += get_uint16(&tzx_data[tzx_data_offset+0x00])*0x02+0x02;
                break;
            case 0x27:
                break;
            case 0x28:
                tzx_data_offset += get_uint16(&tzx_data[tzx_data_offset+0x00])+0x02;
                break;

            case 0x2A:
                tzx_data_offset += 0x04;
                break;

            case 0x30:
                tzx_data_offset += tzx_data[tzx_data_offset+0x00]+0x01;
                break;
            case 0x31:
                tzx_data_offset += tzx_data[tzx_data_offset+0x01]+0x02;
                break;
            case 0x32:
                tzx_data_offset += get_uint16(&tzx_data[tzx_data_offset+0x00])+0x02;
                break;
            case 0x33:
                tzx_data_offset += (tzx_data[tzx_data_offset+0x00]*0x03)+0x01;
                break;
            case 0x34:
                tzx_data_offset += 0x08; break;
            case 0x35:
                tzx_data_offset += get_uint32(&tzx_data[tzx_data_offset+0x10])+0x14;
                break;

            case 0x40:
                tzx_data_offset += get_uint24(&tzx_data[tzx_data_offset+0x01])+0x04;
                break;

            case 0x5A:
                tzx_data_offset += 0x09; break;

            default:   tzx_data_offset += get_uint32(&tzx_data[tzx_data_offset+0x00])+0x04;
                    unrecognised_block++;
        }
        numblocks++;
    }
    printf("Number of Blocks: %d\n",numblocks);
    
    if (unrecognised_block)
    {
        printf("WARNING: Found %d Unknown Block!\n",unrecognised_block);
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
                                  samplefreq,
                                  1,
                                  500000)) < 0)
    { /* 0.5sec */
        shutdown(EXIT_PARAMS_SOUNDCARD, snd_strerror(err));
    }


//************************************************************************************************//
// 
// SETTING UP SOUND CARD
// 
//************************************************************************************************//
    tzx_current_block=0;
    if (tzx_starting_block>1)
    {
        if (tzx_starting_block>numblocks)
        {
            char blocks[5];
            sprintf(blocks,"%d",tzx_starting_block);
            shutdown(ERR_INVALID_STARTING_BLOCK, blocks);
        }
        
        tzx_current_block=tzx_starting_block - 1;
    }
    
    if (tzx_ending_block>0)
    {
        if (tzx_ending_block>numblocks || tzx_ending_block<tzx_starting_block)
        {
            char blocks[5];
            sprintf(blocks,"%d",tzx_ending_block);
            shutdown(ERR_INVALID_ENDING_BLOCK, blocks);
        }
        numblocks=tzx_ending_block;
    }

    printf("\nStarting playback on using %d Hz frequency.\n\n",samplefreq);

    // // SINE WAVE
    // printf("Sine tone at %dHz \n", sinefreq);

    // for (i = 0; i < BUFFER_LEN; i++)
    // {
    //     buffer[i] = (sin(((2 * M_PI * sinefreq) / samplefreq) * i));
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
