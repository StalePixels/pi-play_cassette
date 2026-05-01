#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "tape.h"

#define Z80_HZ  3500000

int tape_open(const char *path, libspectrum_tape **out)
{
	struct stat st;
	if (stat(path, &st) < 0) {
		perror(path);
		return -1;
	}

	FILE *f = fopen(path, "rb");
	if (!f) {
		perror(path);
		return -1;
	}

	unsigned char *buf = malloc(st.st_size);
	if (!buf) {
		fclose(f);
		return -1;
	}
	if ((off_t)fread(buf, 1, st.st_size, f) != st.st_size) {
		fprintf(stderr, "%s: short read\n", path);
		free(buf);
		fclose(f);
		return -1;
	}
	fclose(f);

	libspectrum_tape *tape = libspectrum_tape_alloc();
	if (libspectrum_tape_read(tape, buf, st.st_size,
	                          LIBSPECTRUM_ID_UNKNOWN, path)
	    != LIBSPECTRUM_ERROR_NONE) {
		fprintf(stderr, "%s: not a recognised tape file\n", path);
		libspectrum_tape_free(tape);
		free(buf);
		return -1;
	}

	free(buf);
	*out = tape;
	return 0;
}

void tape_close(libspectrum_tape *tape)
{
	libspectrum_tape_free(tape);
}

int tape_play(libspectrum_tape *tape, struct audio *out)
{
	int            level = 0;
	libspectrum_dword tstates;
	int            flags = 0;

	while (!(flags & LIBSPECTRUM_TAPE_FLAGS_STOP)) {
		if (libspectrum_tape_get_next_edge(&tstates, &flags, tape)
		    != LIBSPECTRUM_ERROR_NONE) {
			fprintf(stderr, "tape iteration failed\n");
			return -1;
		}

		if (flags & LIBSPECTRUM_TAPE_FLAGS_LEVEL_LOW)  level = 0;
		if (flags & LIBSPECTRUM_TAPE_FLAGS_LEVEL_HIGH) level = 1;

		long samples = ((long)tstates * out->rate) / Z80_HZ;
		if (audio_emit(out, level, samples) < 0)
			return -1;

		if (!(flags & (LIBSPECTRUM_TAPE_FLAGS_LEVEL_LOW
		             | LIBSPECTRUM_TAPE_FLAGS_LEVEL_HIGH)))
			level = !level;
	}

	return audio_drain(out);
}
