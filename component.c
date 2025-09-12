#include "component.h"

void component_array_insert(const struct Component *component, uint index) {
	component->vtable->array_insert(component, index);
}
