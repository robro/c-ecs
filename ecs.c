#include <bits/time.h>
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

typedef struct {
	Vec2 position;
	Vec2 velocity;
	Vec2 gravity;
} ComponentPhysics;

typedef struct {
	float jump_force;
	float ground_height;
} ComponentJumper;

typedef struct {
	float shake_speed;
} ComponentShaker;

ComponentPhysics component_physics[MAX_ENTITIES] = {};
ComponentJumper component_jumpers[MAX_ENTITIES] = {};
ComponentShaker component_shakers[MAX_ENTITIES] = {};

bool entity_index_valid(uint entity_index);

void component_physics_set(uint entity_index, ComponentPhysics *data) {
	if (entity_index_valid(entity_index)) {
		component_physics[entity_index] = *data;
	}
}

void component_jumper_set(uint entity_index, ComponentJumper *data) {
	if (entity_index_valid(entity_index)) {
		component_jumpers[entity_index] = *data;
	}
}

void component_shaker_set(uint entity_index, ComponentShaker *data) {
	if (entity_index_valid(entity_index)) {
		component_shakers[entity_index] = *data;
	}
}

/* ==== ENTITIES ==================================== */

uint last_entity_index = 0;
bool active_entities[MAX_ENTITIES] = {};

int entity_index_get() {
	int i = last_entity_index;
	while (active_entities[i]) {
		i++;
		i %= MAX_ENTITIES;
		if (i == last_entity_index) {
			return -1;
		}
	}
	last_entity_index = i;
	return last_entity_index;
}

bool entity_index_valid(uint entity_index) {
	if (entity_index >= MAX_ENTITIES) {
		return false;
	}
	return true;
}

void entity_activate(uint entity_index) {
	if (entity_index_valid(entity_index)) {
		active_entities[entity_index] = true;
	}
}

/* ==== SYSTEMS ==================================== */

void update_jumpers(float delta) {
	for (int i = 0; i < MAX_ENTITIES; ++i) {
		if (!active_entities[i]) {
			return;
		}
		if (component_physics[i].position.y >= component_jumpers[i].ground_height) {
			component_physics[i].position.y = component_jumpers[i].ground_height;
			component_physics[i].velocity.y = -component_jumpers[i].jump_force;
		}
	}
}

void update_shakers(float delta) {
	for (int i = 0; i < MAX_ENTITIES; ++i) {
		if (!active_entities[i]) {
			return;
		}
		if (component_physics[i].velocity.x >= 0) {
			component_physics[i].velocity.x = -component_shakers[i].shake_speed;
		} else {
			component_physics[i].velocity.x = component_shakers[i].shake_speed;
		}
	}
}

void update_physics(float delta) {
	for (int i = 0; i < MAX_ENTITIES; ++i) {
		if (!active_entities[i]) {
			return;
		}
		// Add gravity
		component_physics[i].velocity.x += component_physics[i].gravity.x * delta;
		component_physics[i].velocity.y += component_physics[i].gravity.y * delta;

		// Update position
		component_physics[i].position.x += component_physics[i].velocity.x * delta;
		component_physics[i].position.y += component_physics[i].velocity.y * delta;
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
	int entity_index;
	for (int i = 0; i < MAX_ENTITIES; ++i) {
		entity_index = entity_index_get();
		if (entity_index < 0) {
			continue;
		}
		component_jumper_set(entity_index, &(ComponentJumper){
			.jump_force = 100.0,
			.ground_height = 0.0
		});
		component_shaker_set(entity_index, &(ComponentShaker){
			.shake_speed = 100.0
		});
		component_physics_set(entity_index, &(ComponentPhysics){
			.position = VEC_ZERO,
			.velocity = VEC_ZERO,
			.gravity = GRAVITY
		});
		entity_activate(entity_index);
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
