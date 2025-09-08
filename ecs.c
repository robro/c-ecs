#include <bits/time.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdbool.h>

#define DEBUG_BUILD 1

#define MAX_ENTITIES 1000000
#define TARGET_FPS 60

#define SECS_PER_FRAME (1.0 / TARGET_FPS)
#define NSECS_IN_SEC 1000000000

#define ARRAY_LENGTH(arr) (sizeof(arr) / sizeof(arr[0]))

typedef struct timespec timespec;

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

typedef struct {
	enum {
		PHYSICS,
		JUMPER,
		SHAKER,
	} Type;
	union {
		physics_component Physics;
		jumper_component Jumper;
		shaker_component Shaker;
	};
} component;

physics_component PhysicsComponents[MAX_ENTITIES];
jumper_component JumperComponents[MAX_ENTITIES];
shaker_component ShakerComponents[MAX_ENTITIES];

bool ActivePhysicsComponents[MAX_ENTITIES];
bool ActiveJumperComponents[MAX_ENTITIES];
bool ActiveShakerComponents[MAX_ENTITIES];

/* ==== ENTITIES ==================================== */

int new_entity(component *Components[], size_t ComponentCount) {
	for (int i = 0; i < ComponentCount; ++i) {
		switch (Components[i]->Type) {
		case JUMPER:
			JumperComponents[EntityIndex] = Components[i]->Jumper;
			ActiveJumperComponents[EntityIndex] = true;
			break;
		case SHAKER:
			ShakerComponents[EntityIndex] = Components[i]->Shaker;
			ActiveShakerComponents[EntityIndex] = true;
			break;
		case PHYSICS:
			PhysicsComponents[EntityIndex] = Components[i]->Physics;
			ActivePhysicsComponents[EntityIndex] = true;
			break;
		}
	}
	TotalEntities++;
	return EntityIndex++;
}

/* ==== SYSTEMS ==================================== */

void update_jumpers(float Delta) {
	for (int i = 0; i < TotalEntities; ++i) {
		if (!ActiveJumperComponents[i] || !ActivePhysicsComponents[i]) {
			return;
		}
		if (PhysicsComponents[i].Position.y >= JumperComponents[i].GroundHeight) {
			PhysicsComponents[i].Position.y = JumperComponents[i].GroundHeight;
			PhysicsComponents[i].Velocity.y = -JumperComponents[i].JumpForce;
		}
	}
}

void update_shakers(float Delta) {
	for (int i = 0; i < TotalEntities; ++i) {
		if (!ActiveShakerComponents[i] || !ActivePhysicsComponents[i]) {
			return;
		}
		if (PhysicsComponents[i].Velocity.x >= 0) {
			PhysicsComponents[i].Velocity.x = -ShakerComponents[i].ShakeSpeed;
		} else {
			PhysicsComponents[i].Velocity.x = ShakerComponents[i].ShakeSpeed;
		}
	}
}

void update_physics(float Delta) {
	for (int i = 0; i < TotalEntities; ++i) {
		if (!ActivePhysicsComponents[i]) {
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

typedef void (*update_func)(float);
const update_func UpdateFuncs[] = {
	update_jumpers,
	update_shakers,
	update_physics,
};
const size_t UpdateFuncsCount = ARRAY_LENGTH(UpdateFuncs);

timespec diff_timespec(const timespec *TimeA, const timespec *TimeB) {
	timespec diff = {
		.tv_sec = TimeA->tv_sec - TimeB->tv_sec,
		.tv_nsec = TimeA->tv_nsec - TimeB->tv_nsec,
	};
	if (diff.tv_nsec < 0) {
		diff.tv_nsec += NSECS_IN_SEC;
		diff.tv_sec--;
	}
	return diff;
}

float timespec_to_secs(const timespec *Time) {
	return Time->tv_sec + (float)Time->tv_nsec / NSECS_IN_SEC;
}

int main() {
	component *Components[] = {
		&(component){
			.Type = SHAKER,
			.Shaker = {
				.ShakeSpeed = 100.00
			}
		},
		&(component){
			.Type = JUMPER,
			.Jumper = {
				.JumpForce = 100.0,
				.GroundHeight = 0.0
			}
		},
		&(component){
			.Type = PHYSICS,
			.Physics = {
				.Gravity = GRAVITY,
				.Position = VEC_ZERO,
				.Velocity = VEC_ZERO
			}
		}
	};

	for (int i = 0; i < MAX_ENTITIES; ++i) {
		new_entity(Components, ARRAY_LENGTH(Components));
	}

	timespec TimeStart, TimeEnd, SleepTime, WorkTime, FrameTime;
	const timespec TargetFrameTime = {.tv_sec = 0, .tv_nsec = NSECS_IN_SEC / TARGET_FPS};

	printf("Starting test...\n");

	// Game Loop
	for (int i = 0; i < 10; ++i) {
		clock_gettime(CLOCK_MONOTONIC, &TimeStart);
		for (int j = 0; j < UpdateFuncsCount; ++j) {
			UpdateFuncs[j](SECS_PER_FRAME);
		}
		clock_gettime(CLOCK_MONOTONIC, &TimeEnd);
		WorkTime = diff_timespec(&TimeEnd, &TimeStart);
		SleepTime = diff_timespec(&TargetFrameTime, &WorkTime);
		if (SleepTime.tv_sec >= 0 && SleepTime.tv_nsec > 0) {
			nanosleep(&SleepTime, NULL);
		}
#if DEBUG_BUILD
		clock_gettime(CLOCK_MONOTONIC, &TimeEnd);
		FrameTime = diff_timespec(&TimeEnd, &TimeStart);
		printf(
			"Work time: %lf secs | Frame time: %lf secs\n",
			timespec_to_secs(&WorkTime),
			timespec_to_secs(&FrameTime)
		);
#endif
	}

	return 0;
}
