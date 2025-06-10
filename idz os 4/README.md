# ИДЗ4 ОС

## 1. Основные сведения
**ФИО студента:** Бабушкин Владимир Александрович
**Группа:** 237 
**Вариант задания:** 27 - "Задача о привлекательной студентке"

Задача о привлекательной студентке. У одной очень при-
влекательной студентки есть N поклонников. Традиционно в день
25
св. Валентина очень привлекательная студентка проводит роман-
тический вечер с одним из поклонников. Счастливый избранник
заранее не известен. С утра очень привлекательная студентка по-
лучает N «валентинок» с различными вариантами романтического
вечера. Выбрав наиболее заманчивое предложение, студентка из-
вещает счастливчика о своем согласии, а остальных — об отказе.
Требуется создать клиент–серверное приложение, моде-
лирующее поведение студентки.
Каждый изстудентов должен быть представлен отдельнымкли-
ентом. Студентка — сервер.
При решении использовать парадигму «клиент–сервер» с
активным ожиданием (предполагается, что поклонник не
сразу получает ответ, а только после того, как все они
сделают запросы, после чего студентка выберет лучшего
и разошлет все соответствующие уведомления). В прото-
коле вывода отметить реакцию каждого из студентов на
информацию, полученную от студентки...

## Общая структура проекта

Проект включает следующие компоненты:
- `server.c` - серверная часть (студентка)
- `client.c` - клиентская часть (поклонники)
- `logger_client.c` - система логирования
- `Makefile` - система сборки и управления тестами

## Запуск и тестирование через Makefile

### Основные команды:

```bash
make all      # Сборка всех компонентов
make clean    # Очистка проекта
make test     # Запуск тестового сценария
```

## Реализация на 4-5 баллов (базовая функциональность)

**Полная функциональность:**
1. Серверная часть (student_server.c):
   - Создает UDP сокет и привязывается к указанному порту
   - Принимает сообщения от клиентов в цикле
   - Сохраняет все полученные предложения
   - После получения N сообщений выбирает самое длинное
   - Отправляет "AGREE" выбранному клиенту и "REJECT" остальным
   - Использует неблокирующие операции для обработки подключений

2. Клиентская часть (client.c):
   - Подключается к серверу по указанному адресу и порту
   - Отправляет одно текстовое предложение
   - Ожидает ответ от сервера
   - Выводит полученный ответ ("AGREE"/"REJECT")

**Пример вывода:**
```
[admin@babushkin05server 4-5]$ make test 
Starting server on port 8080 (expecting 3 proposals)...
./server 8080 3 & \
SERVER_PID=$! ; \
sleep 1 ; \
echo "Launching clients..." ; \
./client 127.0.0.1 8080 "Dinner on the Eiffel Tower" & \
./client 127.0.0.1 8080 "Walk under the stars" & \
./client 127.0.0.1 8080 "Weekend in Venice" ; \
wait $SERVER_PID
Server started. Waiting for 3 proposals...
Launching clients...
Proposal sent: "Dinner on the Eiffel Tower"
Received proposal from 127.0.0.1:35642: Dinner on the Eiffel Tower
Proposal sent: "Walk under the stars"
Received proposal from 127.0.0.1:51016: Walk under the stars
Received proposal from 127.0.0.1:46188: Weekend in Venice
Selecting the best proposal...
Proposal sent: "Weekend in Venice"
Woohoo! She said yes!
Chosen proposal: Dinner on the Eiffel Tower
Oh no... Maybe next year.
Responses sent. Shutting down.
Oh no... Maybe next year.
```

## Реализация на 6-7 баллов (логгер)

**Расширенная функциональность:**
1. Новый компонент - logger_client.c:
   - Получает и отображает полную информацию о работе системы
   - Подключается к серверу как независимый наблюдатель
   - Регистрирует все ключевые события:
     * Подключение новых клиентов
     * Полученные предложения
     * Выбор лучшего предложения
     * Отправка ответов клиентам

2. Модификации сервера:
   - Добавлена отправка логов клиенту-наблюдателю
   - Реализован отдельный канал для логгирования
   - Клиент и сервер теперь ничего не выводят


**Пример вывода:**
```
Starting logger client on port 9000...
./logger_client 9000 & \
LOGGER_PID=$! ; \
sleep 1 ; \
echo "Starting server on port 8080 (expecting 3 proposals) with logger..." ; \
./server 8080 3 127.0.0.1 9000 & \
SERVER_PID=$! ; \
sleep 1 ; \
echo "Launching clients..." ; \
./client 127.0.0.1 8080 "Dinner on the Eiffel Tower" & \
./client 127.0.0.1 8080 "Walk under the stars" & \
./client 127.0.0.1 8080 "Weekend in Venice" ; \
wait $SERVER_PID ; \
kill $LOGGER_PID
Logger client started on port 9000...
Starting server on port 8080 (expecting 3 proposals) with logger...
Launching clients...
[LOG] Proposal received from 127.0.0.1:34594: Dinner on the Eiffel Tower
[LOG] Proposal received from 127.0.0.1:51024: Walk under the stars
[LOG] Proposal received from 127.0.0.1:55131: Weekend in Venice
[LOG] Best proposal chosen: Dinner on the Eiffel Tower
[LOG] Response sent to 127.0.0.1:34594: Agree
[LOG] Response sent to 127.0.0.1:51024: Reject
[LOG] Response sent to 127.0.0.1:55131: Reject
```

## Реализация на 8 баллов (множество логгеров)

**Дополнительные возможности:**
1. Поддержка нескольких параллельных логгеров
2. Механизм проверки активности:
   - Сервер периодически отправляет PING
   - Логгеры отвечают PONG
   - Неактивные логгеры автоматически отключаются
