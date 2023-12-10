/**
 * hdr_time.h
 * Written by Michael Barker and released to the public domain,
 * as explained at http://creativecommons.org/publicdomain/zero/1.0/
 */

#ifndef HDR_TIME_H__
#define HDR_TIME_H__

#include <math.h>
#include <time.h>

#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)

typedef struct hdr_timespec {
  long tv_sec;
  long tv_nsec;
} hdr_timespec;

#else

typedef struct timespec hdr_timespec;

#endif

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_MSC_VER)
void hdr_gettime(hdr_timespec *t);
#else
void hdr_gettime(hdr_timespec *t);
#endif

#ifdef __cplusplus
}
#endif

#endif
