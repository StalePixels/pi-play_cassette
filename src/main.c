#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libspectrum.h>

#include "audio.h"
#include "tape.h"

#define DEFAULT_RATE  44100

static void usage(const char *argv0)
{
	fprintf(stderr,
	        "usage: %s [-r rate] <tape-file>\n"
	        "  -r rate  output sample rate (Hz, default %d)\n",
	        argv0, DEFAULT_RATE);
}

int main(int argc, char **argv)
{
	unsigned rate = DEFAULT_RATE;
	int opt;

	while ((opt = getopt(argc, argv, "r:h")) != -1) {
		switch (opt) {
		case 'r':
			rate = (unsigned)atoi(optarg);
			if (!rate) {
				fprintf(stderr, "bad rate: %s\n", optarg);
				return 1;
			}
			break;
		case 'h':
		default:
			usage(argv[0]);
			return opt == 'h' ? 0 : 1;
		}
	}

	if (optind >= argc) {
		usage(argv[0]);
		return 1;
	}

	if (libspectrum_init() != LIBSPECTRUM_ERROR_NONE) {
		fprintf(stderr, "libspectrum_init failed\n");
		return 1;
	}

	libspectrum_tape *tape = NULL;
	if (tape_open(argv[optind], &tape) < 0)
		return 1;

	struct audio out;
	if (audio_open(&out, rate) < 0) {
		tape_close(tape);
		return 1;
	}

	int rc = tape_play(tape, &out);

	audio_close(&out);
	tape_close(tape);
	libspectrum_end();

	return rc < 0 ? 1 : 0;
}
