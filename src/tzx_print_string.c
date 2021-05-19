#include "tzx_print_string.h"

void tzx_print_string(char *data, int len)
{
	for (int n = 0; n < len; n++) {
		printf("%c", data[n]);
	}
}
