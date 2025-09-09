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

typedef enum {
	NONE,
	PHYSICS,
	JUMPER,
	SHAKER,
} ComponentType;

typedef union {
	ComponentPhysicsData physics;
	ComponentJumperData jumper;
	ComponentShakerData shaker;
} ComponentData;

typedef struct {
	ComponentType type;
	ComponentData data;
} Component;

Component component_physics_get(Vec2 position, Vec2 velocity, Vec2 gravity) {
	return (Component){.type = PHYSICS, .data.physics = {
		position,
		velocity,
		gravity
	}};
}

Component component_jumper_get(float jump_force, float ground_height) {
	return (Component){.type = JUMPER, .data.jumper = {
		jump_force,
		ground_height
	}};
}

Component component_shaker_get(float shake_speed) {
	return (Component){.type = SHAKER, .data.shaker = {
		shake_speed
	}};
}

Component components_physics[MAX_ENTITIES] = {};
Component components_jumpers[MAX_ENTITIES] = {};
Component components_shakers[MAX_ENTITIES] = {};

/* ==== ENTITIES ==================================== */

uint entity_index = 0;
uint total_entities = 0;

int entity_create(Component components[], size_t component_count) {
	for (int i = 0; i < component_count; ++i) {
		switch (components[i].type) {
		case NONE:
			return -1;
		case JUMPER:
			components_jumpers[entity_index] = components[i];
			break;
		case SHAKER:
			components_shakers[entity_index] = components[i];
			break;
		case PHYSICS:
			components_physics[entity_index] = components[i];
			break;
		}
	}
	total_entities++;
	return entity_index++;
}

/* ==== SYSTEMS ==================================== */

void update_jumpers(float delta) {
	for (int i = 0; i < total_entities; ++i) {
		if (components_jumpers[i].type != JUMPER || components_physics[i].type != PHYSICS) {
			return;
		}
		if (components_physics[i].data.physics.position.y >= components_jumpers[i].data.jumper.ground_height) {
			components_physics[i].data.physics.position.y = components_jumpers[i].data.jumper.ground_height;
			components_physics[i].data.physics.velocity.y = -components_jumpers[i].data.jumper.jump_force;
		}
	}
}

void update_shakers(float delta) {
	for (int i = 0; i < total_entities; ++i) {
		if (components_shakers[i].type != SHAKER || components_physics[i].type != PHYSICS) {
			return;
		}
		if (components_physics[i].data.physics.velocity.x >= 0) {
			components_physics[i].data.physics.velocity.x = -components_shakers[i].data.shaker.shake_speed;
		} else {
			components_physics[i].data.physics.velocity.x = components_shakers[i].data.shaker.shake_speed;
		}
	}
}

void update_physics(float delta) {
	for (int i = 0; i < total_entities; ++i) {
		if (components_physics[i].type != PHYSICS) {
			return;
		}
		// Add gravity
		components_physics[i].data.physics.velocity.x += components_physics[i].data.physics.gravity.x * delta;
		components_physics[i].data.physics.velocity.y += components_physics[i].data.physics.gravity.y * delta;

		// Update position
		components_physics[i].data.physics.position.x += components_physics[i].data.physics.velocity.x * delta;
		components_physics[i].data.physics.position.y += components_physics[i].data.physics.velocity.y * delta;
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
	Component components[] = {
		component_shaker_get(100.0),
		component_jumper_get(100.0, 0.0),
		component_physics_get(VEC_ZERO, VEC_ZERO, GRAVITY)
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
