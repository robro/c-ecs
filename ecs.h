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

void entity_set_alive(uint index);

void entity_set_dead(uint index);

bool entity_is_alive(uint index);

bool entity_add(const struct Component **components);

typedef void (*UpdateFunc)(float);

extern const UpdateFunc update_funcs[];

bool ecs_initialize(uint size);

#endif
