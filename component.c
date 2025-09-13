#include "component.h"

struct ComponentPhysics *_components_physics = NULL;
struct ComponentJumper *_components_jumper = NULL;
struct ComponentShaker *_components_shaker = NULL;
struct ComponentLifetime *_components_lifetime = NULL;

void component_set_physics_array(struct ComponentPhysics *array) {
	_components_physics = array;
}

void component_set_jumper_array(struct ComponentJumper *array) {
	_components_jumper = array;
}

void component_set_shaker_array(struct ComponentShaker *array) {
	_components_shaker = array;
}

void component_set_lifetime_array(struct ComponentLifetime *array) {
	_components_lifetime = array;
}

void component_add_physics(const struct ComponentPhysics *data, uint index) {
	if (_components_physics) {
		_components_physics[index] = *data;
	}
}

void component_add_jumper(const struct ComponentJumper *data, uint index) {
	if (_components_jumper) {
		_components_jumper[index] = *data;
	}
}

void component_add_shaker(const struct ComponentShaker *data, uint index) {
	if (_components_shaker) {
		_components_shaker[index] = *data;
	}
}

void component_add_lifetime(const struct ComponentLifetime *data, uint index) {
	if (_components_lifetime) {
		_components_lifetime[index] = *data;
	}
}
