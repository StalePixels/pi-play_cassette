#include <ctype.h>
#include <unistd.h>

#include "tzx-pi.h"
#include "usage.h"
#include "shutdown.h"
#include "tzx_file.h"
#include "tzx_load.h"
#include "tzx_block_info.h"
#include "tzx_compute_checksum.h"
#include "tzx_ticks_to_samples.h"
#include "tzx_toggle_amplitude.h"
#include "tzx_play_audio.h"
#include "tzx_pause_audio.h"

static char *device = "default"; /* playback device */
snd_output_t *output = NULL;
// float buffer[BUFFER_LEN];
uint8_t buffer[2][BUFFER_LEN];
uint8_t active_buffer = 0;
int buf_index = 0;
snd_pcm_t *alsa_handle;

const char TZXTapeHeaderID[] = "ZXTape!";
const char TAPTapeHeaderID[] = "TAPtap.";

bool DEBUG = false;
int samplefreq = 44800;         //soundcard audio sample frequency

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
    
    // int playfreq = 44800;
    
    double cpufreq = 3500000.0;
    // double cpufreq = 7000000.0;
    
    tzx_dutycycle = samplefreq / cpufreq;

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

    uint8_t load_error = tzx_load();
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
    int tzx_blockcount=0;
    int tzx_data_current_position=0;        // How far into the datastream we are, an index
    int tzx_block_offsets[2048];            // Array of offsets into the data that a block starts
    int tzx_starting_block = 1;
    int tzx_current_block;
    int tzx_ending_block = 0;
    
    while(tzx_data_current_position<tzx_filesize-10)
    {
        tzx_block_offsets[tzx_blockcount]=tzx_data_current_position;
        
        printf("block at offset %x\n", tzx_data_current_position);
        printf("block at offset %x: %x\n", tzx_data_current_position, tzx_data[tzx_data_current_position]);
        
        tzx_data_current_position++;
        switch(tzx_data[tzx_data_current_position-1])
        {
            case 0x10:
                tzx_data_current_position += get_uint16(&tzx_data[tzx_data_current_position+0x02])+0x04;
                break;
            case 0x11:
                tzx_data_current_position += get_uint24(&tzx_data[tzx_data_current_position+0x0F])+0x12;
                break;
            case 0x12:
                tzx_data_current_position += 0x04;
                break;
            case 0x13:
                tzx_data_current_position += (tzx_data[tzx_data_current_position+0x00]*0x02)+0x01;
                break;
            case 0x14:
                tzx_data_current_position += get_uint24(&tzx_data[tzx_data_current_position+0x07])+0x0A;
                break;
            case 0x15:  
                tzx_data_current_position += get_uint24(&tzx_data[tzx_data_current_position+0x05])+0x08;
                break;
            case 0x16:
                tzx_data_current_position += get_uint32(&tzx_data[tzx_data_current_position+0x00])+0x04;
                break;
            case 0x17:
                tzx_data_current_position += get_uint32(&tzx_data[tzx_data_current_position+0x00])+0x04;
                break;

            case 0x20:
                tzx_data_current_position += 0x02;
                break;
            case 0x21:
                tzx_data_current_position += tzx_data[tzx_data_current_position+0x00]+0x01;
                break;
            case 0x22:
                break;
            case 0x23:
                tzx_data_current_position += 0x02;
                break;
            case 0x24:
                tzx_data_current_position += 0x02;
                break;
            case 0x25:
                break;
            case 0x26:
                tzx_data_current_position += get_uint16(&tzx_data[tzx_data_current_position+0x00])*0x02+0x02;
                break;
            case 0x27:
                break;
            case 0x28:
                tzx_data_current_position += get_uint16(&tzx_data[tzx_data_current_position+0x00])+0x02;
                break;

            case 0x2A:
                tzx_data_current_position += 0x04;
                break;

            case 0x30:
                tzx_data_current_position += tzx_data[tzx_data_current_position+0x00]+0x01;
                break;
            case 0x31:
                tzx_data_current_position += tzx_data[tzx_data_current_position+0x01]+0x02;
                break;
            case 0x32:
                tzx_data_current_position += get_uint16(&tzx_data[tzx_data_current_position+0x00])+0x02;
                break;
            case 0x33:
                tzx_data_current_position += (tzx_data[tzx_data_current_position+0x00]*0x03)+0x01;
                break;
            case 0x34:
                tzx_data_current_position += 0x08; break;
            case 0x35:
                tzx_data_current_position += get_uint32(&tzx_data[tzx_data_current_position+0x10])+0x14;
                break;

            case 0x40:
                tzx_data_current_position += get_uint24(&tzx_data[tzx_data_current_position+0x01])+0x04;
                break;

            case 0x5A:
                tzx_data_current_position += 0x09; break;

            default:   tzx_data_current_position += get_uint32(&tzx_data[tzx_data_current_position+0x00])+0x04;
                    unrecognised_block++;
        }
        tzx_blockcount++;
    }
    
    printf("\tTOTAL BLOCKS\t:\t%d\n", tzx_blockcount);
    if (unrecognised_block)
    {
        printf("\tUNKNOWN BLOCKS.\t:\t%d\n", unrecognised_block);
    }

//************************************************************************************************//
// 
// SET THE START AND END BLOCKS
// 
//************************************************************************************************//
    uint8_t tzx_current_block_type = 0;
    uint8_t *tzx_current_data;
    
    tzx_current_block=0;
    if (tzx_starting_block>1)
    {
        if (tzx_starting_block > tzx_blockcount)
        {
            char blocks[5];
            sprintf(blocks,"%d",tzx_starting_block);
            shutdown(ERR_INVALID_STARTING_BLOCK, blocks);
        }

        tzx_current_block=tzx_starting_block - 1;
    }
    
    if (tzx_ending_block > 0)
    {
        if (tzx_ending_block>tzx_blockcount || tzx_ending_block<tzx_starting_block)
        {
            char blocks[5];
            sprintf(blocks,"%d",tzx_ending_block);
            shutdown(ERR_INVALID_ENDING_BLOCK, blocks);
        }
        tzx_blockcount=tzx_ending_block;
    }


    printf("\nStarting playback on using %d Hz frequency.\n\n",samplefreq);

