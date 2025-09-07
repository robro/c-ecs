#include <bits/time.h>
#include <stdio.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdbool.h>

#define DEBUG 1

#define MAX_ENTITIES 1000000
#define TARGET_FPS 60

#define SECS_PER_FRAME (1.0 / TARGET_FPS)
#define NSECS_IN_SEC 1000000000

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

bool InitializedPhysicsComponents[MAX_ENTITIES];
bool InitializedJumperComponents[MAX_ENTITIES];
bool InitializedShakerComponents[MAX_ENTITIES];

/* ==== ENTITIES ==================================== */

int new_jumper(vec2 Position, float JumpForce) {
	JumperComponents[EntityIndex] = (jumper_component){
		.JumpForce = JumpForce,
		.GroundHeight = Position.y,
	};
	PhysicsComponents[EntityIndex] = (physics_component){
		.Position = Position,
		.Velocity = VEC_ZERO,
		.Gravity = GRAVITY,
	};

	InitializedJumperComponents[EntityIndex] = true;
	InitializedPhysicsComponents[EntityIndex] = true;

	TotalEntities++;
	return EntityIndex++;
}

int new_shaker(vec2 Position, float ShakeSpeed) {
	ShakerComponents[EntityIndex] = (shaker_component){
		.ShakeSpeed = ShakeSpeed,
	};
	PhysicsComponents[EntityIndex] = (physics_component){
		.Position = Position,
		.Velocity = VEC_ZERO,
		.Gravity = VEC_ZERO,
	};

	InitializedShakerComponents[EntityIndex] = true;
	InitializedPhysicsComponents[EntityIndex] = true;

	TotalEntities++;
	return EntityIndex++;
}

/* ==== SYSTEMS ==================================== */

void update_jumpers(double Delta) {
	for (int i = 0; i < TotalEntities; ++i) {
		if (!InitializedJumperComponents[i]) {
			return;
		}
		if (PhysicsComponents[i].Position.y >= JumperComponents[i].GroundHeight) {
			PhysicsComponents[i].Position.y = JumperComponents[i].GroundHeight;
			PhysicsComponents[i].Velocity.y = -JumperComponents[i].JumpForce;
		}
	}
}

void update_shakers(double Delta) {
	for (int i = 0; i < TotalEntities; ++i) {
		if (!InitializedShakerComponents[i]) {
			return;
		}
		if (PhysicsComponents[i].Velocity.x >= 0) {
			PhysicsComponents[i].Velocity.x = -ShakerComponents[i].ShakeSpeed;
		} else {
			PhysicsComponents[i].Velocity.x = ShakerComponents[i].ShakeSpeed;
		}
	}
}

void update_physics(double Delta) {
	for (int i = 0; i < TotalEntities; ++i) {
		if (!InitializedPhysicsComponents[i]) {
			return;
		}
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
update_func update_funcs[] = {
	update_jumpers,
	update_shakers,
	update_physics,
};
size_t UpdateFuncsCount = ARRAY_LENGTH(update_funcs);

struct timespec diff_timespec(const struct timespec *A, const struct timespec *B) {
	struct timespec diff = {
		.tv_sec = A->tv_sec - B->tv_sec,
		.tv_nsec = A->tv_nsec - B->tv_nsec,
	};
	if (diff.tv_nsec < 0) {
		diff.tv_nsec += NSECS_IN_SEC;
		diff.tv_sec--;
	}
	return diff;
}

double timespec_to_secs(const struct timespec *Time) {
	return Time->tv_sec + (double)Time->tv_nsec / NSECS_IN_SEC;
}

int main() {
	for (int i = 0; i < MAX_ENTITIES; ++i) {
		new_jumper(VEC_ZERO, 100);
	}

	struct timespec TimeStart, TimeEnd, SleepTime, WorkTime, FrameTime;
	struct timespec TargetFrameTime = {.tv_sec = 0, .tv_nsec = NSECS_IN_SEC / TARGET_FPS};

	printf("Starting test...\n");

	// Game Loop
	for (int i = 0; i < 10; ++i) {
		clock_gettime(CLOCK_MONOTONIC, &TimeStart);
		for (int j = 0; j < UpdateFuncsCount; ++j) {
			update_funcs[j](SECS_PER_FRAME);
		}
		clock_gettime(CLOCK_MONOTONIC, &TimeEnd);
		WorkTime = diff_timespec(&TimeEnd, &TimeStart);
		SleepTime = diff_timespec(&TargetFrameTime, &WorkTime);
		if (SleepTime.tv_sec >= 0 && SleepTime.tv_nsec > 0) {
			nanosleep(&SleepTime, NULL);
		}
#if DEBUG
		clock_gettime(CLOCK_MONOTONIC, &TimeEnd);
		FrameTime = diff_timespec(&TimeEnd, &TimeStart);
		printf("Work time: %lf secs | ", timespec_to_secs(&WorkTime));
		printf("Frame time: %lf secs\n", timespec_to_secs(&FrameTime));
#endif
	}

	return 0;
}
