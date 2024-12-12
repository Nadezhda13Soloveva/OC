#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "best_fit_allocator.h"
#include "power2_allocator.h"

#define MEMORY_SIZE (1024 * 1024)  // 1MB
#define TEST_ITERATIONS 1000

void run_tests(Allocator* allocator, const char* name) {
    void* blocks[TEST_ITERATIONS];
    size_t total_requested = 0;
    clock_t start, end;
    double alloc_time = 0, free_time = 0;

    // Test allocation
    start = clock();
    for (int i = 0; i < TEST_ITERATIONS; i++) {
        size_t size = rand() % 1024 + 1;  // Random size 1-1024 bytes
        total_requested += size;
        blocks[i] = allocatorAlloc(allocator, size);
        if (blocks[i] == NULL) {
            printf("Failed to allocate block of size %zu\n", size);
        }
    }
    end = clock();
    alloc_time = ((double)(end - start)) / CLOCKS_PER_SEC;

    // Get statistics after allocation
    AllocatorStats stats_after_alloc = getStats(allocator);

    // Test deallocation
    start = clock();
    for (int i = 0; i < TEST_ITERATIONS; i++) {
        allocatorFree(allocator, blocks[i]);
    }
    end = clock();
    free_time = ((double)(end - start)) / CLOCKS_PER_SEC;

    // Get statistics after deallocation
    AllocatorStats stats_after_free = getStats(allocator);

    printf("\nResults for %s:\n", name);
    printf("Allocation time: %f seconds\n", alloc_time);
    printf("Deallocation time: %f seconds\n", free_time);
    printf("Total memory: %zu bytes\n", stats_after_alloc.total_memory);
    printf("Total requested: %zu bytes\n", total_requested);
    printf("Used memory after alloc: %zu bytes\n", stats_after_alloc.used_memory);
    printf("Free memory after alloc: %zu bytes\n", stats_after_alloc.free_memory);
    printf("Memory utilization: %.2f%%\n", stats_after_alloc.utilization * 100);
    printf("Used memory after free: %zu bytes\n", stats_after_free.used_memory);
    printf("Free memory after free: %zu bytes\n", stats_after_free.free_memory);
}

int main() {
    void* memory1 = malloc(MEMORY_SIZE);
    void* memory2 = malloc(MEMORY_SIZE);

    if (!memory1 || !memory2) {
        printf("Failed to allocate initial memory\n");
        return 1;
    }

    Allocator* best_fit = createBestFitAllocator(memory1, MEMORY_SIZE);
    Allocator* power2 = createPower2Allocator(memory2, MEMORY_SIZE);

    srand(time(NULL));

    run_tests(best_fit, "Best-fit Allocator");
    run_tests(power2, "Power-of-2 Allocator");

    free(memory1);
    free(memory2);

    return 0;
} 