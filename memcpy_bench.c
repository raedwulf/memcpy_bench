/**
 * Copyright (c) 2013 Tai Chi Minh Ralph Eastwood <tcmreastwood@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <getopt.h>
#include <stddef.h>
#include <unistd.h>

#if defined(__APPLE__) && defined(__MACH__)
#include <mach/mach_time.h>
#else
#include <time.h>
#endif

#if defined(__APPLE__) && defined(__MACH__)
  static mach_timebase_info_data_t timebase_info;
#endif

static inline uint64_t utime() {
#if defined(__APPLE__) && defined(__MACH__)
    uint64_t tm = mach_absolute_time();
    return (tm * timebase_info.numer) / (1000 * timebase_info.denom);
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
#endif
}

static int help_flag;
static int check_flag;
static int read_offset;
static int write_offset;
static int size;
static int iterations;
static int algtype;
static uint8_t *src_data;
static uint8_t *dst_data;

#define ALIGNMENT 16
#define ALIGN(p) ((((uintptr_t)p) % ALIGNMENT) ? (p + ALIGNMENT - (((uintptr_t)p) % ALIGNMENT)) : p)

extern void *memcpy_movsb(void * destination, const void * source, size_t num);
extern void *memcpy_sse2(void * destination, const void * source, size_t num);

int main(int argc, char **argv) {
	int c;
	check_flag = 0;
	help_flag = 0;
	read_offset = 0;
	write_offset = 0;
	iterations = 100;
	size = 10000;
	algtype = 0;

	while (1) {
		static struct option long_options[] = {
			{"check", no_argument, &check_flag, 1},
			{"read_offset", required_argument, 0, 'r'},
			{"write_offset", required_argument, 0, 'w'},
			{"size",  required_argument, 0, 's'},
			{"iterations",  required_argument, 0, 'i'},
			{"type", required_argument, 0, 't'},
			{"help", no_argument, &help_flag, 'h'},
			{0, 0, 0, 0}
		};

		int option_index = 0;
		c = getopt_long(argc, argv, "o:s:i:t:hc", long_options, &option_index);

		if (c == -1)
			break;
		switch (c) {
		case 0:
			if (long_options[option_index].flag != 0)
				break;
			break;
		case 's':
			sscanf(optarg, "%d", &size);
			break;
		case 'r':
			sscanf(optarg, "%d", &read_offset);
			break;
		case 'w':
			sscanf(optarg, "%d", &write_offset);
			break;
		case 'i':
			sscanf(optarg, "%d", &iterations);
			break;
		case 't':
			sscanf(optarg, "%d", &algtype);
			break;
		case 'h':
			help_flag = 1;
			break;
		case 'c':
			check_flag = 1;
			break;
		default:
			return 1;
		}
	}

	if (help_flag) {
		printf("Usage: %s [-s num] [-o num] [-i num] [-h] filename\n"
		       "    -s    set the size of the data to copy\n"
		       "    -o    set the offset to copy from (default=0)\n"
		       "    -i    set the iterations to run (default=100)\n"
		       "    -t    algorithm type (0=c,1=rep movsb,2=sse2)\n"
		       "    -h    this help\n", argv[0]);
		return 0;
	}

	if (read_offset < 0 || read_offset > ALIGNMENT) {
		fprintf(stderr,
		        "%s: option --read_offset must be between 0 and 15\n",
		        argv[0]);
		return 1;
	}

	if (write_offset < 0 || write_offset > ALIGNMENT) {
		fprintf(stderr,
		        "%s: option --write_offset must be between 0 and 15\n",
		        argv[0]);
		return 1;
	}

	if (size < 0 || size < read_offset || size < write_offset) {
		fprintf(stderr,
		        "%s: option --size must be greater than 0 and offset\n",
		        argv[0]);
		return 1;
	}

	if (algtype < 0 || algtype > 2) {
		fprintf(stderr,
		        "%s: option --type must be between 0 and 2\n",
		        argv[0]);
		return 1;
	}


#if defined(__APPLE__) && defined(__MACH__)
	mach_timebase_info(&timebase_info);
#endif

	src_data = malloc(size + ALIGNMENT * 2);
	dst_data = malloc(size + ALIGNMENT * 2);

	uint8_t *src = ALIGN(src_data) + read_offset;
	memset(src_data, 0, size + ALIGNMENT * 2);
	for (int i = read_offset; i < size + read_offset; i++)
		src[i] = rand() % 255 + 1;

	uint8_t *dst = ALIGN(dst_data) + write_offset;
	memset(dst_data, 0, size + ALIGNMENT * 2);

	uint64_t start, end;

	switch (algtype) {
	case 0:
		start = utime();
		for (int i = 0; i < iterations; i++)
			memcpy(dst, src, size);
		end = utime();
		break;
	case 1:
		start = utime();
		for (int i = 0; i < iterations; i++)
			memcpy_movsb(dst, src, size);
		end = utime();
		break;
	case 2:
		start = utime();
		for (int i = 0; i < iterations; i++)
			memcpy_sse2(dst, src, size);
		end = utime();
		break;
	}

	if (check_flag) {
		for (int i = 0; i < size; i++) {
			if (src[i] != dst[i]) {
				fprintf(stderr, "%s: bad copy %d\n",
					argv[0], i);
				break;
			}
		}
		for (int i = 0; i < read_offset; i++) {
			if (src[i] != 0) {
				fprintf(stderr,	"%s: overflow before src %d\n",
					argv[0],
					i);
			}
		}
		for (int i = 0; i < write_offset; i++) {
			if (dst[i] != 0) {
				fprintf(stderr, "%s: overflow before dst %d\n",
					argv[0], i);
			}
		}
		for (int i = size; i < size + read_offset; i++) {
			if (src[i] != 0) {
				fprintf(stderr, "%s: overflow before src %d\n",
					argv[0], i);
			}
		}
		for (int i = size; i < size + write_offset; i++) {
			if (dst[i] != 0) {
				fprintf(stderr,	"%s: overflow before dst %d\n",
					argv[0], i);
			}
		}
	}

	printf("%llu\n", end - start);

	return 0;
}
