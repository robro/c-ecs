#include <stdio.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>

#define MAX_ENTITIES 1000
#define TARGET_FPS 60

#define USECS_IN_SEC 1000000
#define NSECS_IN_USEC 1000
#define TARGET_USECS (USECS_IN_SEC / TARGET_FPS)

#define ARRAY_LENGTH(arr) sizeof(arr)/sizeof(arr[0])

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

void UpdateJumpers(double Delta) {
	for (int i = 0; i < TotalEntities; ++i) {
		if (PhysicsComponents[i].Position.y >= JumperComponents[i].GroundHeight) {
			PhysicsComponents[i].Position.y = JumperComponents[i].GroundHeight;
			PhysicsComponents[i].Velocity.y = -JumperComponents[i].JumpForce;
		}
	}
}

void UpdateShakers(double Delta) {
	for (int i = 0; i < TotalEntities; ++i) {
		if (PhysicsComponents[i].Velocity.x >= 0) {
			PhysicsComponents[i].Velocity.x = -ShakerComponents[i].ShakeSpeed;
		} else {
			PhysicsComponents[i].Velocity.x = ShakerComponents[i].ShakeSpeed;
		}
	}
}

void UpdatePhysics(double Delta) {
	for (int i = 0; i < TotalEntities; ++i) {
		// Add gravity
		PhysicsComponents[i].Velocity.x += PhysicsComponents[i].Gravity.x * Delta;
		PhysicsComponents[i].Velocity.y += PhysicsComponents[i].Gravity.y * Delta;

		// Update position
		PhysicsComponents[i].Position.x += PhysicsComponents[i].Velocity.x * Delta;
		PhysicsComponents[i].Position.y += PhysicsComponents[i].Velocity.y * Delta;

		// printf("Pos(%f, %f)\n", PhysicsComponents[i].Position.x, PhysicsComponents[i].Position.y);
	}
}

typedef void (*update_func)(double);
update_func UpdateFuncs[] = {
	UpdateJumpers,
	UpdateShakers,
	UpdatePhysics,
};
size_t UpdateFuncsCount = ARRAY_LENGTH(UpdateFuncs);

long GetDeltaUSecs(struct timeval TimeStart, struct timeval TimeEnd) {
	return (TimeEnd.tv_sec - TimeStart.tv_sec) * USECS_IN_SEC + (TimeEnd.tv_usec - TimeStart.tv_usec);
}

int main() {
	for (int i = 0; i < MAX_ENTITIES; ++i) {
		NewJumper(VEC_ZERO, 100);
	}

	struct timeval TimeStart, TimeEnd;
	struct timespec Sleep;
	double TargetDelta = 1.0 / TARGET_FPS;
	double FrameDelta = TargetDelta;
	long FrameUSecs;

	printf("Starting test...\n");
	gettimeofday(&TimeStart, NULL);

	// Gameloop
	for (int i = 0; i < 10; ++i) {
		for (int j = 0; j < UpdateFuncsCount; ++j) {
			UpdateFuncs[j](FrameDelta);
		}
		gettimeofday(&TimeEnd, NULL);
		FrameUSecs = GetDeltaUSecs(TimeStart, TimeEnd);
		if (FrameUSecs < TARGET_USECS) {
			Sleep.tv_sec = (TARGET_USECS - FrameUSecs) / USECS_IN_SEC;
			Sleep.tv_nsec = ((TARGET_USECS - FrameUSecs) % USECS_IN_SEC) * NSECS_IN_USEC;
			nanosleep(&Sleep, NULL);
		}
		gettimeofday(&TimeEnd, NULL);
		FrameDelta = (double)GetDeltaUSecs(TimeStart, TimeEnd) / USECS_IN_SEC;
		printf("Frame time:  %lf secs | ", (double)FrameUSecs / USECS_IN_SEC);
		printf("Frame delta: %lf secs\n", FrameDelta);
		TimeStart = TimeEnd;
	}

	return 0;
}
