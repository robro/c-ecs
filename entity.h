#ifndef ENTITY_H
#define ENTITY_H

#include <sys/types.h>
#include <stdbool.h>
#include "component.h"

struct Entities {
	bool *alive;
	uint size;
	uint last_free_index;
	bool initialized;
};

extern struct Entities entities;

bool entity_initialize_entities(uint size);

int entity_get_free_index();

void entity_set_alive(uint index);

void entity_set_dead(uint index);

bool entity_is_alive(uint index);

bool entity_add(const struct Component **components);

#endif
