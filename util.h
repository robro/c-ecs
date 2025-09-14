#ifndef UTIL_H
#define UTIL_H

#include <sys/types.h>
#include <time.h>

#define NSECS_IN_SEC 1000000000

#define array_size(array) (sizeof(array) / sizeof(*array))

struct Vec2 {
	float x;
	float y;
};

#define VEC_ZERO (struct Vec2){0, 0}

struct timespec timespec_diff(const struct timespec *time_a, const struct timespec *time_b);

float timespec_to_secs(const struct timespec *time);

void free_multiple(void **array, uint size);

#endif
