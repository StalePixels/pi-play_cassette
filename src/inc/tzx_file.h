#include <stdint.h>

//TZX block list - uncomment as supported
// #define TXZ_ID10                0x10    //Standard speed data block
// #define TXZ_ID11                0x11    //Turbo speed data block
// #define TXZ_ID12                0x12    //Pure tone
// #define TXZ_ID13                0x13    //Sequence of pulses of various lengths
// #define TXZ_ID14                0x14    //Pure data block
// #define TXZ_ID15                0x15    //Direct recording block
// #define TXZ_ID18                0x18    //CSW recording block
// #define TXZ_ID19                0x19    //Generalized data block
// #define TXZ_ID20                0x20    //Pause (silence) ot 'Stop the tape' command
// #define TXZ_ID21                0x21    //Group start
// #define TXZ_ID22                0x22    //Group end
// #define TXZ_ID23                0x23    //Jump to block
// #define TXZ_ID24                0x24    //Loop start
// #define TXZ_ID25                0x25    //Loop end
// #define TXZ_ID26                0x26    //Call sequence
// #define TXZ_ID27                0x27    //Return from sequence
// #define TXZ_ID28                0x28    //Select block
// #define TXZ_ID2A                0x2A    //Stop the tape is in 48K mode
// #define TXZ_ID2B                0x2B    //Set signal level
// #define TXZ_ID30                0x30    //Text description
// #define TXZ_ID31                0x31    //Message block
// #define TXZ_ID32                0x32    //Archive info
// #define TXZ_ID33                0x33    //Hardware type
// #define TXZ_ID35                0x35    //Custom info block
// #define TXZ_ID5A                0x5A    //Glue block (90 dec, ASCII Letter 'Z')
// #define TXZ_TAP                 0xFE    //Tap File Mode
// #define TXZ_EOF                 0xFF    //End of file

#define ERR_TZX_FILE_OPEN_ERROR 0x81


extern char *tzx_filename;
extern int tzx_filehandle;