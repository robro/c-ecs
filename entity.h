#ifndef _ENTITY_H
#define _ENTITY_H

#include <sys/types.h>
#include <stdbool.h>
#include "component.h"

bool entity_initialize_entities(uint size);

int entity_get_free_index();

void entity_set_alive(uint index);

void entity_set_dead(uint index);

bool entity_is_alive(uint index);

/* Takes NULL terminated array of Component pointers.
 * Returns entity index or -1 if no free indices. */
int entity_create(const struct Component *components[]);

#endif
