#include "tzx_block_info.h"

void tzx_block_info(int len, char *temp, int type)
{
    int n;
    
    if (temp[0]==0 && (len==19 || len==20) && temp[1]<4)
    {
        if (!type)
        {
            switch (temp[1])
                {
                case 0x00: printf("    Program : "); break;
                case 0x01: printf(" Num. Array : "); break;
                case 0x02: printf("Char. Array : "); break;
                case 0x03: printf("      Bytes : "); break;
                }
        }
        else
        {
            switch (temp[1])
                {
                case 0x00: printf("Program : "); break;
                case 0x01: printf("Num. Array : "); break;
                case 0x02: printf("Char. Array : "); break;
                case 0x03: printf("Bytes : "); break;
                }
        }
        for (n=0; n<10; n++)
        {
            if (temp[n+2]>31 && temp[n+2]<127)
            {
                printf("%c", temp[n+2]);
            }
            else
            {
                printf(" ");
            }
        }
    }
    else
    {
        if (!type) {
            printf("    --------------------");        // Not Header
        }
        else
        {
            printf("Headerless");
        }
    }
}