# Исследование алгоритмов аллокации памяти

## Описание алгоритмов

### Best-fit аллокатор
Алгоритм основан на поиске наиболее подходящего свободного блока памяти для запроса. 
- Использует связный список для отслеживания свободных блоков
- При выделении памяти ищет блок с минимальной разницей между запрошенным и доступным размером
- Поддерживает разделение блоков для минимизации внутренней фрагментации
- Объединяет смежные свободные блоки при освобождении памяти

### Power-of-2 аллокатор
Алгоритм использует фиксированные размеры блоков, округленные до степени двойки.
- Поддерживает отдельные списки для каждого размера блока (степени двойки)
- Быстрое выделение памяти благодаря отсутствию поиска
- Разделяет большие блоки на меньшие при необходимости
- Потенциально больший расход памяти из-за округления размеров

## Процесс тестирования

Реализовано тестирование следующих характеристик:
1. Скорость выделения памяти
2. Скорость освобождения памяти
3. Коэффициент использования памяти
4. Накладные расходы на управление памятью

Код тестирования:
```
#define MEMORY_SIZE (1024 * 1024)  // 1MB
#define TEST_ITERATIONS 1000

// Выделение случайных блоков размером 1-1024 байт
size_t size = rand() % 1024 + 1;
```

## Обоснование подхода тестирования

1. Размер памяти (1MB) выбран для моделирования реальных условий использования
2. 1000 итераций обеспечивают статистически значимые результаты
3. Случайные размеры блоков (1-1024 байт) имитируют реальные сценарии использования
4. Измерение времени выполняется с использованием clock() для точности
5. Отслеживание как запрошенной, так и фактически использованной памяти

## Результаты тестирования

### Best-fit аллокатор:
- Время аллокации: 0.001549 секунд
- Время освобождения: 0.000006 секунд
- Общая память: 1048544 байт
- Запрошено памяти: 509255 байт
- Использовано памяти: 533279 байт
- Коэффициент использования: 50.86%
- Накладные расходы: ~24 байт после освобождения

### Power-of-2 аллокатор:
- Время аллокации: 0.000278 секунд
- Время освобождения: 0.000012 секунд
- Общая память: 1048288 байт
- Запрошено памяти: 498242 байт
- Использовано памяти: 706720 байт
- Коэффициент использования: 67.42%
- Накладные расходы: 0 байт после освобождения

## Заключение

1. **Скорость работы:**
   - Power-of-2 аллокатор примерно в 5.5 раз быстрее при выделении памяти
   - Best-fit аллокатор в 2 раза быстрее при освобождении памяти

2. **Эффективность использования памяти:**
   - Best-fit более эффективен по использованию памяти (меньше фрагментация)
   - Power-of-2 имеет overhead около 42% из-за округления размеров
   
3. **Рекомендации по применению:**
   - Best-fit предпочтителен для систем с ограниченной памятью
   - Power-of-2 лучше подходит для систем, где критична скорость аллокации

4. **Особенности реализации:**
   - Best-fit требует более сложной реализации для управления памятью
   - Power-of-2 проще в реализации благодаря фиксированным размерам блоков