#include <stdlib.h>
#include <stdbool.h>
#include "entity.h"

struct Entities {
	uint size;
	int last_free_index;
	bool *alive;
	bool initialized;
};

struct Entities entities;

void entity_initialize_entities(uint size) {
	if (entities.initialized) {
		return;
	}
	entities.size = size;
	entities.last_free_index = -1;
	entities.alive = calloc(size, sizeof(bool));
	entities.initialized = true;
}

int entity_get_free_index() {
	if (!entities.initialized) {
		return -1;
	}
	uint start_index = (entities.last_free_index < 0) ? 0 : entities.last_free_index;
	uint i = start_index;
	while (entities.alive[i]) {
		i++;
		i %= entities.size;
		if (i == start_index) {
			return -1;
		}
	}
	entities.last_free_index = i;
	return i;
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
	if (!entities.initialized) {
		return false;
	}
	if (index >= entities.size) {
		return false;
	}
	return entities.alive[index];
}

int entity_create(const struct Component *components[]) {
	int entity_index = entity_get_free_index();
	if (entity_index < 0) {
		return -1;
	}
	for (int i = 0; components[i]; ++i) {
		component_array_insert(components[i], entity_index);
	}
	entities.alive[entity_index] = true;
	return entity_index;
}
