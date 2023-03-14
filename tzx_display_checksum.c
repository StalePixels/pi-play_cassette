#include "tzx_display_checksum.h"

void tzx_display_checksum(uint8_t *data,int len)
{
    // Calculates a XOR checksum for a block and returns a STRING containing the result
    char c=0;
    int n;

    for (n=0; n<len-1; n++) c^=data[n];
    if (c==data[len-1])
    {
        printf("OK");
    }
    else
    {
        printf("Wrong, should be %3d ($%02X)",c,c);
    }
}
