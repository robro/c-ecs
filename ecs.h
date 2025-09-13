#ifndef ECS_H
#define ECS_H

#include <sys/types.h>
#include <stdbool.h>

#include "util.h"

struct ComponentPhysics {
	struct Vec2 position;
	struct Vec2 velocity;
	struct Vec2 gravity;
	bool active;
};

struct ComponentJumper {
	float jump_force;
	float ground_height;
	bool active;
};

struct ComponentShaker {
	float shake_speed;
	bool active;
};

struct ComponentLifetime {
	float lifetime;
	bool active;
};

struct Component {
	enum {
		CT_NONE,
		CT_PHYSICS,
		CT_JUMPER,
		CT_SHAKER,
		CT_LIFETIME,
	} type;
	union {
		struct ComponentPhysics physics;
		struct ComponentJumper jumper;
		struct ComponentShaker shaker;
		struct ComponentLifetime lifetime;
	};
};

bool component_allocate_components(uint size);

void component_add_physics(const struct ComponentPhysics *data, uint index);

void component_add_jumper(const struct ComponentJumper *data, uint index);

void component_add_shaker(const struct ComponentShaker *data, uint index);

void component_add_lifetime(const struct ComponentLifetime *data, uint index);

bool entity_initialize_entities(uint size);

int entity_get_free_index();

void entity_set_alive(uint index);

void entity_set_dead(uint index);

bool entity_is_alive(uint index);

bool entity_add(const struct Component **components);

typedef void (*UpdateFunc)(float);

extern const UpdateFunc update_funcs[];

#endif
