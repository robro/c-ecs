#include <stdlib.h>
#include "entity.h"

struct Entities entities;

bool entity_initialize_entities(uint size) {
	if (entities.initialized || size == 0) {
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
