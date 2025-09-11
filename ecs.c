#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

#define DEBUG 1

#define MAX_ENTITIES 1000000
#define TARGET_FPS 60
#define GAME_LOOPS 100

#define SECS_PER_FRAME (1.0 / TARGET_FPS)
#define NSECS_IN_SEC 1000000000

#define array_length(arr) (sizeof(arr) / sizeof(arr[0]))

struct timespec diff_timespec(const struct timespec *time_a, const struct timespec *time_b) {
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

struct Vec2 {
	float x;
	float y;
};

#define VEC_ZERO (struct Vec2){0, 0}
#define GRAVITY (struct Vec2){0, 9.8}

/* ==== COMPONENTS ================================== */

struct Component {
	const struct ComponentInterface *vtable;
	bool active;
};

struct ComponentInterface {
	void (*array_insert)(const struct Component *, uint);
};

struct ComponentPhysics {
	struct Component base;
	struct Vec2 position;
	struct Vec2 velocity;
	struct Vec2 gravity;
};

struct ComponentJumper {
	struct Component base;
	float jump_force;
	float ground_height;
};

struct ComponentShaker {
	struct Component base;
	float shake_speed;
};

struct ComponentPhysics component_physics[MAX_ENTITIES];
struct ComponentJumper component_jumpers[MAX_ENTITIES];
struct ComponentShaker component_shakers[MAX_ENTITIES];

void component_array_insert(const struct Component *component, uint index) {
	component->vtable->array_insert(component, index);
}

void component_array_insert_physics(const struct Component *component, uint index) {
	component_physics[index] = *(struct ComponentPhysics *)component;
}

void component_array_insert_jumper(const struct Component *component, uint index) {
	component_jumpers[index] = *(struct ComponentJumper *)component;
}

void component_array_insert_shaker(const struct Component *component, uint index) {
	component_shakers[index] = *(struct ComponentShaker *)component;
}

struct Component *component_create_physics(struct Vec2 position, struct Vec2 velocity, struct Vec2 gravity) {
	static const struct ComponentInterface vtable = {
		.array_insert = component_array_insert_physics
	};
	struct ComponentPhysics *physics = malloc(sizeof(struct ComponentPhysics));
	if (physics == NULL) {
		return NULL;
	}
	physics->base.vtable = &vtable;
	physics->base.active = true;
	physics->position = position;
	physics->velocity = velocity;
	physics->gravity = gravity;

	return (struct Component *)physics;
}

struct Component *component_create_jumper(float jump_force, float ground_height) {
	static const struct ComponentInterface vtable = {
		.array_insert = component_array_insert_jumper
	};
	struct ComponentJumper *jumper = malloc(sizeof(struct ComponentJumper));
	if (jumper == NULL) {
		return NULL;
	}
	jumper->base.vtable = &vtable;
	jumper->base.active = true;
	jumper->jump_force = jump_force;
	jumper->ground_height = ground_height;
	
	return (struct Component *)jumper;
}

struct Component *component_create_shaker(float shake_speed) {
	static const struct ComponentInterface vtable = {
		.array_insert = component_array_insert_shaker
	};
	struct ComponentShaker *shaker = malloc(sizeof(struct ComponentShaker));
	if (shaker == NULL) {
		return NULL;
	}
	shaker->base.vtable = &vtable;
	shaker->base.active = true;
	shaker->shake_speed = shake_speed;

	return (struct Component *)shaker;
}

/* ==== ENTITIES ==================================== */

uint entity_index_last_free;
bool entities_alive[MAX_ENTITIES];

int entity_index_get_free() {
	int i = entity_index_last_free;
	while (entities_alive[i]) {
		i++;
		i %= MAX_ENTITIES;
		if (i == entity_index_last_free) {
			return -1;
		}
	}
	entity_index_last_free = i;
	return entity_index_last_free;
}

bool entity_index_is_valid(uint entity_index) {
	if (entity_index >= MAX_ENTITIES) {
		return false;
	}
	return true;
}

void entity_set_alive(uint entity_index) {
	if (entity_index_is_valid(entity_index)) {
		entities_alive[entity_index] = true;
	}
}

/*
 * Takes NULL terminated array of Component pointers.
 * Returns entity index or -1 if no free indices.
 */
int entity_create(struct Component **components) {
	int entity_index = entity_index_get_free();
	if (entity_index < 0) {
		return -1;
	}
	for (int i = 0; components[i]; ++i) {
		component_array_insert(components[i], entity_index);
	}
	entity_set_alive(entity_index);
	return entity_index;
}

/* ==== SYSTEMS ==================================== */

void update_jumpers(float delta) {
	for (int i = 0; i < MAX_ENTITIES; ++i) {
		if (!component_jumpers[i].base.active || !component_physics[i].base.active) {
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
		if (!component_shakers[i].base.active || !component_physics[i].base.active) {
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
		if (!component_physics[i].base.active) {
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
	struct Component *test_entity[] = {
		component_create_physics(VEC_ZERO, VEC_ZERO, GRAVITY),
		component_create_jumper(100.0, 0.0),
		component_create_shaker(100.0),
		NULL
	};

	for (int i = 0; i < MAX_ENTITIES; ++i) {
		entity_create(test_entity);
	}

	struct timespec time_start, time_end, sleep_time, work_time, frame_time;
	const struct timespec target_frame_time = {.tv_sec = 0, .tv_nsec = NSECS_IN_SEC / TARGET_FPS};

	// Game Loop
#if DEBUG
	for (int i = 0; i < GAME_LOOPS; ++i) {
#else
	while (1) {
#endif
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
#if DEBUG
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