//************************************************************************************************//
// 
// SETTING UP SOUND CARD
// 
//************************************************************************************************//
    if ((err = snd_pcm_open(&alsa_handle, device, SND_PCM_STREAM_PLAYBACK, 0)) < 0)
    {
        shutdown(EXIT_NO_SOUNDCARD, snd_strerror(err));
    }

    if ((err = snd_pcm_set_params(alsa_handle,
                                  SND_PCM_FORMAT_U8,
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
// SEND DATA TO SOUND CARD
// 
//************************************************************************************************//
    int tzx_pause;                          // Duration of silence in "Half Pulses"
    int tzx_current_datablock_size;         //  This datablock, size
    int tzx_current_datablock_position;     //  This datablock, current position
    int tzx_pilot;                          // Size of pilot signal in "Half Pulses"
    int tzx_audio_pilot;                    // Actual Pilot Pulse
    int tzx_audio_sync1;                    // Sync first half-period (hp)
    int tzx_audio_sync2;                    // Sync second
    int tzx_audio_bit0;                     // Bit-0
    int tzx_audio_bit1;                     // Bit-1
    int tzx_audio_bit;                      // Current audio bit
    int tzx_audio_pulse;                    // Pulse in Sequence of pulses and direct recording block
    int tzx_speed;                          // Audio datarate (not related to CPU speed)
    uint8_t tzx_this_byte;                  // Current Byte to be replayed of the data */
    uint8_t tzx_bits_in_this_byte;          // How many bits of data we are processing this time.
    uint8_t tzx_bits_in_last_byte;          // Number of bits in the last byte of data
    
    // Start replay of blocks ...
    while (tzx_current_block<tzx_blockcount)
    {
        printf("Block %3d-%5X:",tzx_current_block+1, tzx_block_offsets[tzx_current_block]+10);

        tzx_current_block_type = tzx_data[tzx_block_offsets[tzx_current_block]];
        tzx_current_data = &tzx_data[tzx_block_offsets[tzx_current_block]+1];
        
        switch (tzx_current_block_type)
        {
            // Standard Loading Data block
            case 0x10:
                tzx_pause = get_uint16(&tzx_current_data[0]);
                tzx_current_datablock_size = get_uint16(&tzx_current_data[2]);
                tzx_current_data += 4;
                if (tzx_current_data[0] == 0x00){
                    tzx_pilot=8064;
                }
                else
                {
                    tzx_pilot=3220;
                }
                tzx_audio_pilot=tzx_ticks_to_samples(2168);
                tzx_audio_sync1=tzx_ticks_to_samples(667);
                tzx_audio_sync2=tzx_ticks_to_samples(735);
                tzx_audio_bit0=tzx_ticks_to_samples(885);
                tzx_audio_bit1=tzx_ticks_to_samples(1710);
                tzx_bits_in_last_byte=8;
                if (DEBUG)
                {
                    // Identify(tzx_current_datablock_size,data,1);
                    printf("Block %3d (%5X):  10 - Standard Loading Data\n",tzx_current_block+1,
                        tzx_block_offsets[tzx_current_block]+10);
                    printf("                Length: %5d bytes\n",tzx_current_datablock_size);
                    printf("                  Flag: %5d ($%02X)\n",tzx_current_data[0],tzx_current_data[0]);
                    printf("              CheckSum: %5d ($%02X) - ",
                        tzx_current_data[tzx_current_datablock_size-1],
                        tzx_current_data[tzx_current_datablock_size-1]);
                    tzx_compute_checksum(tzx_current_data,tzx_current_datablock_size);
                    printf("\n");
                    printf("     Pause after block: %5d milliseconds\n",tzx_pause);
                }
                printf("\n");
                break;
        /*
            // // Custom Loading Data block
            // case 0x11:
            //     sb_pilot=tzx_ticks_to_samples(get_uint16(&data[0]));
            //     sb_sync1=tzx_ticks_to_samples(get_uint16(&data[2]));
            //     sb_sync2=tzx_ticks_to_samples(get_uint16(&data[4]));
            //     sb_bit0=tzx_ticks_to_samples(get_uint16(&data[6]));
            //     sb_bit1=tzx_ticks_to_samples(get_uint16(&data[8]));
            //     tzx_speed=(int) ((1710.0/(double) get_uint16(&data[8]))*100.0);
            //     pilot=get_uint16(&data[10]);
            //     lastbyte=(int) data[12];
            //     tzx_pause=get_uint16(&data[13]);
            //     tzx_current_datablock_size=get_uint24(&data[15]);
            //     data+=18;
            //     if (info==1)
            //     {
            //         Identify(tzx_current_datablock_size,data,1);
            //         sprintf(pstr,"Block %3d (%5X):  11 - Custom Loading Data - %s\n",curr+1,block[curr]+10,tstr); writeout(pstr);
            //         sprintf(tstr,"                Length: %5d bytes\n",tzx_current_datablock_size); writeout(tstr);
            //         sprintf(tstr,"                  Flag: %5d ($%02X)\n",data[0],data[0]); writeout(tstr);
            //         if (!cpc)
            //         {
            //             sprintf(tstr,"              CheckSum: %5d ($%02X) - %s\n",data[tzx_current_datablock_size-1],data[tzx_current_datablock_size-1],GetCheckSum(data,tzx_current_datablock_size)); writeout(tstr);
            //         }
            //         sprintf(tstr,"           Pilot pulse: %5d T-States\n",get_uint16(data-18)); writeout(tstr);
            //         sprintf(tstr,"          Pilot length: %5d pulses\n",pilot); writeout(tstr);
            //         sprintf(tstr,"      Sync first pulse: %5d T-States\n",get_uint16(data-16)); writeout(tstr);
            //         sprintf(tstr,"     Sync second pulse: %5d T-States\n",get_uint16(data-14)); writeout(tstr);
            //         sprintf(tstr,"           Bit-0 pulse: %5d T-States\n",get_uint16(data-12)); writeout(tstr);
            //         sprintf(tstr,"           Bit-1 pulse: %5d T-States\n",get_uint16(data-10)); writeout(tstr);
            //         sprintf(tstr,"        Last byte used: %5d bits\n",lastbyte); writeout(tstr);
            //         sprintf(tstr,"     Pause after block: %5d milliseconds\n\n",tzx_pause); line++; writeout(tstr);
            //     }
            //     break;
            // // Pure Tone
            // case 0x12:
            //     sb_pilot=tzx_ticks_to_samples(get_uint16(&data[0]));
            //     pilot=get_uint16(&data[2]);
            //     if (info!=1)
            //     {
            //         if (draw) {
            //             printf("    Pure Tone             Length: %5d\n",pilot);
            //         }
            //         if (info!=2)
            //         {
            //             while (pilot) {
            //                 tzx_play_audio(amp,sb_pilot); tzx_toggle_amplitude(); pilot--;
            //             }
            //         }
            //     }
            //     else
            //     {
            //         sprintf(tstr,"Block %3d (%5X):  12 - Pure Tone\n",curr+1,block[curr]+10); writeout(tstr);
            //         sprintf(tstr,"          Pulse length: %5d T-States\n",get_uint16(data)); writeout(tstr);
            //         sprintf(tstr,"           Tone length: %5d pulses\n\n",pilot); line++; writeout(tstr);
            //     }
            //     break;
            // // Sequence of Pulses
            // case 0x13:  
            //     pilot=(int) data[0];
            //     data++;
            //     if (info!=1)
            //     {
            //         if (draw) 
            //         {
            //             printf("    Sequence of Pulses    Length: %5d\n",pilot);
            //         }
            //         if (info!=2)
            //         {
            //             while(pilot)
            //             {
            //                 sb_pulse=tzx_ticks_to_samples(get_uint16(&data[0]));
            //                 tzx_play_audio(amp,sb_pulse); tzx_toggle_amplitude(); pilot--; data+=2;
            //             }
            //         }
            //     }
            //     else
            //     {
            //         sprintf(tstr,"Block %3d (%5X):  13 - Sequence of Pulses\n",curr+1,block[curr]+10); writeout(tstr);
            //         sprintf(tstr,"      Number of Pulses: %5d\n\n",pilot); line++; writeout(tstr);
            //     }
            //     break;
            // // Pure Data
            // case 0x14:
            //     sb_pilot=pilot=sb_sync1=sb_sync2=0;
            //     sb_bit0=tzx_ticks_to_samples(get_uint16(&data[0]));
            //     sb_bit1=tzx_ticks_to_samples(get_uint16(&data[2]));
            //     tzx_speed=(int) ((1710.0/(double) get_uint16(&data[2]))*100.0);
            //     lastbyte=(int) data[4];
            //     tzx_pause=get_uint16(&data[5]);
            //     tzx_current_datablock_size=get_uint24(&data[7]);
            //     data+=10;
            //     if (info==1)
            //     {
            //         sprintf(tstr,"Block %3d (%5X):  14 - Pure Data\n",curr+1,block[curr]+10); writeout(tstr);
            //         sprintf(tstr,"                Length: %5d bytes\n",tzx_current_datablock_size); writeout(tstr);
            //         sprintf(tstr,"                  Flag: %5d ($%02X)\n",data[0],data[0]); writeout(tstr);
            //         sprintf(tstr,"              CheckSum: %5d ($%02X) - %s\n",data[tzx_current_datablock_size-1],data[tzx_current_datablock_size-1],GetCheckSum(data,tzx_current_datablock_size)); writeout(tstr);
            //         sprintf(tstr,"           Bit-0 pulse: %5d T-States\n",get_uint16(data-10)); writeout(tstr);
            //         sprintf(tstr,"           Bit-1 pulse: %5d T-States\n",get_uint16(data-8)); writeout(tstr);
            //         sprintf(tstr,"        Last byte used: %5d bits\n",lastbyte); writeout(tstr);
            //         sprintf(tstr,"     Pause after block: %5d milliseconds\n\n",tzx_pause); line++; writeout(tstr);
            //     }
            //     break;
            // // Direct Recording
            // case 0x15:
            //     sb_pulse=tzx_ticks_to_samples(get_uint16(&data[0]));
            //     // For now the BEST way is to use the sample frequency for replay that is
            //     // exactly the SAME as the Original Freq. used when sampling this block !
            //     // i.e. NO downsampling is handled YET ... use TAPER when you need it ! ;-)
            //     if (!sb_pulse)
            //     {
            //         sb_pulse=1;          // In case sample frequency > 44800
            //     }
            //     tzx_pause=get_uint16(&data[2]);               // (Should work for frequencies upto 48000)
            //     lastbyte=(int) data[4];
            //     tzx_current_datablock_size=get_uint24(&data[5]);
            //     if (info!=1)
            //     {
            //         if (draw) printf("    Direct Recording      Length:%6d  Original Freq.: %5d Hz\n",
            //                         tzx_current_datablock_size, (int) (0.5+(3500000.0/ (double) get_uint16(&data[0]))));
            //         if (info!=2)
            //         {
            //             data=&data[8];
            //             tzx_current_datablock_position=0;
            //             // Replay Direct Recording block ... 
            //             while (tzx_current_datablock_size)
            //             {
            //                 if (tzx_current_datablock_size!=1) bitcount=8; else bitcount=lastbyte;
            //                 tzx_this_byte=data[tzx_current_datablock_position];
            //                 while (bitcount)
            //                 {
            //                     if (tzx_this_byte&0x80) amp=HIAMP; else amp=LOAMP;
            //                     tzx_play_audio(amp,sb_pulse);
            //                     tzx_this_byte<<=1;
            //                     bitcount--;
            //                 }
            //                 tzx_current_datablock_size--; tzx_current_datablock_position++;
            //             }
            //             amp=LOAMP;
            //             if (tzx_pause) {
            //                 tzx_pause_audio(amp,tzx_pause);
            //             }
            //         }
            //     }
            //     else
            //     {
            //         sprintf(tstr,"Block %3d (%5X):  15 - Direct Recording\n",curr+1,block[curr]+10); writeout(tstr);
            //         sprintf(tstr,"                Length:%6d bytes\n",tzx_current_datablock_size); writeout(tstr);
            //         sprintf(tstr,"    Original Frequency: %5d T-States/Sample (%5d Hz)\n",
            //         get_uint16(data),(int) (0.5+(3500000.0/ (double) get_uint16(data)))); writeout(tstr);
            //         sprintf(tstr,"        Last byte used: %5d samples\n",lastbyte); writeout(tstr);
            //         sprintf(tstr,"     Pause after block: %5d milliseconds\n\n",tzx_pause); line++; writeout(tstr);
            //     }
            //     break;
            // // C64 ROM Type Data Block
            // case 0x16:
            //     data+=4;
            //     sb_pilot=get_uint16(&data[0]);
            //     pilot=get_uint16(&data[2]);
            //     sb_sync1=get_uint16(&data[4]);
            //     sb_sync2=get_uint16(&data[6]);
            //     sb_bit0_f=get_uint16(&data[8]);
            //     sb_bit0_s=get_uint16(&data[10]);
            //     sb_bit1_f=get_uint16(&data[12]);
            //     sb_bit1_s=get_uint16(&data[14]);
            //     xortype=(int)(data[16]);
            //     sb_finishbyte_f=get_uint16(&data[17]);
            //     sb_finishbyte_s=get_uint16(&data[19]);
            //     sb_finishdata_f=get_uint16(&data[21]);
            //     sb_finishdata_s=get_uint16(&data[23]);
            //     sb_trailing=get_uint16(&data[25]);
            //     trailing=get_uint16(&data[27]);
            //     lastbyte=(int)(data[29]);
            //     endian=data[30];
            //     tzx_pause=get_uint16(&data[31]);
            //     tzx_current_datablock_size=get_uint24(&data[33]);
            //     data+=36;
            //     IdentifyC64ROM(tzx_current_datablock_size, data, 1);
            //     if (info==1)
            //         {
            //         sprintf(pstr,"Block %3d (%5X):  16 - C64 ROM Type Data - %s\n",curr+1,block[curr]+10,tstr); writeout(pstr);
            //         sprintf(tstr,"                Length: %5d bytes\n",tzx_current_datablock_size); writeout(tstr);
            //         sprintf(tstr,"                  Flag: %5d ($%02X)\n",data[0],data[0]); writeout(tstr);
            //         if (pilot)
            //         { sprintf(tstr,"           Pilot pulse: %5d T-States, length: %5d pulses\n",sb_pilot,pilot); writeout(tstr); }
            //         else
            //         { sprintf(tstr,"           Pilot pulse:  None\n"); writeout(tstr); }
            //         sprintf(tstr,"           Sync pulses: %5d & %5d T-States\n",sb_sync1,sb_sync2); writeout(tstr);
            //         sprintf(tstr,"          Bit-0 pulses: %5d & %5d T-States\n",sb_bit0_f,sb_bit0_s); writeout(tstr);
            //         sprintf(tstr,"          Bit-1 pulses: %5d & %5d T-States\n",sb_bit1_f,sb_bit1_s); writeout(tstr);
            //         sprintf(tstr,"        Last byte used: %5d bits\n",lastbyte); writeout(tstr);
            //         if (xortype != 0xFF)
            //         { sprintf(tstr,"         Byte XOR Type: %d XOR all bits\n",xortype); writeout(tstr); }
            //         else
            //         { sprintf(tstr,"         Byte XOR Type:  None\n"); writeout(tstr); }
            //         sprintf(tstr,"    Finish Byte pulses: %5d & %5d T-States\n",sb_finishbyte_f,sb_finishbyte_s); writeout(tstr);
            //         sprintf(tstr,"    Finish Data pulses: %5d & %5d T-States\n",sb_finishdata_f,sb_finishdata_s); writeout(tstr);
            //         if (trailing)
            //         { sprintf(tstr,"   Trailing Tone pulse: %5d T-States, length: %5d pulses\n",sb_trailing,trailing); writeout(tstr); }
            //         else
            //         { sprintf(tstr,"   Trailing Tone pulse:  None\n"); writeout(tstr); }
            //         if (endian)
            //         strcpy(pstr, "MSb");
            //         else
            //         strcpy(pstr, "LSb");
            //         sprintf(tstr,"             Endianess:   %s\n",pstr); writeout(tstr);
            //         sprintf(tstr,"     Pause after block: %5d milliseconds\n\n",tzx_pause); line++; writeout(tstr);
            
            //         sprintf(tstr,"     First: %02X , Last: %02X, Len: %d\n\n",data[0], data[tzx_current_datablock_size-1], tzx_current_datablock_size); line++; writeout(tstr);
            //         }
            //         break;
            //     // C64 Turbo Tape Data Block
            //     case 0x17:  data+=4;
            //         sb_bit0=get_uint16(&data[0]);
            //         sb_bit1=get_uint16(&data[2]);
            //         add_bit=data[4];
            //         num_lead_in=get_uint16(&data[5]);
            //         lead_in_byte=data[7];
            //         lastbyte=(int) data[8];
            //         endian=data[9];
            //         trailing=get_uint16(&data[10]);
            //         sb_trailing=data[12];
            //         tzx_pause=get_uint16(&data[13]);
            //         tzx_current_datablock_size=get_uint24(&data[15]);
            //         data+=18;
            //         IdentifyC64Turbo(tzx_current_datablock_size, data, 1);
            //         if (info==1)
            //         {
            //             sprintf(pstr,"Block %3d (%5X):  17 - C64 Turbo Tape Type Data - %s\n",curr+1,block[curr]+10,tstr); writeout(pstr);
            //             sprintf(tstr,"                Length: %5d bytes\n",tzx_current_datablock_size); writeout(tstr);
            //             sprintf(tstr,"                  Flag: %5d ($%02X)\n",data[0],data[0]); writeout(tstr);
            //             if (num_lead_in)
            //             { sprintf(tstr,"         Lead In Bytes: %5d, Value: %3d ($%02X)\n",num_lead_in,lead_in_byte,lead_in_byte); writeout(tstr); }
            //             else
            //             { sprintf(tstr,"         Lead In Bytes:  None\n"); writeout(tstr); }
            //             sprintf(tstr,"           Bit-0 pulse: %5d T-States\n",sb_bit0); writeout(tstr);
            //             sprintf(tstr,"           Bit-1 pulse: %5d T-States\n",sb_bit1); writeout(tstr);
            //             if (add_bit&3)
            //             {
            //             if (add_bit&4)
            //                 strcpy(pstr, "After");
            //             else
            //                 strcpy(pstr, "Before");
            //             sprintf(tstr,"       Additional Bits: %5d %s Byte, Value %1d\n",add_bit&3, pstr, (add_bit>>3)&1); writeout(tstr);
            //             }
            //             else
            //             { sprintf(tstr,"       Additional Bits:  None\n"); writeout(tstr); }
            //             sprintf(tstr,"        Last byte used: %5d bits\n",lastbyte); writeout(tstr);
            //             if (endian)
            //             strcpy(pstr, "MSb");
            //             else
            //             strcpy(pstr, "LSb");
            //             sprintf(tstr,"             Endianess:   %s\n",pstr); writeout(tstr);
            //             if (trailing)
            //             { sprintf(tstr,"        Trailing Bytes: %5d, Value: %3d ($%02X)\n",trailing,sb_trailing,sb_trailing); writeout(tstr); }
            //             else
            //             { sprintf(tstr,"        Trailing Bytes:  None\n"); writeout(tstr); }
            //             sprintf(tstr,"     Pause after block: %5d milliseconds\n\n",tzx_pause); line++; writeout(tstr);
            //         }
            //         break;
                // Pause or Stop the Tape command
                case 0x20:
                    tzx_pause=get_uint16(&data[0]);
                    amp=LOAMP;
                    if (tzx_pause)
                        {
                        if (info!=1)
                            {
                            if (draw) printf("    Pause                 Length: %2.3fs\n",((float) tzx_pause)/1000.0);
                            if (info!=2) { tzx_pause_audio(amp,tzx_pause); amp=LOAMP; }
                            }
                        else
                            {
                            sprintf(tstr,"Block %3d (%5X):  20 - Pause (Silence)\n",curr+1,block[curr]+10); writeout(tstr);
                            sprintf(tstr,"              Duration: %5d milliseconds\n\n",tzx_pause); line++; writeout(tstr);
                            }
                        }
                    else
                        {
                        if (info!=1)
                            {
                            if (!voc)
                                {
                                if (info!=2)
                                    {
                                    if (draw) printf("    Stop the tape command - Press any key to continue!\n");
                                    tzx_play_audio(amp,sbbuflen<<1);            // finish last block ...
                                    StopSB();
                                    while (!kbhit()) {} k=GetCh();
                                    if (k==27)
                                        {
                                        free(mem);
                                        close(fh);
                                        error("ESCAPE key pressed!");
                                        }
                                    InitSB();
                                    }
                                else
                                    { if (draw) printf("    Stop the tape command!\n"); }
                                }
                            else
                                {
                                if (draw) printf("    Stop the tape command!\n");
                                if (info!=2) { tzx_pause_audio(amp, 5000); amp=LOAMP; }
                                }
                            }
                        else { sprintf(tstr,"Block %3d (%5X):  20 - Stop the Tape Command\n\n",curr+1,block[curr]+10); line++; writeout(tstr); }
                        }
                    break;
            //     // Group Start
            //     case 0x21:
            //         CopyString(pstr,&data[1],data[0]);
            //         if (info!=1) { if (draw) printf("    Group: %s\n",pstr); }
            //         else    {   sprintf(tstr,"Block %3d (%5X):  21 - Group: %s\n\n",curr+1,block[curr]+10, pstr); line++; writeout(tstr); }
            //         if (!expand) draw=0;
            //         break;
            //     // Group End
            //     case 0x22:  if (info!=1) { if (draw) printf("    Group End\n"); }
            //         else    {   sprintf(tstr,"Block %3d (%5X):  22 - Group End\n\n",curr+1,block[curr]+10); line++; writeout(tstr); }
            //         draw=1;
            //         break;
            //     // Jump To Relative
            //     case 0x23:  jump=(signed short) (data[0]+data[1]*256);
            //         if (info!=1)
            //             {
            //             if (draw) printf("    Jump Relative: %d (To Block %d)\n",jump,curr+jump+1);
            //             if (!info) { curr+=jump; curr--; }
            //             }
            //         else    {   sprintf(tstr,"Block %3d (%5X):  23 - Jump Relative: %d (To Block %d)\n\n",curr+1,block[curr]+10, jump, curr+jump+1); line++; writeout(tstr); }
            //         break;
            //     // Loop Start
            //     case 0x24:  loop_start=curr; loop_count=get_uint16(&data[0]);
            //         if (info!=1) { if (draw) printf("    Loop Start, Counter: %d\n",loop_count); }
            //         else    {       sprintf(tstr,"Block %3d (%5X):  24 - Loop Start, Counter: %d\n\n",curr+1,block[curr]+10, loop_count-1); line++; writeout(tstr); }
            //         break;
            //     // Loop End
            //     case 0x25:  if (info!=1)
            //             {
            //             if (info!=2)
            //                 {
            //                 loop_count--;
            //                 if (loop_count>0)
            //                     {
            //                     if (draw) printf("    Loop End, Still To Go %d Time(s)!\n",loop_count);
            //                     curr=loop_start;
            //                     }
            //                 else
            //                     {
            //                     if (draw) printf("    Loop End, Finished\n");
            //                     }
            //                 }
            //             else
            //                 {
            //                 if (draw) printf("    Loop End\n");
            //                 }
            //             }
            //         else
            //             {
            //             sprintf(tstr,"Block %3d (%5X):  25 - Loop End\n\n",curr+1,block[curr]+10); line++; writeout(tstr);
            //             }
            //         break;
            //     // Call Sequence
            //     case 0x26:  call_pos=curr; call_num=get_uint16(&data[0]); call_cur=0;
            //         if (info==1)
            //             {
            //             sprintf(tstr,"Block %3d (%5X):  26 - Call Sequence, Number of Calls : %d\n\n",curr+1,block[curr]+10,call_num); line++; writeout(tstr);
            //             }
            //         else
            //             {
            //             if (info==2)
            //                 {
            //                 if (draw) printf("    Call Sequence, Number of Calls : %d\n",call_num);
            //                 }
            //             else
            //                 {
            //                 jump=(signed short) (data[2]+data[3]*256);
            //                 if (draw) printf("    Call Sequence, Number of Calls : %d, First: %d (To Block %d)\n",call_num,jump,curr+jump+1);
            //                 curr+=jump; curr--;
            //                 }
            //             }
            //         break;
            //     // Return from Sequence
            //     case 0x27:  call_cur++;
            //         if (info==1)
            //             {
            //             sprintf(tstr,"Block %3d (%5X):  27 - Return from Call\n\n",curr+1,block[curr]+10); line++; writeout(tstr);
            //             }
            //         else
            //             {
            //             if (info==2)
            //                 {
            //                 if (draw) printf("    Return from Call\n");
            //                 }
            //             else
            //                 {
            //                 if (call_cur==call_num)
            //                     {
            //                     if (draw) printf("    Return from Call, Last Call Finished\n");
            //                     curr=call_pos;
            //                     }
            //                 else
            //                     {
            //                     curr=call_pos;
            //                     data=&mem[block[curr]+1];
            //                     jump=(signed short) (data[call_cur*2+2]+data[call_cur*2+3]*256);
            //                     if (draw) printf("    Return from Call, Calls Left: %d, Next: %d (To Block %d)\n",
            //                     call_num-call_cur, jump, curr+jump+1);
            //                     curr+=jump; curr--;
            //                     }
            //                 }
            //             }
            //         break;
            //     // Select Block
            //     case 0x28:  num_sel=data[2];
            //         if (info==2)
            //             {
            //             if (draw)
            //                 {
            //                 sprintf(tstr,"    Select block");
            //                 MakeFixedString(tstr, 69);
            //                 strcpy(tstr+52," (/info for more)");
            //                 printf("%s\n",tstr);
            //                 }
            //             }
            //         else
            //             {
            //             if (info==1)
            //                 {
            //                 sprintf(tstr,"Block %3d (%5X):  28 - Select Block\n",curr+1,block[curr]+10); writeout(tstr);
            //                 data+=3;
            //                 for (n=0; n<num_sel; n++)
            //                     {
            //                     jump=(signed short) (data[0]+data[1]*256);
            //                     CopyString(spdstr,&data[3],data[2]);
            //                     sprintf(tstr,"%5d - Jump: %3d (To Block %4d) : %s\n",n+1,jump,curr+jump+1,spdstr); writeout(tstr);
            //                     data+=3+data[2];
            //                     }
            //                 sprintf(tstr,"\n"); writeout(tstr);
            //                 }
            //             else
            //                 {
            //                 printf("    Select :\n");
            //                 data+=3;
            //                 for (n=0; n<num_sel; n++)
            //                     {
            //                     jump=(signed short) (data[0]+data[1]*256);
            //                     jumparray[n]=jump;
            //                     CopyString(spdstr,&data[3],data[2]);
            //                     printf("%5d : %s\n",n+1,spdstr);
            //                     data+=3+data[2];
            //                     }
            //                 printf(">> Press the number!\n");
            //                 if (!voc)
            //                     {
            //                     tzx_play_audio(amp,sbbuflen<<1);            // finish last block ...
            //                     StopSB();
            //                     }
            //                 else
            //                     {
            //                     tzx_pause_audio(amp, 5000); amp=LOAMP;
            //                     }
            //                 while (!kbhit()) {} k=GetCh();
            //                 if (k==27)
            //                     {
            //                     free(mem);
            //                     close(fh);
            //                     error("ESCAPE key pressed!");
            //                     }
            //                 if (!voc) InitSB();
            //                 k-=48;
            //                 if (k<1 || k>num_sel) printf("Illegal Selection... Continuing...\n");
            //                 else { curr+=jumparray[k-1]; curr--; }
            //                 }
            //             }
            //         break;
            //     // Stop the tape if in 48k mode
            //     case 0x2A:  if (info==1)
            //             {
            //             sprintf(tstr,"Block %3d (%5X):  2A - Stop the tape if in 48k mode\n\n",curr+1,block[curr]+10); line++; writeout(tstr);
            //             break;
            //             }
            //         if (info==2)
            //             {
            //             if (draw) printf("    Stop the tape if in 48k mode!\n");
            //             break;
            //             }
            //         if (mode128)
            //             {
            //             if (draw) printf("    Stop the tape only in 48k mode!\n");
            //             }
            //         else
            //             {
            //             if (voc)
            //                 {
            //                 if (draw) printf("    Stop the tape in 48k mode!\n");
            //                 tzx_pause_audio(amp, 5000); amp=LOAMP;
            //                 }
            //             else
            //                 {
            //                 if (draw) printf("    Stop the tape in 48k mode - Press any key to continue!\n");
            //                 tzx_play_audio(amp,sbbuflen<<1);            // finish last block ...
            //                 StopSB();
            //                 while (!kbhit()) {} k=GetCh();
            //                 if (k==27)
            //                     {
            //                     free(mem);
            //                     close(fh);
            //                     error("ESCAPE key pressed!");
            //                     }
            //                 InitSB();
            //                 }
            //             }
            //         break;
            */
                // Description
                case 0x30:
                    printf("Block ID %3d (start: %5X, len: %5X):  30 - Description: ", 
                        tzx_current_block + 1, 
                        tzx_block_offsets[tzx_current_block] + 10, 
                        tzx_current_data[0]); 
                    for(uint8_t description_char = 0; description_char < tzx_current_data[0]; description_char++) {
                        printf("%c", tzx_current_data[description_char+1]);
                    }
                    printf("\n\n"); 
                    break;
                /*
            //     // Message
            //     case 0x31:  CopyString(pstr,&data[2],data[1]);
            //         if (info!=1) { if (draw) printf("    Message: %s\n",pstr); }    // Pause in Message block is ignored ...
            //         else
            //             {
            //             line+=MultiLine(pstr,34,spdstr);
            //             sprintf(tstr,"Block %3d (%5X):  31 - Message: %s\n",curr+1,block[curr]+10, spdstr); writeout(tstr);
            //             sprintf(tstr,"               Duration: %d seconds\n\n",data[0]); line++; writeout(tstr);
            //             }
            //         break;
            //     // Archive Info
            //     case 0x32:  if (info!=1)
            //             {
            //             if (draw)
            //                 {
            //                 if (data[3]==0)
            //                     {
            //                     CopyString(spdstr,&data[5],data[4]);
            //                     sprintf(tstr,"    Title: %s",spdstr);
            //                     MakeFixedString(tstr, 69);
            //                     strcpy(tstr+52," (/info for more)");
            //                     printf("%s\n",tstr);
            //                     }
            //                 else
            //                     {
            //                     sprintf(tstr,"    Archive Info");
            //                     MakeFixedString(tstr, 69);
            //                     strcpy(tstr+52," (/info for more)");
            //                     printf("%s\n",tstr);
            //                     }
            //                 }
            //             }
            //         else
            //             {
            //             num=data[2];
            //             data+=3;
            //             sprintf(tstr,"Block %3d (%5X):  32 - Archive Info:\n",curr+1,block[curr]+10); writeout(tstr);
            //             while(num)
            //                 {
            //                 switch (data[0])
            //                     {
            //                     case    0x00:   sprintf(pstr,"         Title:"); break;
            //                     case    0x01:   sprintf(pstr,"     Publisher:"); break;
            //                     case    0x02:   sprintf(pstr,"     Author(s):"); break;
            //                     case    0x03:   sprintf(pstr,"  Release Date:"); break;
            //                     case    0x04:   sprintf(pstr,"      Language:"); break;
            //                     case    0x05:   sprintf(pstr,"     Game Type:"); break;
            //                     case    0x06:   sprintf(pstr,"         Price:"); break;
            //                     case    0x07:   sprintf(pstr,"        Loader:"); break;
            //                     case    0x08:   sprintf(pstr,"        Origin:"); break;
            //                     default:        sprintf(pstr,"      Comments:"); break;
            //                     }
            //                 CopyString(spdstr,&data[2],data[1]);
            //                 line+=MultiLine(spdstr,16,tstr);
            //                 sprintf(spdstr,"%s %s\n",pstr,tstr); writeout(spdstr);
            //                 data+=data[1]+2;
            //                 num--;
            //                 }
            //             sprintf(tstr,"\n"); writeout(tstr);
            //             }
            //         break;
            //     // Hardware Info
            //     case 0x33:  if (data[1]==0 && data[2]>0x14 && data[2]<0x1a && data[3]==1) cpc=1;
            //         if (data[1]==0 && data[2]==0x09 && data[3]==1) sam=1;
            //         if (info!=1)
            //             {
            //             if (draw)
            //                 {
            //                 if (data[1]!=0 || data[3]!=1)
            //                     {
            //                     sprintf(tstr, "    Hardware Type");
            //                     MakeFixedString(tstr, 69);
            //                     strcpy(tstr+52," (/info for more)");
            //                     printf("%s\n",tstr);
            //                     }
            //                 else
            //                     {
            //                     printf("    This tape is made for %s !\n",hid[0][data[2]]);
            //                     }
            //                 }
            //             }
            //         else
            //             {
            //             num=data[0];
            //             data+=1;
            //             sprintf(tstr,"Block %3d (%5X):  33 - Hardware Info:\n",curr+1,block[curr]+10); writeout(tstr);
            //             for (n=0; n<4; n++)
            //                 {
            //                 prvi=1;
            //                 d=data;
            //                 for (m=0; m<num; m++)
            //                     {
            //                     if (d[2]==n)
            //                         {
            //                         if (prvi)
            //                             {
            //                             prvi=0;
            //                             switch (n)
            //                                 {
            //                                 case 0: sprintf(pstr,"  Runs on the following hardware:\n"); writeout(pstr); break;
            //                                 case 1: sprintf(pstr,"  Uses the following hardware:\n"); writeout(pstr); break;
            //                                 case 2: sprintf(pstr,"  Runs, but it doesn't use the following hardware:\n"); writeout(pstr); break;
            //                                 case 3: sprintf(pstr,"  Doesn't run on the following hardware:\n"); writeout(pstr); break;
            //                                 }
            //                             }
            //                         if (!prvi && last==d[0])
            //                             {
            //                             for (x=0; x<lastlen; x++) spdstr[x]=' '; spdstr[x]=0;
            //                             sprintf(pstr,"      %s  %s\n",spdstr,hid[d[0]][d[1]]); writeout(pstr);
            //                             }
            //                         else
            //                             {
            //                             sprintf(pstr,"      %s: %s\n",htype[d[0]],hid[d[0]][d[1]]); writeout(pstr);
            //                             }
            //                         lastlen=strlen(htype[d[0]]);
            //                         last=d[0];
            //                         }
            //                     d+=3;
            //                     }
            //                 }
            //             sprintf(tstr,"\n"); writeout(tstr);
            //             }
            //         break;
            //     // Emulation info
            //     case 0x34:  if (info!=1) { if (draw) printf("    Information for emulators.\n"); }
            //         else    {   sprintf(tstr,"Block %3d (%5X):  34 - Emulation Info\n\n",curr+1,block[curr]+10); line++; writeout(tstr); }
            //         break;
            //     // Custom Info
            //     case 0x35:  CopyString(pstr,data,16);
            //         if (info!=1)
            //             {
            //             if (draw)
            //                 {
            //                 if (strcmp(pstr,"POKEs           ")) printf("    Custom Info: %s\n",pstr);  // Only Name of Custom info except POKEs is used ...
            //                 else
            //                     {
            //                     sprintf(tstr,"    Custom Info: %s",pstr); 
            //                     MakeFixedString(tstr, 69);
            //                     strcpy(tstr+52," (/info for more)");
            //                     printf("%s\n",tstr);
            //                     }
            //                 }
            //             }
            //         else    {
            //                 sprintf(tstr,"Block %3d (%5X):  35 - Custom Info: %s\n",curr+1,block[curr]+10,pstr); writeout(tstr);
            //                 if (!strcmp(pstr,"POKEs           "))
            //                     {
            //                     data+=20;
            //                     if (data[0])
            //                         {
            //                         sprintf(pstr,"  Description:");
            //                         CopyString(spdstr,&data[1],data[0]);
            //                         line+=MultiLine(spdstr,15,tstr)+1;
            //                         sprintf(spdstr,"%s %s\n\n",pstr,tstr); writeout(spdstr);
            //                         }
            //                     data+=data[0]+1;
            //                     numt=data[0]; data++;
            //                     sprintf(pstr,"          Trainer Description                       Poke Val Org Page\n"); writeout(pstr);
            //                     sprintf(pstr,"         -------------------------------------------------------------\n"); writeout(pstr);
            //                     while (numt)
            //                         {
            //                         CopyString(pstr,&data[1],data[0]);
            //                         data+=data[0]+1;
            //                         nump=data[0]; data++;
            //                         for (n=0; n<nump; n++)
            //                             {
            //                             sprintf(spdstr,"          %s",pstr);
            //                             MakeFixedString(spdstr,48);
            //                             if (data[0]&8)  strcpy(tstr2,"   -");
            //                             else            sprintf(tstr2,"%4d",data[0]&7);
            //                             if (data[0]&32) strcpy(tstr,"  -");
            //                             else            sprintf(tstr,"%3d",data[4]);
            //                             if (data[0]&16) strcpy(tstr3,"  -");
            //                             else            sprintf(tstr3,"%3d",data[3]);
            //                             if (n>0) strcpy(tstr4,"+");
            //                             else        strcpy(tstr4," ");
            //                             sprintf(pstr,"%s %s %5d %s %s %s\n",spdstr, tstr4, get_uint16(&data[1]), tstr3, tstr, tstr2);
            //                             writeout(pstr);
            //                             data+=5;
            //                             pstr[0]=0;
            //                             }
            //                         numt--;
            //                         }
            //                     }
            //                 sprintf(tstr,"\n"); writeout(tstr);
            //                 }
            //         break;
            //     // Snapshot
            //     case 0x40:  if (info!=1) { if (draw) printf("    Snapshot               (Not Supported yet)\n"); }
            //         else    {
            //                 sprintf(tstr,"Block %3d (%5X):  40 - Snapshot\n\n",curr+1,block[curr]+10); line++; writeout(tstr);
            //                 switch (data[0])
            //                     {
            //                     case 0:   sprintf(pstr,"Type: Z80"); break;
            //                     case 1:   sprintf(pstr,"Type: SNA"); break;
            //                     default : sprintf(pstr,"Unknown Type"); break;
            //                     }
            //                 sprintf(tstr,"                      %s\n\n",pstr); line++; writeout(tstr);
            //                 }

            //         break;
            //     // ZXTape!xx
            //     case 0x5A:  if (info!=1) { if (draw) printf("    Start of the new tape  (Merged Tapes)\n"); }
            //         else    {   sprintf(tstr,"Block %3d (%5X):  5A - Merget Tapes\n\n",curr+1,block[curr]+10); line++; writeout(tstr); }
            //         break;
            */
                    // Other (unknown) blocks
                    default:
                        printf("Block ID %3d (start: %5X):  %02X Unknown Block \n\n",
                            tzx_current_block + 1,
                            tzx_current_data[tzx_data_current_position] + 10,
                            tzx_current_block_type);
                        break;
        }
        tzx_current_block_type++;

        if (tzx_current_block_type==0x10 
            || tzx_current_block_type==0x11 
            || tzx_current_block_type==0x14)    // One of the data blocks ...
        {
            if (tzx_current_block_type != 0x14){
                printf("  Length:%6d  ",tzx_current_datablock_size);
                // Identify(tzx_current_datablock_size,data,0);
            }
            else
            {
                printf("    Pure Data           ");
            }
            if (tzx_current_block_type==0x10)
            {
                   printf("Normal Speed");
            }
            else
            {
                      printf(" Speed: %3d%%", tzx_speed);
            }
            if (tzx_current_block==tzx_blockcount-1)
            {
                printf("\n");
            }
            else
            {
                printf(",Pause: %2.3fs",((float) tzx_pause)/1000.0);
            }

            while (tzx_pilot)
            {
                tzx_play_audio(tzx_amp,tzx_audio_pilot);
                tzx_toggle_amplitude();
                tzx_pilot--;
            }   // Play PILOT TONE
            if (tzx_audio_sync1)
            {
                tzx_play_audio(tzx_amp,tzx_audio_sync1);
                tzx_toggle_amplitude();
            }  // Play SYNC PULSES
            if (tzx_audio_sync2)
            {
                tzx_play_audio(tzx_amp,tzx_audio_sync2);
                tzx_toggle_amplitude();
            }
            
            tzx_current_datablock_position=0;
            while (tzx_current_datablock_size)                                                 // Play actual DATA
            {
                if (tzx_current_datablock_size!=1)
                {
                    tzx_bits_in_this_byte=8;
                }
                else
                {
                    tzx_bits_in_this_byte=tzx_bits_in_last_byte;
                }
                tzx_this_byte=tzx_current_data[tzx_current_datablock_position];
                while (tzx_bits_in_this_byte)
                {
                    if (tzx_this_byte&0x80) 
                    {
                        tzx_audio_bit=tzx_audio_bit1;
                    }
                    else
                    {
                        tzx_audio_bit=tzx_audio_bit0;
                    }
                    tzx_play_audio(tzx_amp,tzx_audio_bit);
                    tzx_toggle_amplitude();
                    tzx_play_audio(tzx_amp,tzx_audio_bit);
                    tzx_toggle_amplitude();
                    tzx_this_byte<<=1;
                    tzx_bits_in_this_byte--;
                }
                tzx_current_datablock_size--; tzx_current_datablock_position++;
            }
            // If there is pause after block present then make first millisecond the oposite
            // pulse of last pulse played and the rest in LOAMP ... otherwise don't do ANY pause
            if (tzx_pause)
            {
                tzx_pause_audio(tzx_amp,1);
                tzx_amp=LOAMP;
                if (tzx_pause>1) {
                    tzx_pause_audio(tzx_amp,tzx_pause-1);
                }
            }
        }

        if (tzx_current_block_type==0x16)    // C64 ROM data block ...
        {
/*
            IdentifyC64ROM(tzx_current_datablock_size, data, 0);
            if (curr==numblocks-1)  sprintf(pstr,"\0");
            else                    sprintf(pstr,",Pause: %2.3fs",((float) pause)/1000.0);
            if (draw) printf(" %s Length:%6d  %s%s\n",tstr,tzx_current_datablock_size,spdstr,pstr);
            if (info!=2)
                {
                sb_pilot=tzx_ticks_to_samples(sb_pilot);
                sb_sync1=tzx_ticks_to_samples(sb_sync1); sb_sync2=tzx_ticks_to_samples(sb_sync2);
                sb_bit1_f=tzx_ticks_to_samples(sb_bit1_f); sb_bit1_s=tzx_ticks_to_samples(sb_bit1_s);
                sb_bit0_f=tzx_ticks_to_samples(sb_bit0_f); sb_bit0_s=tzx_ticks_to_samples(sb_bit0_s);
                sb_finishbyte_f=tzx_ticks_to_samples(sb_finishbyte_f);
                sb_finishbyte_s=tzx_ticks_to_samples(sb_finishbyte_s);
                sb_finishdata_f=tzx_ticks_to_samples(sb_finishdata_f);
                sb_finishdata_s=tzx_ticks_to_samples(sb_finishdata_s);
                sb_trailing=tzx_ticks_to_samples(sb_trailing);
                num_lead_in=0;
                amp=LOAMP;      // This might be just opposite !!!!
                while (pilot) { PlayC64SB(sb_pilot); pilot--; }    // Play PILOT TONE
                if (sb_sync1) PlayC64SB(sb_sync1);                 // Play SYNC PULSES
                if (sb_sync2) PlayC64SB(sb_sync2);
                tzx_current_datablock_position=0;
                while (tzx_current_datablock_size)                                                 // Play actual DATA
                    {
                    if (tzx_current_datablock_size!=1)
                        {
                        bitcount=8;
                        PlayC64ROMByte(data[tzx_current_datablock_position], 0);
                        }
                    else
                        {
                        bitcount=lastbyte;
                        PlayC64ROMByte(data[tzx_current_datablock_position], 1);
                        }			 
                    tzx_this_byte=data[tzx_current_datablock_position];
                    tzx_current_datablock_size--; tzx_current_datablock_position++;
                    }
                while (trailing) { PlayC64SB(sb_trailing); trailing--; }    // Play TRAILING TONE
                // If there is pause after block present then make first millisecond the oposite
                // pulse of last pulse played and the rest in LOAMP ... otherwise don't do ANY pause


    //            if (tzx_pause) { tzx_pause_audio(amp,1); amp=LOAMP; if (tzx_pause>1) tzx_pause_audio(amp,tzx_pause-1); }


                if (tzx_pause)
                {
                    tzx_pause_audio(amp, tzx_pause/2); tzx_toggle_amplitude();
                    tzx_pause_audio(amp, (tzx_pause/2)+(tzx_pause%2)); tzx_toggle_amplitude();
                }
            }
*/
        }

        if (tzx_current_block_type==0x17)    // C64 Turbo Tape data block ...
        {
/*
            IdentifyC64Turbo(tzx_current_datablock_size, data, 0);
            if (curr==numblocks-1)  sprintf(pstr,"\0");
            else                    sprintf(pstr,",Pause: %2.3fs",((float) tzx_pause)/1000.0);
            if (draw) printf(" %s Length:%6d  %s%s\n",tstr,tzx_current_datablock_size,spdstr,pstr);
            if (info!=2)
                {
                sb_bit1=tzx_ticks_to_samples(sb_bit1);
                sb_bit0=tzx_ticks_to_samples(sb_bit0);
                amp=LOAMP;      // This might be just opposite !!!!
                while (num_lead_in)     // Lead In bytes !
                    {
                    bitcount=8;
                    PlayC64TurboByte(lead_in_byte);
                    num_lead_in--;
                    }
                tzx_current_datablock_position=0;
                while (tzx_current_datablock_size)                                                 // Play actual DATA
                    {
                    if (tzx_current_datablock_size!=1) bitcount=8; else bitcount=lastbyte;
                    PlayC64TurboByte(data[tzx_current_datablock_position]);
                    tzx_this_byte=data[tzx_current_datablock_position];
                    tzx_current_datablock_size--; tzx_current_datablock_position++;
                    }
                while (trailing)     // Trailing bytes !
                    {
                    bitcount=8;
                    PlayC64TurboByte((unsigned char) sb_trailing);
                    trailing--;
                    }
                // If there is pause after block present then make first millisecond the oposite
                // pulse of last pulse played and the rest in LOAMP ... otherwise don't do ANY pause


    //            if (tzx_pause) { tzx_pause_audio(amp,1); amp=LOAMP; if (tzx_pause>1) tzx_pause_audio(amp,tzx_pause-1); }


                if (tzx_pause)
                {
                    tzx_pause_audio(amp, tzx_pause/2); tzx_toggle_amplitude();
                    tzx_pause_audio(amp, (tzx_pause/2)+(tzx_pause%2)); tzx_toggle_amplitude();
                }
            }
    */
        }
        tzx_current_block++;
        printf("\n\n");
    }




    // SINE WAVE
    // printf("Sine tone at %dHz \n", sinefreq);

    // for (i = 0; i < BUFFER_LEN; i++)
    // {
    //     buffer[active_buffer][i] = (sin(((2 * M_PI * sinefreq) / samplefreq) * i))*255;
    // }

    // frames = snd_pcm_writei(alsa_handle, buffer[0], BUFFER_LEN);  //sending values to sound driver

    err = snd_pcm_drain(alsa_handle);
    if (err < 0)
    {
        printf("snd_pcm_drain failed: %s\n", snd_strerror(err));
    }
    snd_pcm_close(alsa_handle);
    return 0;
}
