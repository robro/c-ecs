#include <stdlib.h>
#include "component.h"

struct ComponentPhysics *components_physics = NULL;
struct ComponentJumper *components_jumpers = NULL;
struct ComponentShaker *components_shakers = NULL;
struct ComponentLifetime *components_lifetimes = NULL;

bool component_initialize_components(uint size) {
	components_physics = calloc(size, sizeof(struct ComponentPhysics));
	if (!components_physics) {
		return false;
	}
	components_jumpers = calloc(size, sizeof(struct ComponentJumper));
	if (!components_jumpers) {
		return false;
	}
	components_shakers = calloc(size, sizeof(struct ComponentShaker));
	if (!components_shakers) {
		return false;
	}
	components_lifetimes = calloc(size, sizeof(struct ComponentLifetime));
	if (!components_lifetimes) {
		return false;
	}
	return true;
}

void component_add_physics(const struct ComponentPhysics *data, uint index) {
	if (components_physics) {
		components_physics[index] = *data;
	}
}

void component_add_jumper(const struct ComponentJumper *data, uint index) {
	if (components_jumpers) {
		components_jumpers[index] = *data;
	}
}

void component_add_shaker(const struct ComponentShaker *data, uint index) {
	if (components_shakers) {
		components_shakers[index] = *data;
	}
}

void component_add_lifetime(const struct ComponentLifetime *data, uint index) {
	if (components_lifetimes) {
		components_lifetimes[index] = *data;
	}
}
