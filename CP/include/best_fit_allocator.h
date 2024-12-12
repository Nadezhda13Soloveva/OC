#ifndef BEST_FIT_ALLOCATOR_H
#define BEST_FIT_ALLOCATOR_H

#include "allocator.h"

// Create best fit allocator
Allocator* createBestFitAllocator(void* memory, size_t size);

#endif 