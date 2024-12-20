#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct Allocator Allocator;
typedef struct AllocatorStats AllocatorStats;

// типы указателей на функции
typedef void* (*AllocFn)(Allocator*, size_t);
typedef void (*FreeFn)(Allocator*, void*);
typedef AllocatorStats (*StatsFn)(Allocator*);

// структурка таблицы виртуальных функций
typedef struct {
    AllocFn alloc;
    FreeFn free;
    StatsFn getStats;
} AllocatorVTable;

// базовая структурка аллокатора
struct Allocator {
    AllocatorVTable* vtable;
};

// структурка для хранения статистики
struct AllocatorStats {
    size_t total_memory;
    size_t used_memory;
    size_t free_memory;
    double utilization; // коэф-т использования памяти
};

// интерфейсные функции для функций из AllocatorVTable
// static - каждая единица трансляции, включающая этот заголовочный файл, будет иметь свою собственную версию функции, что устраняет конфликты при линковке
// inline - функция будет встраиваться в точку вызова
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