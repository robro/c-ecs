#ifndef ENTITY_H
#define ENTITY_H

#include <sys/types.h>
#include <stdbool.h>

bool entity_initialize_entities(uint size);

int entity_get_free_index();

void entity_set_alive(uint index);

void entity_set_dead(uint index);

bool entity_is_alive(uint index);

#endif
