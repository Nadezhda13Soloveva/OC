# Отчет по лабораторной работе № 5 (5-7)
## по курсу "Операционные системы"

Студент группы М8О-208Б-23 Соловьева Надежда Сергеевна

Работа выполнена 

Преподаватель: Егор Живалев

1. **Тема**: Сервера сообщений      
2. **Цель работы**:  
   - Управлении серверами сообщений      
   - Применение отложенных вычислений   
   - Интеграция программных систем друг с другом   
   
3. **Задание**:  Реализовать распределенную систему по асинхронной обработке запросов. В данной распределенной системе должно существовать 2 вида узлов: «управляющий» и «вычислительный». Необходимо объединить данные узлы в соответствии с той топологией, которая определена вариантом. Связь между узлами необходимо осуществить при помощи технологии очередей сообщений. Также в данной системе необходимо предусмотреть проверку доступности узлов в соответствии с вариантом. При убийстве («kill -9») любого вычислительного узла система должна пытаться максимально сохранять свою работоспособность, а именно все дочерние узлы убитого узла могут стать недоступными, но родительские узлы должны сохранить свою работоспособность.  
Управляющий узел отвечает за ввод команд от пользователя и отправку этих команд на вычислительные узлы.   
Список основных поддерживаемых команд:  
    1.  Создание нового вычислительного узла: create id [parent]  
    2. Исполнение команды на вычислительном узле: exec id [params]  

Возможные технологии очередей сообщений:  
    ● ZeroMQ  
    ● MSMQ  
    ● RabbitMQ  
    ● Nats  

   Вариант 9:   
   Топология 1: *"Все вычислительные узлы находятся в списке. Есть только один управляющий узел. Чтобы добавить новый вычислительный узел к управляющему, то необходимо выполнить команду: create id -1."*  
   Набор команд 2 (локальный целочисленный словарь): *"Формат команды сохранения значения: exec id name value, где id – целочисленный идентификатор вычислительного узла, на который отправляется команда, name – ключ, по которому будет сохранено значение (строка формата [A-Za-z0-9]+), value – целочисленное значение. Формат команды загрузки значения: exec id name"*   
   Команда проверки 2: *"Формат команды: ping id. Команда проверяет доступность конкретного узла. Если узла нет, то необходимо выводить ошибку: «Error: Not found» "*  
   

4. **Код решения**: [controller](controller.c), [worker](worker.c)  
5. **Тестовые данные**: [test](tests/test5_controller.cpp)  
6. **Вывод:** В процессе выполнения лабораторной работы я чуть не отбросила коньки. Какой-то лютый треш, но мы справились. Лирическое отступление закончено. Так вот, в роцессе выполнения лабораторной работы успешно реализовали систему с ассинхронной обработкой запросов. Научились обеспечивать управление серверами сообщений с помощью технологии очередей сообщений ZeroMQ. Как говорится: "Этот год был непростым". Личная оценка: 6/10.