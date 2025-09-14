#include <stdlib.h>
#include "util.h"

struct timespec timespec_diff(const struct timespec *time_a, const struct timespec *time_b) {
	struct timespec diff = {
		.tv_sec = time_a->tv_sec - time_b->tv_sec,
		.tv_nsec = time_a->tv_nsec - time_b->tv_nsec,
	};
	if (diff.tv_nsec < 0) {
		diff.tv_nsec += NSECS_IN_SEC;
		diff.tv_sec--;
	}
	return diff;
}

float timespec_to_secs(const struct timespec *time) {
	return time->tv_sec + (float)time->tv_nsec / NSECS_IN_SEC;
}

void free_multiple(void **array, uint size) {
	for (int i = 0; i < size; ++i) {
		free(array[i]);
		array[i] = NULL;
	}
}
