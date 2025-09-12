#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

#define MAX_ENTITIES 1000000
#define TARGET_FPS 60
#define FRAMES 100

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
	struct ComponentInterface *vtable;
};

struct ComponentInterface {
	void (*array_insert)(const struct Component *, uint);
};

struct ComponentDataPhysics {
	struct Vec2 position;
	struct Vec2 velocity;
	struct Vec2 gravity;
	bool active;
};

struct ComponentPhysics {
	struct Component base;
	struct ComponentDataPhysics data;
};

struct ComponentDataJumper {
	float jump_force;
	float ground_height;
	bool active;
};

struct ComponentJumper {
	struct Component base;
	struct ComponentDataJumper data;
};

struct ComponentDataShaker {
	float shake_speed;
	bool active;
};

struct ComponentShaker {
	struct Component base;
	struct ComponentDataShaker data;
};

struct ComponentDataLifetime {
	float lifetime;
	bool active;
};

struct ComponentLifetime {
	struct Component base;
	struct ComponentDataLifetime data;
};

struct ComponentDataPhysics component_data_physics[MAX_ENTITIES];
struct ComponentDataJumper component_data_jumpers[MAX_ENTITIES];
struct ComponentDataShaker component_data_shakers[MAX_ENTITIES];
struct ComponentDataLifetime component_data_lifetimes[MAX_ENTITIES];

void component_array_insert(const struct Component *component, uint index) {
	component->vtable->array_insert(component, index);
}

void component_array_insert_physics(const struct Component *component, uint index) {
	component_data_physics[index] = ((struct ComponentPhysics *)component)->data;
}

void component_array_insert_jumper(const struct Component *component, uint index) {
	component_data_jumpers[index] = ((struct ComponentJumper *)component)->data;
}

void component_array_insert_shaker(const struct Component *component, uint index) {
	component_data_shakers[index] = ((struct ComponentShaker *)component)->data;
}

void component_array_insert_lifetime(const struct Component *component, uint index) {
	component_data_lifetimes[index] = ((struct ComponentLifetime *)component)->data;
}

struct Component* component_get_physics(const struct ComponentDataPhysics *data) {
	static struct ComponentInterface vtable = {
		.array_insert = component_array_insert_physics
	};
	struct ComponentPhysics *c_physics = malloc(sizeof(struct ComponentPhysics));
	c_physics->base.vtable = &vtable;
	c_physics->data = *data;
	return (struct Component *)c_physics;
}

struct Component* component_get_jumper(const struct ComponentDataJumper *data) {
	static struct ComponentInterface vtable = {
		.array_insert = component_array_insert_jumper
	};
	struct ComponentJumper *c_jumper = malloc(sizeof(struct ComponentJumper));
	c_jumper->base.vtable = &vtable;
	c_jumper->data = *data;
	return (struct Component *)c_jumper;
}

struct Component* component_get_shaker(const struct ComponentDataShaker *data) {
	static struct ComponentInterface vtable = {
		.array_insert = component_array_insert_shaker
	};
	struct ComponentShaker *c_shaker = malloc(sizeof(struct ComponentJumper));
	c_shaker->base.vtable = &vtable;
	c_shaker->data = *data;
	return (struct Component *)c_shaker;
}

struct Component* component_get_lifetime(const struct ComponentDataLifetime *data) {
	static struct ComponentInterface vtable = {
		.array_insert = component_array_insert_lifetime
	};
	struct ComponentLifetime *c_lifetime = malloc(sizeof(struct ComponentLifetime));
	c_lifetime->base.vtable = &vtable;
	c_lifetime->data = *data;
	return (struct Component *)c_lifetime;
}

const struct ComponentDataPhysics test_physics = {.position = VEC_ZERO, .velocity = VEC_ZERO, .gravity = GRAVITY, .active = true};
const struct ComponentDataJumper test_jumper = {.jump_force = 69, .ground_height = 420, .active = true};
const struct ComponentDataShaker test_shaker = {.shake_speed = 69, .active = true};
const struct ComponentDataLifetime test_lifetime = {.lifetime = 1, .active = true};

