#include "power2_allocator.h"
#include <string.h>

struct BlockHeader {
    size_t size;
    struct BlockHeader* next;
    uint8_t is_free;
};

typedef struct {
    Allocator base;
    struct BlockHeader* free_lists[MAX_POWER];
    void* memory_start;
    size_t total_size;
    size_t allocated_size;
} Power2Allocator;

static int get_power2_ceil(size_t size) {
    int power = 0;
    size_t n = 1;
    while (n < size) {
        n <<= 1;
        power++;
    }
    return power;
}

static void* power2_allocator_alloc(Allocator* allocator, size_t size) {
    Power2Allocator* p2 = (Power2Allocator*)allocator;
    int power = get_power2_ceil(size + sizeof(struct BlockHeader));
    
    // Find first non-empty list with blocks >= requested size
    while (power < MAX_POWER && p2->free_lists[power] == NULL) {
        power++;
    }

    if (power >= MAX_POWER) return NULL;

    // Remove block from free list
    struct BlockHeader* block = p2->free_lists[power];
    p2->free_lists[power] = block->next;

    // Split block if possible
    while (power > get_power2_ceil(size + sizeof(struct BlockHeader))) {
        power--;
        size_t half_size = 1ULL << power;
        struct BlockHeader* split = (struct BlockHeader*)((char*)block + half_size);
        split->size = half_size;
        split->next = p2->free_lists[power];
        split->is_free = 1;
        p2->free_lists[power] = split;
        block->size = half_size;
    }

    block->is_free = 0;
    p2->allocated_size += block->size;
    return (void*)((char*)block + sizeof(struct BlockHeader));
}

static void power2_allocator_free(Allocator* allocator, void* ptr) {
    if (!ptr) return;

    Power2Allocator* p2 = (Power2Allocator*)allocator;
    struct BlockHeader* header = (struct BlockHeader*)((char*)ptr - sizeof(struct BlockHeader));
    
    if (!header->is_free) {
        p2->allocated_size -= header->size;
        header->is_free = 1;
    }
    
    int power = get_power2_ceil(header->size);
    header->next = p2->free_lists[power];
    p2->free_lists[power] = header;
}

static AllocatorStats power2_allocator_stats(Allocator* allocator) {
    Power2Allocator* p2 = (Power2Allocator*)allocator;
    AllocatorStats stats = {0};
    
    stats.total_memory = p2->total_size;
    stats.used_memory = p2->allocated_size;
    stats.free_memory = stats.total_memory - stats.used_memory;
    
    // Ensure utilization is between 0 and 1
    if (stats.total_memory > 0) {
        stats.utilization = (double)stats.used_memory / (double)stats.total_memory;
    } else {
        stats.utilization = 0.0;
    }
    
    return stats;
}

static AllocatorVTable power2_vtable = {
    .alloc = power2_allocator_alloc,
    .free = power2_allocator_free,
    .getStats = power2_allocator_stats
};

Allocator* createPower2Allocator(void* memory, size_t size) {
    Power2Allocator* allocator = (Power2Allocator*)memory;
    allocator->base.vtable = &power2_vtable;
    allocator->memory_start = (char*)memory + sizeof(Power2Allocator);
    allocator->total_size = size - sizeof(Power2Allocator);
    allocator->allocated_size = 0;

    // Initialize free lists
    memset(allocator->free_lists, 0, sizeof(allocator->free_lists));

    // Initialize first block
    struct BlockHeader* first_block = (struct BlockHeader*)allocator->memory_start;
    first_block->size = allocator->total_size - sizeof(struct BlockHeader);
    first_block->is_free = 1;
    
    // Add to appropriate free list
    int power = get_power2_ceil(first_block->size);
    first_block->next = NULL;
    allocator->free_lists[power] = first_block;

    return (Allocator*)allocator;
} 