#include <stdlib.h>

#include "ecs.h"

uint ecs_size;
uint last_free_index;
bool initialized;
bool *entities_alive;

struct ComponentPhysics *components_physics;
struct ComponentJumper *components_jumpers;
struct ComponentShaker *components_shakers;
struct ComponentLifetime *components_lifetimes;

bool *temp_entities_alive;

struct ComponentPhysics *temp_components_physics;
struct ComponentJumper *temp_components_jumpers;
struct ComponentShaker *temp_components_shakers;
struct ComponentLifetime *temp_components_lifetimes;

void free_array(void **array) {
	for (int i = 0; array[i]; ++i) {
		if (!array[i]) {
			continue;
		}
		free(array[i]);
		array[i] = NULL;
	}
}

bool ecs_allocate(uint size) {
	void *temp_arrays[] = {
		temp_entities_alive,
		temp_components_physics,
		temp_components_jumpers,
		temp_components_shakers,
		temp_components_lifetimes,
	};
	temp_entities_alive = calloc(size, sizeof(*entities_alive));
	if (!temp_entities_alive) {
		return false;
	}
	temp_components_physics = calloc(size, sizeof(*temp_components_physics));
	if (!temp_components_physics) {
		free_array(temp_arrays);
		return false;
	}
	temp_components_jumpers = calloc(size, sizeof(*temp_components_jumpers));
	if (!temp_components_jumpers) {
		free_array(temp_arrays);
		return false;
	}
	temp_components_shakers = calloc(size, sizeof(*temp_components_shakers));
	if (!temp_components_shakers) {
		free_array(temp_arrays);
		return false;
	}
	temp_components_lifetimes = calloc(size, sizeof(*temp_components_lifetimes));
	if (!temp_components_lifetimes) {
		free_array(temp_arrays);
		return false;
	}
	ecs_free();
	entities_alive = temp_entities_alive;
	components_physics = temp_components_physics;
	components_jumpers = temp_components_jumpers;
	components_shakers = temp_components_shakers;
	components_lifetimes = temp_components_lifetimes;
	return true;
}

void free_entities() {
	free(entities_alive);
	entities_alive = NULL;
}

void free_components() {
	free(components_physics);
	free(components_jumpers);
	free(components_shakers);
	free(components_lifetimes);
	components_physics = NULL;
	components_jumpers = NULL;
	components_shakers = NULL;
	components_lifetimes = NULL;
}

int entity_get_free_index() {
	if (!initialized) {
		return -1;
	}
	uint i = last_free_index;
	while (entities_alive[i]) {
		i++;
		i %= ecs_size;
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
	if (index < ecs_size) {
		entities_alive[index] = true;
	}
}

void entity_set_dead(uint index) {
	if (!initialized) {
		return;
	}
	if (index < ecs_size) {
		entities_alive[index] = false;
	}
}

bool entity_is_alive(uint index) {
	if (!initialized || index >= ecs_size) {
		return false;
	}
	return entities_alive[index];
}

void update_jumpers(float delta) {
	for (int i = 0; i < ecs_size; ++i) {
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
	for (int i = 0; i < ecs_size; ++i) {
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
	for (int i = 0; i < ecs_size; ++i) {
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
	for (int i = 0; i < ecs_size; ++i) {
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
	ecs_size = size;
	last_free_index = 0;
	initialized = true;
	return true;
}

bool ecs_add_entity(const struct Component **components) {
	int entity_index = entity_get_free_index();
	if (entity_index < 0) {
		return false;
	}
	for (int i = 0; components[i]; ++i) {
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
	for (int i = 0; update_funcs[i]; ++i) {
		update_funcs[i](delta);
	}
}

void ecs_free(void) {
	if (!initialized) {
		return;
	}
	free_components();
	free_entities();
	ecs_size = 0;
	last_free_index = 0;
	initialized = false;
}
