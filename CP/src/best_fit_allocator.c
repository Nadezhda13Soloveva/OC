#include "best_fit_allocator.h"
#include <string.h>

struct BlockHeader {
    size_t size;
    struct BlockHeader* next;
    uint8_t is_free;
};

typedef struct {
    Allocator base;           // Must be first member
    struct BlockHeader* free_list;
    void* memory_start;
    size_t total_size;
} BestFitAllocator;

static void* best_fit_allocator_alloc(Allocator* allocator, size_t size) {
    BestFitAllocator* bf = (BestFitAllocator*)allocator;
    struct BlockHeader *best = NULL, *current = bf->free_list;
    size_t min_diff = SIZE_MAX;

    // Find best fitting block
    while (current != NULL) {
        if (current->is_free && current->size >= size) {
            size_t diff = current->size - size;
            if (diff < min_diff) {
                min_diff = diff;
                best = current;
            }
        }
        current = current->next;
    }

    if (best == NULL) return NULL;

    // Split block if possible
    if (min_diff > sizeof(struct BlockHeader)) {
        struct BlockHeader* new_block = (struct BlockHeader*)((char*)best + size + sizeof(struct BlockHeader));
        new_block->size = best->size - size - sizeof(struct BlockHeader);
        new_block->next = best->next;
        new_block->is_free = 1;
        
        best->size = size;
        best->next = new_block;
    }

    best->is_free = 0;
    return (void*)((char*)best + sizeof(struct BlockHeader));
}

static void best_fit_allocator_free(Allocator* allocator, void* ptr) {
    if (!ptr) return;

    struct BlockHeader* header = (struct BlockHeader*)((char*)ptr - sizeof(struct BlockHeader));
    header->is_free = 1;

    // Merge with next block if free
    if (header->next && header->next->is_free) {
        header->size += header->next->size + sizeof(struct BlockHeader);
        header->next = header->next->next;
    }
    
    // We should also try to merge with the previous block if it's free
    BestFitAllocator* bf = (BestFitAllocator*)allocator;
    struct BlockHeader* current = bf->free_list;
    while (current && current->next != header) {
        current = current->next;
    }
    if (current && current->is_free) {
        current->size += header->size + sizeof(struct BlockHeader);
        current->next = header->next;
    }
}

static AllocatorStats best_fit_allocator_stats(Allocator* allocator) {
    BestFitAllocator* bf = (BestFitAllocator*)allocator;
    AllocatorStats stats = {0};
    stats.total_memory = bf->total_size;
    
    // Calculate used and free memory
    struct BlockHeader* current = bf->free_list;
    while (current != NULL) {
        if (current->is_free) {
            stats.free_memory += current->size;
        }
        current = current->next;
    }
    
    stats.used_memory = stats.total_memory - stats.free_memory;
    stats.utilization = (double)stats.used_memory / stats.total_memory;
    
    return stats;
}

// Static vtable for best fit allocator
static AllocatorVTable best_fit_vtable = {
    .alloc = best_fit_allocator_alloc,
    .free = best_fit_allocator_free,
    .getStats = best_fit_allocator_stats
};

Allocator* createBestFitAllocator(void* memory, size_t size) {
    BestFitAllocator* allocator = (BestFitAllocator*)memory;
    allocator->base.vtable = &best_fit_vtable;
    allocator->memory_start = (char*)memory + sizeof(BestFitAllocator);
    allocator->total_size = size - sizeof(BestFitAllocator);

    // Initialize first block
    struct BlockHeader* first_block = (struct BlockHeader*)allocator->memory_start;
    first_block->size = allocator->total_size - sizeof(struct BlockHeader);
    first_block->next = NULL;
    first_block->is_free = 1;
    allocator->free_list = first_block;

    return (Allocator*)allocator;
} 