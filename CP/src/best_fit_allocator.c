#include "best_fit_allocator.h"
#include <string.h>

// структурка заголовка блока памяти
struct BlockHeader {
    size_t size; // размер блока
    struct BlockHeader* next; // следующий
    uint8_t is_free; // флаг свободен ли 
};

// структурка самого аллокатора
typedef struct {
    Allocator base; // базовая структурка с таблицей
    struct BlockHeader* free_list; // указатель на список своб блоков
    void* memory_start; // начало управляемой памяти
    size_t total_size; // общий размер доступной памяти
} BestFitAllocator;

// выделение памяти
static void* best_fit_allocator_alloc(Allocator* allocator, size_t size) {
    BestFitAllocator* bf = (BestFitAllocator*)allocator;
    struct BlockHeader *best = NULL, *current = bf->free_list;
    size_t min_diff = SIZE_MAX;

    // проходимся по каждому свободному и ищем с минимальной разницей в размере
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

    // не найден
    if (best == NULL) return NULL;

    // если разница размеров больше заголовка блоков -> попытка разделить 
    if (min_diff > sizeof(struct BlockHeader)) {
        // сразу после выделенного пространства создаем новый блок 
        struct BlockHeader* new_block = (struct BlockHeader*)((char*)best + size + sizeof(struct BlockHeader));
        new_block->size = best->size - size - sizeof(struct BlockHeader);
        new_block->next = best->next;
        new_block->is_free = 1;
        
        best->size = size;
        best->next = new_block;
    }

    best->is_free = 0; // блок занят
    // указатель на выделенную память после заголовка
    return (void*)((char*)best + sizeof(struct BlockHeader));
}


// освобождение памяти
static void best_fit_allocator_free(Allocator* allocator, void* ptr) {
    if (!ptr) return; // если освобождать нечего
    // считаем указатель на заголовок
    struct BlockHeader* header = (struct BlockHeader*)((char*)ptr - sizeof(struct BlockHeader));
    header->is_free = 1;

    // если есть пустой следующий, то соединяем с ним
    if (header->next && header->next->is_free) {
        header->size += header->next->size + sizeof(struct BlockHeader);
        header->next = header->next->next;
    }
    
    // находим пустой блок перед текущим и если он есть, то соединяем с ним
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


// рассчет статистики по аллокатору
static AllocatorStats best_fit_allocator_stats(Allocator* allocator) {
    BestFitAllocator* bf = (BestFitAllocator*)allocator;
    AllocatorStats stats = {0};
    stats.total_memory = bf->total_size;
    
    // считаем использованную и свободную память
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

// статическая таблица вирт функций
static AllocatorVTable best_fit_vtable = {
    .alloc = best_fit_allocator_alloc,
    .free = best_fit_allocator_free,
    .getStats = best_fit_allocator_stats
};

// создание аллокатора
Allocator* createBestFitAllocator(void* memory, size_t size) {
    // преобразуем указатель на блок памяти, инит таблицы
    BestFitAllocator* allocator = (BestFitAllocator*)memory;
    allocator->base.vtable = &best_fit_vtable;
    // установка начала управляемой памяти и размера
    allocator->memory_start = (char*)memory + sizeof(BestFitAllocator);
    allocator->total_size = size - sizeof(BestFitAllocator);

    // инит первого блока в начале управлемой памяти
    struct BlockHeader* first_block = (struct BlockHeader*)allocator->memory_start;
    first_block->size = allocator->total_size - sizeof(struct BlockHeader);
    first_block->next = NULL;
    first_block->is_free = 1;
    allocator->free_list = first_block;

    return (Allocator*)allocator;
} 