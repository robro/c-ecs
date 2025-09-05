#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

#define ARRAY_LENGTH(arr) sizeof(arr)/sizeof(arr[0])
#define MAX_ENTITIES 10

uint EntityIndex = 0;
uint TotalEntities = 0;

typedef struct {
	float x;
	float y;
} vec2;

#define VELOCITY_ZERO (vec2){0, 0}
#define GRAVITY (vec2){0, 9.8}

/* ==== COMPONENTS ================================== */

typedef struct {
	vec2 Position;
	vec2 Velocity;
	vec2 Gravity;
} physics_component;

typedef struct {
	float JumpForce;
	float GroundHeight;
} jumper_component;

typedef struct {
	float ShakeSpeed;
} shaker_component;

physics_component PhysicsComponents[MAX_ENTITIES];
jumper_component JumperComponents[MAX_ENTITIES];
shaker_component ShakerComponents[MAX_ENTITIES];

/* ==== ENTITIES ==================================== */

int NewJumper(vec2 Position, float JumpForce) {
	JumperComponents[EntityIndex] = (jumper_component){
		.JumpForce = JumpForce,
		.GroundHeight = Position.y,
	};
	PhysicsComponents[EntityIndex] = (physics_component){
		.Position = Position,
		.Velocity = VELOCITY_ZERO,
		.Gravity = GRAVITY,
	};
	TotalEntities++;
	return EntityIndex++;
}

int NewShaker(vec2 Position, float ShakeSpeed) {
	ShakerComponents[EntityIndex] = (shaker_component){
		.ShakeSpeed = ShakeSpeed,
	};
	PhysicsComponents[EntityIndex] = (physics_component){
		.Position = Position,
		.Velocity = VELOCITY_ZERO,
		.Gravity = VELOCITY_ZERO,
	};
	TotalEntities++;
	return EntityIndex++;
}

/* ==== SYSTEMS ==================================== */

void UpdateJumpers() {
	for (int i = 0; i < TotalEntities; ++i) {
		if (PhysicsComponents[i].Position.y >= JumperComponents[i].GroundHeight) {
			PhysicsComponents[i].Position.y = JumperComponents[i].GroundHeight;
			PhysicsComponents[i].Velocity.y = -JumperComponents[i].JumpForce;
		}
	}
}

void UpdateShakers() {
	for (int i = 0; i < TotalEntities; ++i) {
		if (PhysicsComponents[i].Velocity.x >= 0) {
			PhysicsComponents[i].Velocity.x = -ShakerComponents[i].ShakeSpeed;
		} else {
			PhysicsComponents[i].Velocity.x = ShakerComponents[i].ShakeSpeed;
		}
	}
}

void UpdatePhysics() {
	for (int i = 0; i < TotalEntities; ++i) {
		// Add gravity
		PhysicsComponents[i].Velocity.x += PhysicsComponents[i].Gravity.x;
		PhysicsComponents[i].Velocity.y += PhysicsComponents[i].Gravity.y;

		// Update position
		PhysicsComponents[i].Position.x += PhysicsComponents[i].Velocity.x;
		PhysicsComponents[i].Position.y += PhysicsComponents[i].Velocity.y;

		printf("Pos(%f, %f)\n", PhysicsComponents[i].Position.x, PhysicsComponents[i].Position.y);
	}
}

int main() {
	int jumper_id = NewJumper((vec2){4, 5}, 100);
	int shaker_id = NewShaker((vec2){-3, 9}, 10);

	while (1) {
		UpdateJumpers();
		UpdateShakers();
		UpdatePhysics();
		sleep(1);
	}
	return 0;
}
