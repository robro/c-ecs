#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/time.h>

#define ARRAY_LENGTH(arr) sizeof(arr)/sizeof(arr[0])
#define MAX_ENTITIES 1000
#define TARGET_FPS 60

uint EntityIndex = 0;
uint TotalEntities = 0;

typedef struct {
	float x;
	float y;
} vec2;

#define VEC_ZERO (vec2){0, 0}
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
		.Velocity = VEC_ZERO,
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
		.Velocity = VEC_ZERO,
		.Gravity = VEC_ZERO,
	};
	TotalEntities++;
	return EntityIndex++;
}

/* ==== SYSTEMS ==================================== */

void UpdateJumpers(long Delta) {
	for (int i = 0; i < TotalEntities; ++i) {
		if (PhysicsComponents[i].Position.y >= JumperComponents[i].GroundHeight) {
			PhysicsComponents[i].Position.y = JumperComponents[i].GroundHeight;
			PhysicsComponents[i].Velocity.y = -JumperComponents[i].JumpForce;
		}
	}
}

void UpdateShakers(long Delta) {
	for (int i = 0; i < TotalEntities; ++i) {
		if (PhysicsComponents[i].Velocity.x >= 0) {
			PhysicsComponents[i].Velocity.x = -ShakerComponents[i].ShakeSpeed;
		} else {
			PhysicsComponents[i].Velocity.x = ShakerComponents[i].ShakeSpeed;
		}
	}
}

void UpdatePhysics(long Delta) {
	for (int i = 0; i < TotalEntities; ++i) {
		// Add gravity
		PhysicsComponents[i].Velocity.x += PhysicsComponents[i].Gravity.x;
		PhysicsComponents[i].Velocity.y += PhysicsComponents[i].Gravity.y;

		// Update position
		PhysicsComponents[i].Position.x += PhysicsComponents[i].Velocity.x;
		PhysicsComponents[i].Position.y += PhysicsComponents[i].Velocity.y;

		// printf("Pos(%f, %f)\n", PhysicsComponents[i].Position.x, PhysicsComponents[i].Position.y);
	}
}

typedef void (*update_func) (long);
update_func UpdateFuncs[] = {
	UpdateJumpers,
	UpdateShakers,
	UpdatePhysics,
};
size_t UpdateFuncsCount = ARRAY_LENGTH(UpdateFuncs);

int main() {
	for (int i = 0; i < MAX_ENTITIES; ++i) {
		NewJumper(VEC_ZERO, 100);
	}

	printf("Starting test...\n");
	double FrameDelta = 0;
	double TargetDelta = 1.0 / TARGET_FPS;
	struct timeval TimeStart, TimeEnd;
	gettimeofday(&TimeStart, NULL);

	// Gameloop
	for (int i = 0; i < 10; ++i) {
		for (int j = 0; j < UpdateFuncsCount; ++j) {
			UpdateFuncs[j](FrameDelta);
		}
		gettimeofday(&TimeEnd, NULL);
		FrameDelta = (double)((TimeEnd.tv_sec-TimeStart.tv_sec)*1000000 + (TimeEnd.tv_usec-TimeStart.tv_usec))/1000000;
		TimeStart = TimeEnd;
		printf("Frame delta:  %lf secs | Target delta: %lf secs\n", FrameDelta, TargetDelta);
	}

	return 0;
}
