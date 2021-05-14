#include "tzx_get_block_type.h"

char tzx_block_type[32];

char *tzx_get_block_type(int len, char *data, int type)
{
	if (data[0] == 0 && (len == 19 || len == 20) && data[1] < 4)
	{
		switch (data[1])
		{
		case 0x00:
			sprintf(tzx_block_type, "         Program: ");
			break;

		case 0x01:
			sprintf(tzx_block_type, "      Num. Array: ");
			break;

		case 0x02:
			sprintf(tzx_block_type, "     Char. Array: ");
			break;

		case 0x03:
			sprintf(tzx_block_type, "           Bytes: ");
			break;
		}

		uint8_t n;	// defined here, so we can keep the data outside the loop later
		for (n = 0; n < 10; n++)
		{
			if (data[n + 2] > 31 && data[n + 2] < 127)
			{
				tzx_block_type[n + 18] = data[n + 2];
			}
			else
			{
				tzx_block_type[n + 18] = '?';
			}
		}
		tzx_block_type[n + 18] = 0;
	}
	else
	{
		if (!type) {
			sprintf(tzx_block_type, "            Type: <<NO TYPE>>");
		}
		else
		{
			sprintf(tzx_block_type, "            Type: Headerless");
		}
	}

	return tzx_block_type;
}