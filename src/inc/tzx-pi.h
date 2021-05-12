#include <stdio.h>
#include <stdbool.h>

#include <alsa/asoundlib.h>
#include <math.h>

const char TZXTape[7] = {'Z','X','T','a','p','e','!'};
const char TAPcheck[7] = {'T','A','P','t','a','p','.'};

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