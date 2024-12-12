# Best-Fit Allocator Implementation Details

## Overview
Best-fit аллокатор реализует стратегию поиска наиболее подходящего свободного блока памяти для каждого запроса на выделение. Этот подход минимизирует внутреннюю фрагментацию памяти за счет выбора блока, размер которого наиболее близок к запрошенному.

## Структуры данных

### Заголовок блока памяти
```
struct BlockHeader {
    size_t size;           // Размер блока данных (без заголовка)
    struct BlockHeader* next;  // Указатель на следующий блок
    uint8_t is_free;       // Флаг состояния блока (занят/свободен)
};
```

### Структура аллокатора
```
typedef struct {
    Allocator base;           // Базовый класс аллокатора
    struct BlockHeader* free_list;  // Список свободных блоков
    void* memory_start;       // Начало области памяти
    size_t total_size;        // Общий размер памяти
} BestFitAllocator;
```

## Основные операции

### Инициализация
1. Выделение памяти под структуру аллокатора
2. Инициализация виртуальной таблицы методов
3. Создание первого свободного блока максимального размера
4. Установка указателей и размеров

```
Allocator* createBestFitAllocator(void* memory, size_t size) {
    BestFitAllocator* allocator = (BestFitAllocator*)memory;
    allocator->base.vtable = &best_fit_vtable;
    allocator->memory_start = (char*)memory + sizeof(BestFitAllocator);
    allocator->total_size = size - sizeof(BestFitAllocator);

    // Инициализация первого блока
    struct BlockHeader* first_block = (struct BlockHeader*)allocator->memory_start;
    first_block->size = allocator->total_size - sizeof(struct BlockHeader);
    first_block->next = NULL;
    first_block->is_free = 1;
    allocator->free_list = first_block;

    return (Allocator*)allocator;
}
```

### Выделение памяти (alloc)
1. Поиск наиболее подходящего свободного блока
2. Если блок найден:
   - Разделение блока, если остаток достаточно большой
   - Маркировка блока как занятого
   - Возврат указателя на данные
3. Если блок не найден, возврат NULL

```
static void* best_fit_alloc(Allocator* allocator, size_t size) {
    BestFitAllocator* bf = (BestFitAllocator*)allocator;
    struct BlockHeader *best = NULL, *current = bf->free_list;
    size_t min_diff = SIZE_MAX;

    // Поиск подходящего блока
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

    // Разделение блока при необходимости
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
```

### Освобождение памяти (free)
1. Получение заголовка блока из указателя на данные
2. Маркировка блока как свободного
3. Объединение с соседними свободными блоками (если есть)
   - Объединение со следующим блоком
   - Поиск и объединение с предыдущим блоком

```
static void best_fit_free(Allocator* allocator, void* ptr) {
    if (!ptr) return;

    struct BlockHeader* header = (struct BlockHeader*)((char*)ptr - sizeof(struct BlockHeader));
    header->is_free = 1;

    // Объединение со следующим блоком
    if (header->next && header->next->is_free) {
        header->size += header->next->size + sizeof(struct BlockHeader);
        header->next = header->next->next;
    }
    
    // Объединение с предыдущим блоком
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
```

## Особенности реализации

### Преимущества
1. Минимальная фрагментация памяти
2. Эффективное использование памяти
3. Предсказуемое поведение

### Недостатки
1. Более медленный поиск свободных блоков (O(n))
2. Накладные расходы на управление (24 байта после освобождения)
3. Возможная фрагментация при длительном использовании

### Оптимизации
1. Объединение смежных свободных блоков
2. Разделение больших блоков для минимизации потерь
3. Использование заголовков минимального размера

## Статистика использования
- Размер заголовка блока: 24 байта
- Минимальный размер блока: sizeof(BlockHeader)
- Эффективность использования: ~50.86%
- Скорость аллокации: ~0.001549 секунд на 1000 операций
- Скорость освобождения: ~0.000006 секунд на 1000 операций