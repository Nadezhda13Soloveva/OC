#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

// Forward declarations
typedef struct Allocator Allocator;
typedef struct AllocatorStats AllocatorStats;

// Function pointer types
typedef void* (*AllocFn)(Allocator*, size_t);
typedef void (*FreeFn)(Allocator*, void*);
typedef AllocatorStats (*StatsFn)(Allocator*);

// Vtable structure
typedef struct {
    AllocFn alloc;
    FreeFn free;
    StatsFn getStats;
} AllocatorVTable;

// Base allocator structure
struct Allocator {
    AllocatorVTable* vtable;
};

// Statistics structure
struct AllocatorStats {
    size_t total_memory;
    size_t used_memory;
    size_t free_memory;
    double utilization;
};

// Interface functions
static inline void* allocatorAlloc(Allocator* allocator, size_t size) {
    return allocator->vtable->alloc(allocator, size);
}

static inline void allocatorFree(Allocator* allocator, void* ptr) {
    allocator->vtable->free(allocator, ptr);
}

static inline AllocatorStats getStats(Allocator* allocator) {
    return allocator->vtable->getStats(allocator);
}

#endif 