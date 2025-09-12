#ifndef _COMPONENT_H
#define _COMPONENT_H

#include <stdlib.h>

struct Component {
	struct ComponentInterface *vtable;
};

struct ComponentInterface {
	void (*array_insert)(const struct Component *component, uint index);
};

void component_array_insert(const struct Component *component, uint index);

#endif
