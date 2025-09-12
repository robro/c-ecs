#ifndef _COMPONENTS_H
#define _COMPONENTS_H

#include <stdbool.h>
#include "util.h"
#include "component.h"

struct ComponentDataPhysics {
	struct Vec2 position;
	struct Vec2 velocity;
	struct Vec2 gravity;
	bool active;
};

struct ComponentDataJumper {
	float jump_force;
	float ground_height;
	bool active;
};

struct ComponentDataShaker {
	float shake_speed;
	bool active;
};

struct ComponentDataLifetime {
	float lifetime;
	bool active;
};

struct ComponentPhysics {
	struct Component base;
	struct ComponentDataPhysics data;
	struct ComponentDataPhysics *array;
};

struct ComponentJumper {
	struct Component base;
	struct ComponentDataJumper data;
	struct ComponentDataJumper *array;
};

struct ComponentShaker {
	struct Component base;
	struct ComponentDataShaker data;
	struct ComponentDataShaker *array;
};

struct ComponentLifetime {
	struct Component base;
	struct ComponentDataLifetime data;
	struct ComponentDataLifetime *array;
};

struct Component* component_get_physics(const struct ComponentDataPhysics *data, struct ComponentDataPhysics array[]);
struct Component* component_get_jumper(const struct ComponentDataJumper *data, struct ComponentDataJumper array[]);
struct Component* component_get_shaker(const struct ComponentDataShaker *data, struct ComponentDataShaker array[]);
struct Component* component_get_lifetime(const struct ComponentDataLifetime *data, struct ComponentDataLifetime array[]);

#endif
