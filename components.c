#include "components.h"

void component_array_insert_physics(const struct Component *component, uint index) {
	((struct ComponentPhysics *)component)->array[index] = ((struct ComponentPhysics *)component)->data;
}

void component_array_insert_jumper(const struct Component *component, uint index) {
	((struct ComponentJumper *)component)->array[index] = ((struct ComponentJumper *)component)->data;
}

void component_array_insert_shaker(const struct Component *component, uint index) {
	((struct ComponentShaker *)component)->array[index] = ((struct ComponentShaker *)component)->data;
}

void component_array_insert_lifetime(const struct Component *component, uint index) {
	((struct ComponentLifetime *)component)->array[index] = ((struct ComponentLifetime *)component)->data;
}

struct Component* component_get_physics(const struct ComponentDataPhysics *data, struct ComponentDataPhysics array[]) {
	static struct ComponentInterface vtable = {
		.array_insert = component_array_insert_physics
	};
	struct ComponentPhysics *c_physics = malloc(sizeof(struct ComponentPhysics));
	if (c_physics == NULL) {
		return NULL;
	}
	c_physics->base.vtable = &vtable;
	c_physics->data = *data;
	c_physics->array = array;
	return (struct Component *)c_physics;
}

struct Component* component_get_jumper(const struct ComponentDataJumper *data, struct ComponentDataJumper array[]) {
	static struct ComponentInterface vtable = {
		.array_insert = component_array_insert_jumper
	};
	struct ComponentJumper *c_jumper = malloc(sizeof(struct ComponentJumper));
	if (c_jumper == NULL) {
		return NULL;
	}
	c_jumper->base.vtable = &vtable;
	c_jumper->data = *data;
	c_jumper->array = array;
	return (struct Component *)c_jumper;
}

struct Component* component_get_shaker(const struct ComponentDataShaker *data, struct ComponentDataShaker array[]) {
	static struct ComponentInterface vtable = {
		.array_insert = component_array_insert_shaker
	};
	struct ComponentShaker *c_shaker = malloc(sizeof(struct ComponentShaker));
	if (c_shaker == NULL) {
		return NULL;
	}
	c_shaker->base.vtable = &vtable;
	c_shaker->data = *data;
	c_shaker->array = array;
	return (struct Component *)c_shaker;
}

struct Component* component_get_lifetime(const struct ComponentDataLifetime *data, struct ComponentDataLifetime array[]) {
	static struct ComponentInterface vtable = {
		.array_insert = component_array_insert_lifetime
	};
	struct ComponentLifetime *c_lifetime = malloc(sizeof(struct ComponentLifetime));
	if (c_lifetime == NULL) {
		return NULL;
	}
	c_lifetime->base.vtable = &vtable;
	c_lifetime->data = *data;
	c_lifetime->array = array;
	return (struct Component *)c_lifetime;
}
