#ifndef _tzxpi_h
#define _tzxpi_h

#include <stdio.h>
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


//************************************************************************************************//
// 
// "Global" variables
// 
//************************************************************************************************//
extern bool DEBUG;

#endif // _tzxpi_h