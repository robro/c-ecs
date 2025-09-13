#ifndef COMPONENT_H
#define COMPONENT_H

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

void component_set_physics_array(struct ComponentPhysics *array);
void component_set_jumper_array(struct ComponentJumper *array);
void component_set_shaker_array(struct ComponentShaker *array);
void component_set_lifetime_array(struct ComponentLifetime *array);

void component_add_physics(const struct ComponentPhysics *data, uint index);
void component_add_jumper(const struct ComponentJumper *data, uint index);
void component_add_shaker(const struct ComponentShaker *data, uint index);
void component_add_lifetime(const struct ComponentLifetime *data, uint index);

#endif
