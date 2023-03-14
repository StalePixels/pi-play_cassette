#ifndef _tzxpi_h
#define _tzxpi_h

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include <alsa/asoundlib.h>
#include <math.h>

#define TZX_SUPPORTED_MAJOR_REV         1        // Major revision of the format this program supports
#define TZX_SUPPORTED_MINOR_REV         13       // Minor revision of the same

extern const char TZXTapeHeaderID[];
extern const char TAPTapeHeaderID[];

//TZX File Tasks
#define GETFILEHEADER         0
#define GETID                 1
#define PROCESSID             2

//TZX ID Tasks
#define READPARAM             0
#define PILOT                 1
#define SYNC1                 2
#define SYNC2                 3
#define DATA                  4
#define PAUSE                 5

//Buffer size
#define buffsize              64

//Spectrum Standards
#define PILOTLENGTH           619
#define SYNCFIRST             191
#define SYNCSECOND            210
#define ZEROPULSE             244
#define ONEPULSE              489
#define PILOTNUMBERL          8063
#define PILOTNUMBERH          3223
#define PAUSELENGTH           1000

#define SAMPLE_LEN          48000
#define BUFFER_LEN			(SAMPLE_LEN*4)

//************************************************************************************************//
// 
// "Global" variables
// 
//************************************************************************************************//
extern bool DEBUG;
extern bool VERBOSE;
extern int samplefreq;         //soundcard audio sample frequency

extern snd_output_t *output;            // snd output
// float buffer[BUFFER_LEN];
extern uint8_t buffer[2][BUFFER_LEN];          // rotating buggers
extern uint8_t active_buffer;                  // which of our buffers we are using

extern const char TZXTapeHeaderID[];                   // TZX identifing header
extern const char TAPTapeHeaderID[];                   // TAP identifying header (not yet used)
extern int buf_index;
extern snd_pcm_t *alsa_handle;
#endif // _tzxpi_h