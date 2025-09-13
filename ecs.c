#include <string.h>
#include <stdlib.h>

#include "ecs.h"

struct ComponentPhysics *components_physics;
struct ComponentJumper *components_jumpers;
struct ComponentShaker *components_shakers;
struct ComponentLifetime *components_lifetimes;

void component_free_components() {
	free(components_physics);
	free(components_jumpers);
	free(components_shakers);
	free(components_lifetimes);
	components_physics = NULL;
	components_jumpers = NULL;
	components_shakers = NULL;
	components_lifetimes = NULL;
}

bool component_allocate_components(uint size) {
	components_physics = calloc(size, sizeof(struct ComponentPhysics));
	if (!components_physics) {
		component_free_components();
		return false;
	}
	components_jumpers = calloc(size, sizeof(struct ComponentJumper));
	if (!components_jumpers) {
		component_free_components();
		return false;
	}
	components_shakers = calloc(size, sizeof(struct ComponentShaker));
	if (!components_shakers) {
		component_free_components();
		return false;
	}
	components_lifetimes = calloc(size, sizeof(struct ComponentLifetime));
	if (!components_lifetimes) {
		component_free_components();
		return false;
	}
	return true;
}

void component_add_physics(const struct ComponentPhysics *data, uint index) {
	if (components_physics) {
		components_physics[index] = *data;
	}
}

void component_add_jumper(const struct ComponentJumper *data, uint index) {
	if (components_jumpers) {
		components_jumpers[index] = *data;
	}
}

void component_add_shaker(const struct ComponentShaker *data, uint index) {
	if (components_shakers) {
		components_shakers[index] = *data;
	}
}

void component_add_lifetime(const struct ComponentLifetime *data, uint index) {
	if (components_lifetimes) {
		components_lifetimes[index] = *data;
	}
}

struct Entities {
	bool *alive;
	uint size;
	uint last_free_index;
	bool initialized;
};

struct Entities entities;

bool entity_initialize_entities(uint size) {
	if (entities.initialized) {
		return false;
	}
	entities.alive = calloc(size, sizeof(bool));
	if (entities.alive == NULL) {
		return false;
	}
	entities.size = size;
	entities.last_free_index = 0;
	entities.initialized = true;
	return true;
}

void entity_free_entities() {
	free(entities.alive);
	memset(&entities, 0, sizeof(struct Entities));
}

int entity_get_free_index() {
	if (!entities.initialized) {
		return -1;
	}
	uint i = entities.last_free_index;
	while (entities.alive[i]) {
		i++;
		i %= entities.size;
		if (i == entities.last_free_index) {
			return -1;
		}
	}
	entities.last_free_index = i;
	return entities.last_free_index;
}

void entity_set_alive(uint index) {
	if (!entities.initialized) {
		return;
	}
	if (index < entities.size) {
		entities.alive[index] = true;
	}
}

void entity_set_dead(uint index) {
	if (!entities.initialized) {
		return;
	}
	if (index < entities.size) {
		entities.alive[index] = false;
	}
}

bool entity_is_alive(uint index) {
	if (!entities.initialized || index >= entities.size) {
		return false;
	}
	return entities.alive[index];
}

bool entity_add(const struct Component **components) {
	int entity_index = entity_get_free_index();
	if (entity_index < 0) {
		return false;
	}
	for (int i = 0; components[i]; ++i) {
		switch (components[i]->type) {
		case CT_NONE:
			break;
		case CT_PHYSICS:
			component_add_physics(&components[i]->physics, entity_index);
			break;
		case CT_JUMPER:
			component_add_jumper(&components[i]->jumper, entity_index);
			break;
		case CT_SHAKER:
			component_add_shaker(&components[i]->shaker, entity_index);
			break;
		case CT_LIFETIME:
			component_add_lifetime(&components[i]->lifetime, entity_index);
			break;
		}
	}
	entity_set_alive(entity_index);
	return true;
}

void update_jumpers(float delta) {
	for (int i = 0; i < entities.size; ++i) {
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
	for (int i = 0; i < entities.size; ++i) {
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
	for (int i = 0; i < entities.size; ++i) {
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
	for (int i = 0; i < entities.size; ++i) {
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
	if (!entity_initialize_entities(size)) {
		return false;
	}
	if (!component_allocate_components(size)) {
		entity_free_entities();
		return false;
	}
	return true;
}
