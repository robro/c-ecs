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

bool ecs_initialize(uint size);

bool ecs_add_entity(const struct Component **components);

void ecs_update(float delta);

void ecs_free(void);

#endif
