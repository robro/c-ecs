#include "system.h"

void update_jumpers(float delta) {
	for (int i = 0; i < entities.size; ++i) {
		if (!entity_is_alive(i) || !components_jumpers[i].active) {
			return;
		}
		if (components_physics[i].position.y >= components_jumpers[i].ground_height) {
			components_physics[i].position.y = components_jumpers[i].ground_height;
			components_physics[i].velocity.y = -components_jumpers[i].jump_force;
		}
	}
}

void update_shakers(float delta) {
	for (int i = 0; i < entities.size; ++i) {
		if (!entity_is_alive(i) || !components_shakers[i].active) {
			return;
		}
		if (components_physics[i].velocity.x >= 0) {
			components_physics[i].velocity.x = -components_shakers[i].shake_speed;
		} else {
			components_physics[i].velocity.x = components_shakers[i].shake_speed;
		}
	}
}

void update_physics(float delta) {
	for (int i = 0; i < entities.size; ++i) {
		if (!entity_is_alive(i) || !components_physics[i].active) {
			return;
		}
		// Add gravity
		components_physics[i].velocity.x += components_physics[i].gravity.x * delta;
		components_physics[i].velocity.y += components_physics[i].gravity.y * delta;

		// Update position
		components_physics[i].position.x += components_physics[i].velocity.x * delta;
		components_physics[i].position.y += components_physics[i].velocity.y * delta;
	}
}

void update_lifetime(float delta) {
	for (int i = 0; i < entities.size; ++i) {
		if (!entity_is_alive(i) || !components_lifetimes[i].active) {
			return;
		}
		if (components_lifetimes[i].lifetime <= 0) {
			entity_set_dead(i);
			return;
		}
		components_lifetimes[i].lifetime -= delta;
	}
}

const UpdateFunc update_funcs[] = {
	update_jumpers,
	update_shakers,
	update_physics,
	update_lifetime,
	NULL
};
