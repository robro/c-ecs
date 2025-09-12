#include <stdio.h>
#include <time.h>
#include "component.h"
#include "entity.h"

#define MAX_ENTITIES 1000000
#define TARGET_FPS 60
#define FRAMES 100

#define GRAVITY (struct Vec2){0, 9.8}

#define SECS_PER_FRAME (1.0 / TARGET_FPS)

/* ==== COMPONENTS ================================== */

const struct ComponentPhysics test_physics = {
	.position = VEC_ZERO,
	.velocity = VEC_ZERO,
	.gravity = GRAVITY,
	.active = true
};
const struct ComponentJumper test_jumper = {
	.jump_force = 69,
	.ground_height = 420,
	.active = true
};
const struct ComponentShaker test_shaker = {
	.shake_speed = 69,
	.active = true
};
const struct ComponentLifetime test_lifetime = {
	.lifetime = 1,
	.active = true
};

/* ==== SYSTEMS ==================================== */

struct ComponentPhysics components_physics[MAX_ENTITIES];
struct ComponentJumper components_jumpers[MAX_ENTITIES];
struct ComponentShaker components_shakers[MAX_ENTITIES];
struct ComponentLifetime components_lifetimes[MAX_ENTITIES];

void update_jumpers(float delta) {
	for (int i = 0; i < MAX_ENTITIES; ++i) {
		if (!entity_is_alive(i) || !components_jumpers[i].active) {
			return;
		}
		if (components_physics[i].position.y >= components_jumpers[i].ground_height) {
			components_physics[i].position.y = components_jumpers[i].ground_height;
			components_physics[i].velocity.y = -components_jumpers[i].jump_force;
		}
	}
}

void update_shakers(float delta) {
	for (int i = 0; i < MAX_ENTITIES; ++i) {
		if (!entity_is_alive(i) || !components_shakers[i].active) {
			return;
		}
		if (components_physics[i].velocity.x >= 0) {
			components_physics[i].velocity.x = -components_shakers[i].shake_speed;
		} else {
			components_physics[i].velocity.x = components_shakers[i].shake_speed;
		}
	}
}

void update_physics(float delta) {
	for (int i = 0; i < MAX_ENTITIES; ++i) {
		if (!entity_is_alive(i) || !components_physics[i].active) {
			return;
		}
		// Add gravity
		components_physics[i].velocity.x += components_physics[i].gravity.x * delta;
		components_physics[i].velocity.y += components_physics[i].gravity.y * delta;

		// Update position
		components_physics[i].position.x += components_physics[i].velocity.x * delta;
		components_physics[i].position.y += components_physics[i].velocity.y * delta;
	}
}

void update_lifetime(float delta) {
	for (int i = 0; i < MAX_ENTITIES; ++i) {
		if (!entity_is_alive(i) || !components_lifetimes[i].active) {
			return;
		}
		if (components_lifetimes[i].lifetime <= 0) {
			entity_set_dead(i);
			return;
		}
		components_lifetimes[i].lifetime -= delta;
	}
}

typedef void (*UpdateFunc)(float);
const UpdateFunc update_funcs[] = {
	update_jumpers,
	update_shakers,
	update_physics,
	update_lifetime,
	NULL
};

int main(void) {
	if (!entity_initialize_entities(MAX_ENTITIES)) {
		printf("Entity initialization failed\n");
		return 1;
	}
	for (int i = 0; i < MAX_ENTITIES; ++i) {
		components_physics[i] = test_physics;
		components_jumpers[i] = test_jumper;
		components_shakers[i] = test_shaker;
		components_lifetimes[i] = test_lifetime;
		entity_set_alive(i);
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
