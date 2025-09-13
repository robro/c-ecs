#ifndef SYSTEM_H
#define SYSTEM_H

#include "entity.h"

typedef void (*UpdateFunc)(float);

extern const UpdateFunc update_funcs[];

#endif
