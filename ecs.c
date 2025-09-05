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

typedef struct PhysicsComponent physics_component;
typedef void (*physics_func) (physics_component*);

struct PhysicsComponent {
	vec2 Position;
	vec2 Velocity;
	vec2 Gravity;
	physics_func Update;
};

physics_component PhysicsComponents[MAX_ENTITIES];
uint PhysicsComponentIndex = 0;
uint TotalPhysicsComponents = 0;

void UpdatePosition(physics_component *PhysicsComponent) {
	PhysicsComponent->Position.x += PhysicsComponent->Velocity.x;
	PhysicsComponent->Position.y += PhysicsComponent->Velocity.y;
}

void AddGravity(physics_component *PhysicsComponent) {
	PhysicsComponent->Velocity.x += PhysicsComponent->Gravity.x;
	PhysicsComponent->Velocity.y += PhysicsComponent->Gravity.y;
}

/* ==== ENTITIES ==================================== */

typedef struct {
	uint ComponentMask;
} entity;

void JumperPhysicsUpdate(physics_component *PhysicsComponent) {
	if (PhysicsComponent->Position.y == 0) {
		PhysicsComponent->Velocity.y = -100;
	}
	AddGravity(PhysicsComponent);
	UpdatePosition(PhysicsComponent);
	if (PhysicsComponent->Position.y >= 0) {
		PhysicsComponent->Position.y = 0;
		PhysicsComponent->Velocity.y = 0;
	}
}

int NewJumper(vec2 Position) {
	PhysicsComponents[PhysicsComponentIndex] = (physics_component){
		.Position = Position,
		.Velocity = VELOCITY_ZERO,
		.Gravity = GRAVITY,
		.Update = JumperPhysicsUpdate,
	};
	TotalPhysicsComponents++;
	PhysicsComponentIndex++;
	TotalEntities++;
	return EntityIndex++;
}

void ShakerPhysicsUpdate(physics_component *PhysicsComponent) {
	if (PhysicsComponent->Velocity.x > 0) {
		PhysicsComponent->Velocity.x = -10;
	}
	else {
		PhysicsComponent->Velocity.x = 10;
	}
	UpdatePosition(PhysicsComponent);
}

int NewShaker(vec2 Position) {
	PhysicsComponents[PhysicsComponentIndex] = (physics_component){
		.Position = Position,
		.Velocity = VELOCITY_ZERO,
		.Gravity = VELOCITY_ZERO,
		.Update = ShakerPhysicsUpdate,
	};
	TotalPhysicsComponents++;
	PhysicsComponentIndex++;
	TotalEntities++;
	return EntityIndex++;
}

int main() {
	int jumper_id = NewJumper((vec2){4, 5});
	int shaker_id = NewShaker((vec2){-3, 9});

	while (1) {
		for (int i = 0; i < TotalPhysicsComponents; ++i) {
			PhysicsComponents[i].Update(&PhysicsComponents[i]);
			printf("Position(%f, %f)\n", PhysicsComponents[i].Position.x, PhysicsComponents[i].Position.y);
		}
		sleep(1);
	}
	return 0;
}
