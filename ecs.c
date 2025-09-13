#include <stdio.h>
#include <time.h>
#include "component.h"
#include "entity.h"
#include "system.h"

#define MAX_ENTITIES 1000000
#define TARGET_FPS 60
#define FRAMES 100

#define GRAVITY (struct Vec2){0, 9.8}

#define SECS_PER_FRAME (1.0 / TARGET_FPS)

/* ==== COMPONENTS ================================== */

const struct Component test_physics = {
	.type = CT_PHYSICS,
	.physics = {
		.position = VEC_ZERO,
		.velocity = VEC_ZERO,
		.gravity = GRAVITY,
		.active = true,
	}
};
const struct Component test_jumper = {
	.type = CT_JUMPER,
	.jumper = {
		.jump_force = 69,
		.ground_height = 420,
		.active = true
	}
};
const struct Component test_shaker = {
	.type = CT_SHAKER,
	.shaker = {
		.shake_speed = 69,
		.active = true
	}
};
const struct Component test_lifetime = {
	.type = CT_LIFETIME,
	.lifetime = {
		.lifetime = 1,
		.active = true
	}
};

const struct Component *test_entity[] = {
	&test_physics,
	&test_jumper,
	&test_shaker,
	&test_lifetime,
	NULL
};

/* ==== SYSTEMS ==================================== */
int main(void) {
	if (!component_initialize_components(MAX_ENTITIES)) {
		printf("Components initialization failed\n");
		return 1;
	};
	if (!entity_initialize_entities(MAX_ENTITIES)) {
		printf("Entity initialization failed\n");
		return 1;
	}
	for (int i = 0; i < MAX_ENTITIES; ++i) {
		entity_add(test_entity);
	}

	struct timespec time_start, time_end, sleep_time, work_time, frame_time;
	const struct timespec target_frame_time = {.tv_sec = 0, .tv_nsec = NSECS_IN_SEC / TARGET_FPS};

	// Game Loop
	for (int i = 0; i < FRAMES; ++i) {
		clock_gettime(CLOCK_MONOTONIC, &time_start);
		for (int j = 0; update_funcs[j]; ++j) {
			update_funcs[j](SECS_PER_FRAME);
		}
		clock_gettime(CLOCK_MONOTONIC, &time_end);
		work_time = timespec_diff(&time_end, &time_start);
		sleep_time = timespec_diff(&target_frame_time, &work_time);
		if (sleep_time.tv_sec >= 0 && sleep_time.tv_nsec > 0) {
			nanosleep(&sleep_time, NULL);
		}
		clock_gettime(CLOCK_MONOTONIC, &time_end);
		frame_time = timespec_diff(&time_end, &time_start);
		printf(
			"Frame: %d | Work time: %lf secs | Frame time: %lf secs\n",
			i + 1,
			timespec_to_secs(&work_time),
			timespec_to_secs(&frame_time)
		);
	}

	return 0;
}
