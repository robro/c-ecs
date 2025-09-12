#include <stdlib.h>
#include "entity.h"

struct Entities {
	bool *alive_array;
	uint size;
	uint last_free_index;
	bool initialized;
};

struct Entities entities;

bool entity_initialize_entities(uint size) {
	if (entities.initialized || size == 0) {
		return false;
	}
	entities.alive_array = calloc(size, sizeof(bool));
	if (entities.alive_array == NULL) {
		return false;
	}
	entities.size = size;
	entities.last_free_index = 0;
	entities.initialized = true;
	return true;
}

int entity_get_free_index() {
	if (!entities.initialized) {
		return -1;
	}
	uint i = entities.last_free_index;
	while (entities.alive_array[i]) {
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
		entities.alive_array[index] = true;
	}
}

void entity_set_dead(uint index) {
	if (!entities.initialized) {
		return;
	}
	if (index < entities.size) {
		entities.alive_array[index] = false;
	}
}

bool entity_is_alive(uint index) {
	if (!entities.initialized || index >= entities.size) {
		return false;
	}
	return entities.alive_array[index];
}
