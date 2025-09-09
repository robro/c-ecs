#include <bits/time.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdbool.h>

#define DEBUG_BUILD 1

#define MAX_ENTITIES 1000000
#define TARGET_FPS 60
#define GAME_LOOPS 100

#define SECS_PER_FRAME (1.0 / TARGET_FPS)
#define NSECS_IN_SEC 1000000000

#define array_length(arr) (sizeof(arr) / sizeof(arr[0]))

typedef struct timespec timespec;

timespec diff_timespec(const timespec *time_a, const timespec *time_b) {
	timespec diff = {
		.tv_sec = time_a->tv_sec - time_b->tv_sec,
		.tv_nsec = time_a->tv_nsec - time_b->tv_nsec,
	};
	if (diff.tv_nsec < 0) {
		diff.tv_nsec += NSECS_IN_SEC;
		diff.tv_sec--;
	}
	return diff;
}

float timespec_to_secs(const timespec *time) {
	return time->tv_sec + (float)time->tv_nsec / NSECS_IN_SEC;
}

typedef struct {
	float x;
	float y;
} Vec2;

#define VEC_ZERO (Vec2){0, 0}
#define GRAVITY (Vec2){0, 9.8}

/* ==== COMPONENTS ================================== */

typedef enum {
	CT_NONE,
	CT_PHYSICS,
	CT_JUMPER,
	CT_SHAKER,
} ComponentType;

typedef struct {
	ComponentType type;
	bool active;
} ComponentHeader;

typedef struct {
	Vec2 position;
	Vec2 velocity;
	Vec2 gravity;
} ComponentPhysicsData;

typedef struct {
	float jump_force;
	float ground_height;
} ComponentJumperData;

typedef struct {
	float shake_speed;
} ComponentShakerData;

typedef struct {
	ComponentHeader header;
	ComponentPhysicsData data;
} ComponentPhysics;

typedef struct {
	ComponentHeader header;
	ComponentJumperData data;
} ComponentJumper;

typedef struct {
	ComponentHeader header;
	ComponentShakerData data;
} ComponentShaker;

ComponentPhysics component_physics[MAX_ENTITIES] = {};
ComponentJumper component_jumpers[MAX_ENTITIES] = {};
ComponentShaker component_shakers[MAX_ENTITIES] = {};

/* ==== ENTITIES ==================================== */

uint entity_index = 0;
uint total_entities = 0;

int entity_create(ComponentHeader *components[], size_t component_count) {
	for (int i = 0; i < component_count; ++i) {
		switch (components[i]->type) {
		case CT_NONE:
			return -1;
		case CT_PHYSICS:
			component_physics[entity_index] = *(ComponentPhysics *)components[i];
			break;
		case CT_JUMPER:
			component_jumpers[entity_index] = *(ComponentJumper *)components[i];
			break;
		case CT_SHAKER:
			component_shakers[entity_index] = *(ComponentShaker *)components[i];
			break;
		}
	}
	total_entities++;
	return entity_index++;
}

/* ==== SYSTEMS ==================================== */

void update_jumpers(float delta) {
	for (int i = 0; i < total_entities; ++i) {
		if (!component_jumpers[i].header.active || !component_physics[i].header.active) {
			return;
		}
		if (component_physics[i].data.position.y >= component_jumpers[i].data.ground_height) {
			component_physics[i].data.position.y = component_jumpers[i].data.ground_height;
			component_physics[i].data.velocity.y = -component_jumpers[i].data.jump_force;
		}
	}
}

void update_shakers(float delta) {
	for (int i = 0; i < total_entities; ++i) {
		if (!component_shakers[i].header.active || !component_physics[i].header.active) {
			return;
		}
		if (component_physics[i].data.velocity.x >= 0) {
			component_physics[i].data.velocity.x = -component_shakers[i].data.shake_speed;
		} else {
			component_physics[i].data.velocity.x = component_shakers[i].data.shake_speed;
		}
	}
}

void update_physics(float delta) {
	for (int i = 0; i < total_entities; ++i) {
		if (!component_physics[i].header.active) {
			return;
		}
		// Add gravity
		component_physics[i].data.velocity.x += component_physics[i].data.gravity.x * delta;
		component_physics[i].data.velocity.y += component_physics[i].data.gravity.y * delta;

		// Update position
		component_physics[i].data.position.x += component_physics[i].data.velocity.x * delta;
		component_physics[i].data.position.y += component_physics[i].data.velocity.y * delta;
	}
}

typedef void (*UpdateFunc)(float);
const UpdateFunc update_funcs[] = {
	update_jumpers,
	update_shakers,
	update_physics,
};
const size_t update_funcs_count = array_length(update_funcs);

int main() {
	ComponentHeader *components[] = {
		(ComponentHeader *)&(ComponentShaker){.header = {.type = CT_SHAKER, .active = true}, .data = {.shake_speed = 100.0}},
		(ComponentHeader *)&(ComponentJumper){.header = {.type = CT_JUMPER, .active = true}, .data = {.jump_force = 100.0, .ground_height = 0.0}},
		(ComponentHeader *)&(ComponentPhysics){.header = {.type = CT_PHYSICS, .active = true}, .data = {.position = VEC_ZERO, .velocity = VEC_ZERO, .gravity = GRAVITY}},
	};

	for (int i = 0; i < MAX_ENTITIES; ++i) {
		entity_create(components, array_length(components));
	}

	timespec time_start, time_end, sleep_time, work_time, frame_time;
	const timespec target_frame_time = {.tv_sec = 0, .tv_nsec = NSECS_IN_SEC / TARGET_FPS};

	// Game Loop
	while (1) {
	// for (int i = 0; i < GAME_LOOPS; ++i) {
		clock_gettime(CLOCK_MONOTONIC, &time_start);
		for (int j = 0; j < update_funcs_count; ++j) {
			update_funcs[j](SECS_PER_FRAME);
		}
		clock_gettime(CLOCK_MONOTONIC, &time_end);
		work_time = diff_timespec(&time_end, &time_start);
		sleep_time = diff_timespec(&target_frame_time, &work_time);
		if (sleep_time.tv_sec >= 0 && sleep_time.tv_nsec > 0) {
			nanosleep(&sleep_time, NULL);
		}
#if DEBUG_BUILD
		clock_gettime(CLOCK_MONOTONIC, &time_end);
		frame_time = diff_timespec(&time_end, &time_start);
		printf(
			"Work time: %lf secs | Frame time: %lf secs\n",
			timespec_to_secs(&work_time),
			timespec_to_secs(&frame_time)
		);
#endif
	}

	return 0;
}