/* ==== ENTITIES ==================================== */

uint entity_index_last_free;
bool entities_alive[MAX_ENTITIES];

int entity_index_get_free(void) {
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

void entity_set_alive(uint index) {
	if (index >= MAX_ENTITIES) {
		return;
	}
	entities_alive[index] = true;
}

void entity_set_dead(uint index) {
	if (index >= MAX_ENTITIES) {
		return;
	}
	entities_alive[index] = false;
}

bool entity_is_alive(uint index) {
	if (index >= MAX_ENTITIES) {
		return false;
	}
	return entities_alive[index];
}

/*
 * Takes NULL terminated array of Component pointers.
 * Returns entity index or -1 if no free indices.
 */
int entity_create(const struct Component *components[]) {
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
		if (!entity_is_alive(i) || !component_data_jumpers[i].active) {
			return;
		}
		if (component_data_physics[i].position.y >= component_data_jumpers[i].ground_height) {
			component_data_physics[i].position.y = component_data_jumpers[i].ground_height;
			component_data_physics[i].velocity.y = -component_data_jumpers[i].jump_force;
		}
	}
}

void update_shakers(float delta) {
	for (int i = 0; i < MAX_ENTITIES; ++i) {
		if (!entity_is_alive(i) || !component_data_shakers[i].active) {
			return;
		}
		if (component_data_physics[i].velocity.x >= 0) {
			component_data_physics[i].velocity.x = -component_data_shakers[i].shake_speed;
		} else {
			component_data_physics[i].velocity.x = component_data_shakers[i].shake_speed;
		}
	}
}

void update_physics(float delta) {
	for (int i = 0; i < MAX_ENTITIES; ++i) {
		if (!entity_is_alive(i) || !component_data_physics[i].active) {
			return;
		}
		// Add gravity
		component_data_physics[i].velocity.x += component_data_physics[i].gravity.x * delta;
		component_data_physics[i].velocity.y += component_data_physics[i].gravity.y * delta;

		// Update position
		component_data_physics[i].position.x += component_data_physics[i].velocity.x * delta;
		component_data_physics[i].position.y += component_data_physics[i].velocity.y * delta;
	}
}

void update_lifetime(float delta) {
	for (int i = 0; i < MAX_ENTITIES; ++i) {
		if (!entity_is_alive(i) || !component_data_lifetimes[i].active) {
			return;
		}
		if (component_data_lifetimes[i].lifetime <= 0) {
			entity_set_dead(i);
			return;
		}
		component_data_lifetimes[i].lifetime -= delta;
	}
}

typedef void (*UpdateFunc)(float);
const UpdateFunc update_funcs[] = {
	update_jumpers,
	update_shakers,
	update_physics,
	update_lifetime,
};
const size_t update_funcs_count = array_length(update_funcs);

int main(void) {
	const struct Component *test_components[] = {
		component_get_physics(&test_physics),
		component_get_jumper(&test_jumper),
		component_get_shaker(&test_shaker),
		component_get_lifetime(&test_lifetime),
		NULL
	};

	for (int i = 0; i < MAX_ENTITIES; ++i) {
		entity_create(test_components);
	}

	struct timespec time_start, time_end, sleep_time, work_time, frame_time;
	const struct timespec target_frame_time = {.tv_sec = 0, .tv_nsec = NSECS_IN_SEC / TARGET_FPS};

	// Game Loop
	for (int i = 0; i < FRAMES; ++i) {
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
		clock_gettime(CLOCK_MONOTONIC, &time_end);
		frame_time = diff_timespec(&time_end, &time_start);
		printf(
			"Frame: %d | Work time: %lf secs | Frame time: %lf secs\n",
			i + 1,
			timespec_to_secs(&work_time),
			timespec_to_secs(&frame_time)
		);
	}

	return 0;
}
