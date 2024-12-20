#ifndef POWER2_ALLOCATOR_H
#define POWER2_ALLOCATOR_H

#include "allocator.h"

#define MAX_POWER 32  // макс степень 2-ки (4GB)

Allocator* createPower2Allocator(void* memory, size_t size);

#endif 