#include <stdlib.h>

#include "ecs.h"

uint pool_size;
uint last_free_index;
bool initialized;
bool *entities_alive;

struct ComponentPhysics *components_physics;
struct ComponentJumper *components_jumpers;
struct ComponentShaker *components_shakers;
struct ComponentLifetime *components_lifetimes;

void free_multiple(void **array) {
	for (int i = 0; array[i] != NULL; ++i) {
		free(array[i]);
		array[i] = NULL;
	}
}

bool ecs_allocate(uint size) {
	void *arrays[5] = {};
	arrays[0] = calloc(size, sizeof(*entities_alive));
	if (arrays[0] == NULL) {
		free_multiple(arrays);
		return false;
	}
	arrays[1] = calloc(size, sizeof(*components_physics));
	if (arrays[1] == NULL) {
		free_multiple(arrays);
		return false;
	}
	arrays[2] = calloc(size, sizeof(*components_jumpers));
	if (arrays[2] == NULL) {
		free_multiple(arrays);
		return false;
	}
	arrays[3] = calloc(size, sizeof(*components_shakers));
	if (arrays[3] == NULL) {
		free_multiple(arrays);
		return false;
	}
	arrays[4] = calloc(size, sizeof(*components_lifetimes));
	if (arrays[4] == NULL) {
		free_multiple(arrays);
		return false;
	}
	ecs_free();
	entities_alive = arrays[0];
	components_physics = arrays[1];
	components_jumpers = arrays[2];
	components_shakers = arrays[3];
	components_lifetimes = arrays[4];
	return true;
}

void free_arrays() {
	void *arrays[] = {
		entities_alive,
		components_physics,
		components_jumpers,
		components_shakers,
		components_lifetimes,
		NULL
	};
	free_multiple(arrays);
}

int entity_get_free_index() {
	if (!initialized) {
		return -1;
	}
	uint i = last_free_index;
	while (entities_alive[i]) {
		i++;
		i %= pool_size;
		if (i == last_free_index) {
			return -1;
		}
	}
	last_free_index = i;
	return i;
}

void entity_set_alive(uint index) {
	if (!initialized) {
		return;
	}
	if (index < pool_size) {
		entities_alive[index] = true;
	}
}

void entity_set_dead(uint index) {
	if (!initialized) {
		return;
	}
	if (index < pool_size) {
		entities_alive[index] = false;
	}
}

bool entity_is_alive(uint index) {
	if (!initialized || index >= pool_size) {
		return false;
	}
	return entities_alive[index];
}

void update_jumpers(float delta) {
	for (int i = 0; i < pool_size; ++i) {
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
	for (int i = 0; i < pool_size; ++i) {
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
	for (int i = 0; i < pool_size; ++i) {
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
	for (int i = 0; i < pool_size; ++i) {
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

bool ecs_initialize(uint size) {
	if (size == 0) {
		return false;
	}
	if (!ecs_allocate(size)) {
		return false;
	}
	pool_size = size;
	last_free_index = 0;
	initialized = true;
	return true;
}

bool ecs_add_entity(const struct Component **components) {
	int entity_index = entity_get_free_index();
	if (entity_index < 0) {
		return false;
	}
	for (int i = 0; components[i] != NULL; ++i) {
		switch (components[i]->type) {
		case CT_NONE:
			break;
		case CT_PHYSICS:
			components_physics[entity_index] = components[i]->physics;
			break;
		case CT_JUMPER:
			components_jumpers[entity_index] = components[i]->jumper;
			break;
		case CT_SHAKER:
			components_shakers[entity_index] = components[i]->shaker;
			break;
		case CT_LIFETIME:
			components_lifetimes[entity_index] = components[i]->lifetime;
			break;
		}
	}
	entity_set_alive(entity_index);
	return true;
}

void ecs_update(float delta) {
	for (int i = 0; update_funcs[i] != NULL; ++i) {
		update_funcs[i](delta);
	}
}

void ecs_free(void) {
	if (!initialized) {
		return;
	}
	free_arrays();
	pool_size = 0;
	last_free_index = 0;
	initialized = false;
}
