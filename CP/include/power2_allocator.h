#ifndef POWER2_ALLOCATOR_H
#define POWER2_ALLOCATOR_H

#include "allocator.h"

#define MAX_POWER 32  // Maximum power of 2 (4GB)

// Create power of 2 allocator
Allocator* createPower2Allocator(void* memory, size_t size);

#endif 