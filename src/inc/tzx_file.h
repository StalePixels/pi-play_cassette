#include <stdio.h>
#include <stdint.h>

#ifndef _tzxfile_h
#define _tzxfile_h

//TZX block list - uncomment as supported, list taken straight from sadken's TZXduino
#define TZX_ID10_STANDARD                0x10    //Standard speed data block
#define TZX_ID11_TURBO                   0x11    //Turbo speed data block
#define TZX_ID12_TONE                    0x12    //Pure tone
#define TZX_ID13_PULSES                  0x13    //Sequence of pulses of various lengths
// #define TZX_ID14                0x14    //Pure data block
// #define TZX_ID15                0x15    //Direct recording block
// #define TZX_ID18                0x18    //CSW recording block
// #define TZX_ID19                0x19    //Generalized data block
#define TZX_ID20_PAUSE                   0x20    //Pause (silence) ot 'Stop the tape' command
#define TZX_ID21_GROUP_START             0x21    //Group start
#define TZX_ID22_GROUP_END               0x22    //Group end
// #define TZX_ID23                0x23    //Jump to block
// #define TZX_ID24                0x24    //Loop start
// #define TZX_ID25                0x25    //Loop end
// #define TZX_ID26                0x26    //Call sequence
// #define TZX_ID27                0x27    //Return from sequence
// #define TZX_ID28                0x28    //Select block
// #define TZX_ID2A                0x2A    //Stop the tape is in 48K mode
// #define TZX_ID2B                0x2B    //Set signal level
#define TZX_ID30_DESCRIPTION             0x30    //Text description
// #define TZX_ID31                0x31    //Message block
// #define TZX_ID32                0x32    //Archive info
// #define TZX_ID33                0x33    //Hardware type
// #define TZX_ID35                0x35    //Custom info block
// #define TZX_ID5A                0x5A    //Glue block (90 dec, ASCII Letter 'Z')
// #define TZX_TAP                 0xFE    //Tap File Mode
// #define TZX_EOF                 0xFF    //End of file

#define ERR_TZX_FILE_SIZE_ERROR     0x81
#define ERR_TZX_FILE_OPEN_ERROR     0x82
#define ERR_TZX_FILE_NO_MEMORY      0x83
#define ERR_TZX_NOT_TZX_FILE        0x84
#define ERR_TZX_BETAS_NOT_SUPPORTED 0x85
#define ERR_TZX_IS_ACTUALLY_TAP     0x86

#define ERR_INVALID_STARTING_BLOCK  0x88
#define ERR_INVALID_ENDING_BLOCK    0x89

extern char *tzx_filename;
extern FILE *tzx_filehandle;
extern int32_t tzx_filesize;
extern int8_t *tzx_data;
extern uint8_t tzx_version_major, tzx_version_minor;

#endif // _tzxfile_h
