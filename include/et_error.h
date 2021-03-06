#ifndef include_ET_ERROR_H
#define include_ET_ERROR_H

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define et_assert(hr) \
	do{ \
		if(!(hr)){ \
			fprintf(stderr, "et_assert: %s()[%d]:'%s'\n", __func__, __LINE__, #hr); \
			assert(hr); \
		} \
	}while(0);

#define et_assertf(hr, fmt, ...) \
	do{ \
		if(!(hr)){ \
			fprintf(stderr, "et_assertf: %s()[%d]: "fmt"\n", \
					__func__, __LINE__, ## __VA_ARGS__); \
			assert(hr); \
		} \
	}while(0);

#define et_abortf(fmt, ...) \
	do{ \
		fprintf(stderr, "et_bortf: %s()[%d]: "fmt"\n", \
				__func__, __LINE__, ## __VA_ARGS__); \
		abort(); \
	}while(0);

// Caution: depend gcc
#define et_bug(fmt, ...)  \
	fprintf(stderr, "BUG: %s()[%d]: "fmt"\n", __func__, __LINE__, ## __VA_ARGS__)
#define et_fixme(fmt, ...)  \
	fprintf(stderr, "FIXME: %s()[%d]: "fmt"\n", __func__, __LINE__, ## __VA_ARGS__)

#define et_critical(fmt, ...)  \
	fprintf(stderr, "critical: %s()[%d]: "fmt"\n", __func__, __LINE__, ## __VA_ARGS__)
#define et_error(fmt, ...)  \
	fprintf(stderr, "error: %s()[%d]: "fmt"\n", __func__, __LINE__, ## __VA_ARGS__)
#define et_warning(fmt, ...)  \
	fprintf(stderr, "warning: %s()[%d]: "fmt"\n", __func__, __LINE__, ## __VA_ARGS__)
#define et_debug(fmt, ...)  \
	fprintf(stdout, "debug: %s()[%d]: "fmt"\n", __func__, __LINE__, ## __VA_ARGS__)

#ifdef include_ET_TEST
#endif // include_ET_TEST

#endif // include_ET_ERROR_H