3. Команда STATUS для проверки состояния системы


**Пример вывода:**
```
[admin@babushkin05server 8]$ make test
Starting logger clients...
Logger started on port 9000 (PID 2755653)
Logger started on port 9001 (PID 2755654)
Logger started on port 9002 (PID 2755655)
Logger client started on port 9000...
Logger client started on port 9001...
Logger client started on port 9002...
Starting server with 3 loggers...
Launching 3 clients...
[LOG] Proposal received from 127.0.0.1:60153: Romantic dinner
[LOG] Proposal received from 127.0.0.1:60153: Romantic dinner
[LOG] Proposal received from 127.0.0.1:34610: Weekend getaway
[LOG] Proposal received from 127.0.0.1:34610: Weekend getaway
[LOG] Proposal received from 127.0.0.1:34307: Movie night
[LOG] Proposal received from 127.0.0.1:34307: Movie night
[LOG] Best proposal chosen: Romantic dinner
[LOG] Best proposal chosen: Romantic dinner
[LOG] Response sent to 127.0.0.1:60153: Agree
[LOG] Response sent to 127.0.0.1:34610: Reject
[LOG] Response sent to 127.0.0.1:34307: Reject
[LOG] Response sent to 127.0.0.1:60153: Agree
[LOG] Response sent to 127.0.0.1:34610: Reject
[LOG] Response sent to 127.0.0.1:34307: Reject
Stopping loggers...
```

## Реализация на 9 баллов (динамическое управление)

**Ключевые улучшения:**
1. Горячее подключение/отключение логгеров:
   - Команда REGISTER <port> для подключения
   - Команда UNREGISTER для отключения
   - Автоматическое переподключение при обрыве связи

2. Улучшенная обработка ошибок:
   - Повторная отправка при неудачной доставке
   - Буферизация сообщений при временной недоступности

**Пример вывода:**
```
[admin@babushkin05server 9]$ make test
Starting logger clients...
Logger started on port 9000 (PID 2755827)
Logger started on port 9001 (PID 2755828)
Logger started on port 9002 (PID 2755829)
Logger client started on port 9000
Sent registration to server 127.0.0.1:8080
Logger client started on port 9001
Sent registration to server 127.0.0.1:8080
Logger client started on port 9002
Sent registration to server 127.0.0.1:8080
Starting server with 3 loggers...
New logger registered: 127.0.0.1:9000 (1/10)
New logger registered: 127.0.0.1:9001 (2/10)
New logger registered: 127.0.0.1:9002 (3/10)
Server started on port 8080
Waiting for 3 client proposals...
Launching 3 clients...
[LOG] Proposal 1/3 from 127.0.0.1:59800: Romantic dinner
[LOG] Proposal 1/3 from 127.0.0.1:59800: Romantic dinner
[LOG] Proposal 2/3 from 127.0.0.1:55881: Weekend getaway
[LOG] Proposal 2/3 from 127.0.0.1:55881: Weekend getaway
[LOG] Proposal 3/3 from 127.0.0.1:52737: Movie night
[LOG] Selected best proposal: Romantic dinner (from 127.0.0.1:59800)
[LOG] Sent AGREE to 127.0.0.1:59800
[LOG] Sent REJECT to 127.0.0.1:55881
[LOG] Sent REJECT to 127.0.0.1:52737
Server completed. Best proposal selected.
[LOG] Proposal 1/3 from 127.0.0.1:59800: Romantic dinner
[LOG] Proposal 2/3 from 127.0.0.1:55881: Weekend getaway
[LOG] Proposal 3/3 from 127.0.0.1:52737: Movie night
[LOG] Selected best proposal: Romantic dinner (from 127.0.0.1:59800)
[LOG] Sent AGREE to 127.0.0.1:59800
[LOG] Sent REJECT to 127.0.0.1:55881
[LOG] Sent REJECT to 127.0.0.1:52737
[LOG] Proposal 3/3 from 127.0.0.1:52737: Movie night
[LOG] Selected best proposal: Romantic dinner (from 127.0.0.1:59800)
[LOG] Sent AGREE to 127.0.0.1:59800
[LOG] Sent REJECT to 127.0.0.1:55881
[LOG] Sent REJECT to 127.0.0.1:52737
Stopping loggers...
```

## Реализация на 10 баллов (корректное завершение)


**Финальные улучшения:**

Обработка SIGINT (Ctrl+C):
   - Рассылка SHUTDOWN всем клиентам
   - Уведомление логгеров о завершении работы
   - Корректное освобождение ресурсов


**Пример вывода:**
```
[admin@babushkin05server 10]$ make test
gcc -Wall -Wextra -O2 -o server server.c
server.c: In function ‘handle_sigint’:
server.c:35:24: warning: unused parameter ‘sig’ [-Wunused-parameter]
   35 | void handle_sigint(int sig) {
      |                    ~~~~^~~
Logger started on port 9000
[LOG] Client 1/3: 127.0.0.1:37171 - Proposal 1
[LOG] Client 2/3: 127.0.0.1:54980 - Proposal 2
[LOG] Client 3/3: 127.0.0.1:46337 - Proposal 3
[admin@babushkin05server 10]$ make test_sigint 
Testing SIGINT handling...
Logger started on port 9000
[LOG] Client 1/3: 127.0.0.1:53337 - Test 1
[LOG] Client 2/3: 127.0.0.1:33051 - Test 2
[LOG] SERVER SHUTTING DOWN
Server is shutting down. Goodbye!
Server is shutting down. Goodbye!
```

