#ifndef _UTIL_H
#define _UTIL_H

#include <time.h>

#define NSECS_IN_SEC 1000000000

struct Vec2 {
	float x;
	float y;
};

struct timespec timespec_diff(const struct timespec *time_a, const struct timespec *time_b);

float timespec_to_secs(const struct timespec *time);

#endif
