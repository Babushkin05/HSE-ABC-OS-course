# Большое Домашнее Задание номер 4

## Юридическая информация

Бабушкин Владимир Александрович БПИ 237

Вариант номер 16 (Социалистическая гостиница)

Требования к решению доступны в файле [req-task-threads.pdf](req-task-threads.pdf)

Описание вариантов доступно в файле [task-threads.pdf](task-threads.pdf)

[Репозиторий с этим решением](https://github.com/Babushkin05/HSE-ABC-OS-course/tree/main/IDZ4)

## Задача

Задача о социалистической гостинице. В гостинице 30 одноместных номеров. Клиенты гостиницы снимают номер на одни или несколько суток (задается при создании клиента). Если в гостинице нет свободных номеров, клиенты не уходят, а устраиваются на рядом с гостиницей на скамейках и ждут в порядке очереди, пока любой номеров не освободится (других гостиниц в городе нет).
Создать многопоточное приложение, моделирующее работу гостиницы.Каждого клиента и гостиницу(точнее ее администратора) моделировать отдельным потоком.

![alt text](assets/hotel_moscow.png)

## Описание решения

первое решение находится в файле [/mutexes](/mutexes/)

Для удобства масштабирования, все потоки обернуты в отдельные классы.

Программа является смесью клиент-серверной модели управляющего и рабочих.

Клиентами являются проживающие в гостиницы: в день заселения они посылают запрос на проживание, путем добавления заявки в ```std::queue```. 

Сервер в лице администрации отеля принимает заявки через эту очередь, и предоставляет им номера, если они свободны.

Проживание гостиницы обернуто в отдельный класс, который выводит информацию о проживании и изменяет такие переменные как количество свободных комнат. Этот класс является рабочим, а его хозяином является администрация, которая запускает его выполнение.

В программе действует идея, что каждый день длится 1 секунду, и все потоки ориентируются на это. Благодаря этому, они живут по одному времени.

Значительного расхождения по времени не произойдет даже при огромных входных данных, потому что максимум в цикле внутри дня администрации происходит 30 итераций(ограничение на количество комнат в условии). А отдельные постояльцы в случайной генерации живут не больше недели, тем самым время между постояльцами и администрацией не успеет разойтись.

Для того, чтобы не случилось ошибок доступа к данным по архитектуре MISD (multiple instruction single data) используется такой синхропримитив как мьютексы.

То как описаны эти классы можно посмотреть в [Client.cpp](/mutexes/Client.cpp) и [Administration.cpp](/mutexes/Administration.cpp)

## Тесты:

Рассмотрим тесты

(то как представляются данные рассмотрим позже)

для файла [test1.txt](/mutexes/test1.txt) с содержимым

```
0 1 2
1 0 3
2 2 2
3 3 1
```

Выводится результат:

```
vovababuskin@Vovas-MacBook-Air rwlocks % ./socialistic_hotel -s test1.txt -o testout.txt
Day 0:
        Client 1 check-in into his room for 3 days.
Day 1:
        Client 0 check-in into his room for 2 days.
Day 2:
        Client 2 check-in into his room for 2 days.
        Client 1 leave his room.
        Client 0 leave his room.
Day 3:
        Client 3 check-in into his room for 1 days.
        Client 2 leave his room.
Day 4:
        Client 3 leave his room.
```
---
для файла [test2.txt](/mutexes/test2.txt) с содержимым

```
0 7 4
1 4 6
2 6 1
3 5 3
4 3 7
5 5 2
6 3 5
7 4 3
8 2 3
9 1 3
10 5 4
11 3 5
12 6 6
13 4 3
14 6 7
15 2 1
16 3 4
17 3 3
18 3 6
19 2 3
20 5 5
21 5 5
22 3 3
23 7 2
24 6 4
25 1 4
26 1 6
27 1 2
28 7 6
29 5 3
30 5 2
31 1 3
32 2 6
33 6 6
34 2 6
35 7 1
36 3 6
37 4 3
38 2 7
39 7 5
40 3 4
41 4 1
42 7 7
43 7 3
44 6 1
45 1 6
46 7 6
47 6 4
48 5 4
49 4 4
50 7 4
51 7 3
52 5 4
53 6 5
54 4 1
55 4 1
56 2 2
57 2 5
```
(данные были сгенерированы специальным методом, о котором ниже)

Выводится результат

```
Day 0:
Day 1:
        Client 31 check-in into his room for 3 days.
        Client 45 check-in into his room for 6 days.
        Client 9 check-in into his room for 3 days.
        Client 25 check-in into his room for 4 days.
        Client 27 check-in into his room for 2 days.
        Client 26 check-in into his room for 6 days.
Day 2:
        Client 38 check-in into his room for 7 days.
        Client 57 check-in into his room for 5 days.
        Client 15 check-in into his room for 1 days.
        Client 19 check-in into his room for 3 days.
        Client 32 check-in into his room for 6 days.
        Client 34 check-in into his room for 6 days.
        Client 56 check-in into his room for 2 days.
        Client 27 leave his room.
Day 3:
        Client 11 check-in into his room for 5 days.
        Client 4 check-in into his room for 7 days.
        Client 6 check-in into his room for 5 days.
        Client 16 check-in into his room for 4 days.
        Client 18 check-in into his room for 6 days.
        Client 22 check-in into his room for 3 days.
        Client 36 check-in into his room for 6 days.
        Client 17 check-in into his room for 3 days.
        Client 40 check-in into his room for 4 days.
        Client 15 leave his room.
        Client 31 leave his room.
        Client 9 leave his room.
        Client 56 leave his room.
Day 4:
        Client 13 check-in into his room for 3 days.
        Client 7 check-in into his room for 3 days.
        Client 37 check-in into his room for 3 days.
        Client 41 check-in into his room for 1 days.
        Client 49 check-in into his room for 4 days.
        Client 54 check-in into his room for 1 days.
        Client 55 check-in into his room for 1 days.
        Client 25 leave his room.
        Client 19 leave his room.
        Client 55 leave his room.
Day 5:
        Client 21 check-in into his room for 5 days.
        Client 48 check-in into his room for 4 days.
        Client 3 check-in into his room for 3 days.
        Client 5 check-in into his room for 2 days.
        Client 10 check-in into his room for 4 days.
        Client 30 check-in into his room for 2 days.
        Client 29 check-in into his room for 3 days.
        Client 52 check-in into his room for 4 days.
        Client 20 check-in into his room for 5 days.
        Client 41 leave his room.
        Client 54 leave his room.
        Client 17 leave his room.
        Client 22 leave his room.
Day 6:
        Client 33 check-in into his room for 6 days.
        Client 2 check-in into his room for 1 days.
        Client 12 check-in into his room for 6 days.
        Client 14 check-in into his room for 7 days.
        Client 45 leave his room.
        Client 26 leave his room.
        Client 57 leave his room.
        Client 13 leave his room.
        Client 7 leave his room.
        Client 16 leave his room.
        Client 40 leave his room.
        Client 37 leave his room.
        Client 5 leave his room.
        Client 30 leave his room.
Day 7:
        Client 2 leave his room.
        Client 44 check-in into his room for 1 days.
        Client 24 check-in into his room for 4 days.
        Client 28 check-in into his room for 6 days.
        Client 53 check-in into his room for 5 days.
        Client 35 check-in into his room for 1 days.
        Client 39 check-in into his room for 5 days.
        Client 47 check-in into his room for 4 days.
        Client 42 check-in into his room for 7 days.
        Client 23 check-in into his room for 2 days.
        Client 46 check-in into his room for 6 days.
        Client 51 check-in into his room for 3 days.
        Client 34 leave his room.
        Client 32 leave his room.
        Client 11 leave his room.
        Client 6 leave his room.
        Client 49 leave his room.
        Client 3 leave his room.
        Client 29 leave his room.
        Client 35 leave his room.
Day 8:
        Client 43 check-in into his room for 3 days.
        Client 50 check-in into his room for 4 days.
        Client 44 leave his room.
        Client 38 leave his room.
        Client 18 leave his room.
        Client 36 leave his room.
        Client 48 leave his room.
        Client 52 leave his room.
        Client 10 leave his room.
        Client 23 leave his room.
Day 9:
        Client 4 leave his room.
        Client 21 leave his room.
        Client 20 leave his room.
        Client 51 leave his room.
Day 10:
        Client 47 leave his room.
        Client 24 leave his room.
        Client 43 leave his room.
Day 11:
        Client 12 leave his room.
        Client 33 leave his room.
        Client 53 leave his room.
        Client 39 leave his room.
        Client 50 leave his room.
Day 12:
        Client 14 leave his room.
        Client 28 leave his room.
        Client 46 leave his room.
Day 13:
        Client 42 leave his room.
Day 14:
```
Вроде бы правильно(как минимум количество строчек правильное)

---


для файла [test3.txt](/mutexes/test3.txt) с содержимым

```
0 1 2
1 2 5
2 5 5
3 4 4
4 5 2
5 6 6
6 4 3
7 1 7
8 1 7
9 1 4
10 3 7
11 4 7
12 6 7
13 3 5
14 3 7
15 1 2
16 4 4
17 3 1
18 1 4
19 3 1
20 1 6
21 1 3
22 6 2
23 2 7
24 2 4
25 4 5
26 7 5
27 4 5
28 5 1
29 3 2
30 1 5
31 1 6
32 6 6
33 3 5
34 2 3
35 5 7
36 1 1
37 5 7
38 4 7
39 5 4
40 7 6
41 6 7
42 2 6
43 5 3
44 3 1
45 5 4
46 7 1
47 7 3
48 6 2
49 5 7
50 2 3
51 2 5
52 1 4
53 2 2
54 3 7
55 1 3
56 2 6
57 7 6
58 3 5
59 7 2
60 4 1
61 1 2
62 4 2
63 6 7
64 7 5
65 2 3
66 5 1
67 2 1
68 3 5
69 3 1
70 5 2
71 4 7
72 3 3
73 1 2
74 2 1
75 5 5
76 6 1
77 1 2
78 7 5
79 3 2
80 2 4
81 3 1
82 3 3
83 6 4
84 5 7
85 1 5
86 4 2
87 4 5
88 6 2
89 1 1
90 5 3
91 4 1
```
Выводится соответствующий результат:

```
Day 0:
Day 1:
        Client 0 check-in into his room for 2 days.
        Client 8 check-in into his room for 7 days.
        Client 15 check-in into his room for 2 days.
        Client 9 check-in into his room for 4 days.
        Client 18 check-in into his room for 4 days.
        Client 7 check-in into his room for 7 days.
        Client 20 check-in into his room for 6 days.
        Client 21 check-in into his room for 3 days.
        Client 30 check-in into his room for 5 days.
        Client 52 check-in into his room for 4 days.
        Client 36 check-in into his room for 1 days.
        Client 61 check-in into his room for 2 days.
        Client 31 check-in into his room for 6 days.
        Client 73 check-in into his room for 2 days.
        Client 55 check-in into his room for 3 days.
        Client 77 check-in into his room for 2 days.
        Client 85 check-in into his room for 5 days.
        Client 89 check-in into his room for 1 days.
Day 2:
        Client 36 leave his room.
        Client 89 leave his room.
        Client 24 check-in into his room for 4 days.
        Client 34 check-in into his room for 3 days.
        Client 1 check-in into his room for 5 days.
        Client 50 check-in into his room for 3 days.
        Client 51 check-in into his room for 5 days.
        Client 23 check-in into his room for 7 days.
        Client 53 check-in into his room for 2 days.
        Client 65 check-in into his room for 3 days.
        Client 67 check-in into his room for 1 days.
        Client 56 check-in into his room for 6 days.
        Client 74 check-in into his room for 1 days.
        Client 80 check-in into his room for 4 days.
        Client 42 check-in into his room for 6 days.
        Client 0 leave his room.
        Client 15 leave his room.
        Client 61 leave his room.
        Client 73 leave his room.
        Client 77 leave his room.
        Client 74 leave his room.
Day 3:
        Client 10 check-in into his room for 7 days.
        Client 14 check-in into his room for 7 days.
        Client 13 check-in into his room for 5 days.
        Client 17 check-in into his room for 1 days.
        Client 29 check-in into his room for 2 days.
        Client 33 check-in into his room for 5 days.
        Client 19 check-in into his room for 1 days.
        Client 67 leave his room.
        Client 55 leave his room.
        Client 21 leave his room.
        Client 19 leave his room.
Day 4:
        Client 54 check-in into his room for 7 days.
        Client 58 check-in into his room for 5 days.
        Client 44 check-in into his room for 1 days.
        Client 68 check-in into his room for 5 days.
        Client 53 leave his room.
        Client 17 leave his room.
        Client 18 leave his room.
        Client 9 leave his room.
        Client 52 leave his room.
        Client 29 leave his room.
        Client 65 leave his room.
        Client 34 leave his room.
        Client 50 leave his room.
Day 5:
        Client 44 leave his room.
        Client 69 check-in into his room for 1 days.
        Client 72 check-in into his room for 3 days.
        Client 79 check-in into his room for 2 days.
        Client 81 check-in into his room for 1 days.
        Client 82 check-in into his room for 3 days.
        Client 3 check-in into his room for 4 days.
        Client 15626636 check-in into his room for 1 days.
        Client 25 check-in into his room for 5 days.
        Client 27 check-in into his room for 5 days.
        Client 38 check-in into his room for 7 days.
        Client 30 leave his room.
        Client 85 leave his room.
        Client 80 leave his room.
        Client 24 leave his room.
        Client 81 leave his room.
        Client 69 leave his room.
        Client 15626636 leave his room.
Day 6:
        Client 60 check-in into his room for 1 days.
        Client 62 check-in into his room for 2 days.
        Client 71 check-in into his room for 7 days.
        Client 86 check-in into his room for 2 days.
        Client 87 check-in into his room for 5 days.
        Client 2 check-in into his room for 5 days.
        Client 91 check-in into his room for 1 days.
        Client 20 leave his room.
        Client 31 leave his room.
        Client 1 leave his room.
        Client 51 leave his room.
        Client 79 leave his room.
        Client 60 leave his room.
Day 7:
        Client 4 check-in into his room for 2 days.
        Client 28 check-in into his room for 1 days.
        Client 35 check-in into his room for 7 days.
        Client 43 check-in into his room for 3 days.
        Client 45 check-in into his room for 4 days.
        Client 66 check-in into his room for 1 days.
        Client 91 leave his room.
        Client 7 leave his room.
        Client 8 leave his room.
        Client 33 leave his room.
        Client 13 leave his room.
        Client 56 leave his room.
        Client 42 leave his room.
        Client 82 leave his room.
        Client 62 leave his room.
        Client 72 leave his room.
Day 8:
        Client 86 leave his room.
        Client 70 check-in into his room for 2 days.
        Client 75 check-in into his room for 5 days.
        Client 84 check-in into his room for 7 days.
        Client 90 check-in into his room for 3 days.
        Client 39 check-in into his room for 4 days.
        Client 37 check-in into his room for 7 days.
        Client 49 check-in into his room for 7 days.
        Client 5 check-in into his room for 6 days.
        Client 41 check-in into his room for 7 days.
        Client 12 check-in into his room for 7 days.
        Client 22 check-in into his room for 2 days.
        Client 28 leave his room.
        Client 66 leave his room.
        Client 23 leave his room.
        Client 58 leave his room.
        Client 68 leave his room.
        Client 3 leave his room.
        Client 4 leave his room.
Day 9:
        Client 32 check-in into his room for 6 days.
        Client 48 check-in into his room for 2 days.
        Client 63 check-in into his room for 7 days.
        Client 83 check-in into his room for 4 days.
        Client 76 check-in into his room for 1 days.
        Client 26 check-in into his room for 5 days.
        Client 88 check-in into his room for 2 days.
        Client 10 leave his room.
        Client 14 leave his room.
        Client 27 leave his room.
        Client 25 leave his room.
        Client 43 leave his room.
        Client 70 leave his room.
        Client 22 leave his room.
Day 10:
        Client 76 leave his room.
        Client 46 check-in into his room for 1 days.
        Client 59 check-in into his room for 2 days.
        Client 64 check-in into his room for 5 days.
        Client 78 check-in into his room for 5 days.
        Client 57 check-in into his room for 6 days.
        Client 54 leave his room.
        Client 2 leave his room.
        Client 87 leave his room.
        Client 45 leave his room.
        Client 90 leave his room.
        Client 48 leave his room.
        Client 88 leave his room.
Day 11:
        Client 46 leave his room.
        Client 38 leave his room.
        Client 39 leave his room.
        Client 59 leave his room.
Day 12:
        Client 71 leave his room.
        Client 75 leave his room.
        Client 83 leave his room.
Day 13:
        Client 35 leave his room.
        Client 5 leave his room.
        Client 26 leave his room.
Day 14:
        Client 37 leave his room.
        Client 49 leave his room.
        Client 84 leave his room.
        Client 41 leave his room.
        Client 12 leave his room.
        Client 32 leave his room.
        Client 78 leave his room.
        Client 64 leave his room.
Day 15:
        Client 63 leave his room.
        Client 57 leave his room.
```

Как и в прошлом, вроде бы правильно.

---

Также еще проводились тесты, с уменьшением количества комнат в гостинице, для проверки правильности ожидания клиентов в очереди. Дали корректный результат.

## Флаги

Для понимания работы с флагами был разработан флаг ```-h```:

```
vovababuskin@Vovas-MacBook-Air mutexes % ./socialistic_hotel -h


        Welcome to Socialistic Hotel Modulation Manual
--------------------------------------------------------------
                -o <file> : output to file
                -s <file> : read data from file
                -r : generate data randomly
                -h : writes this manual
---------------------------------------------------------------
                How to Write Data:
                        0 1 2
hotel client with id=0, who comes in the day=1 and live for 2 days


vovababuskin@Vovas-MacBook-Air mutexes % 
```

Тут же как раз и описан метод записи данных о посетителях в файл.

Обработкой флагов занимается специальный метод ```ProgramParams parse_args(int argc, char **argv)```в [program.cpp](/mutexes/program.cpp)

Также этот метод отправляет запросы на чтение из файла или генерацию данных о постояльцах отеля. Давайте рассмотрим их подробнее

## Чтение данных из файла

Этим занимается файл [read_file.cpp](/mutexes/read_file.cpp)

```cpp
#pragma once
#include "Client.cpp"
#include <fstream>
#include <string>
#include <vector>

std::vector<Client> read_clients_from_file(std::string filename,
                                           std::queue<Client *> &waiters,
                                           pthread_rwlock_t &mutex) {
  std::vector<Client> clients;
  std::ifstream file(filename);
  if (!file.is_open()) {
    std::cerr << "ERROR :: there is no file " << filename;
    exit(1);
  }
  while (file.good()) {
    int id, checkin, duration;
    file >> id >> checkin >> duration;
    clients.emplace_back(id, checkin, duration, waiters, mutex);
  }
  return clients;
}
```

Тут в целом ничего сложного

## Генерация Данных

Этим занимается файл [generate_clients.cpp](/mutexes/generate_clients.cpp)

```cpp
#pragma once
#include "Client.cpp"
#include <queue>
#include <random>
#include <vector>

const int MAX_CLIENT_NUMBER = 100;
const int MAX_DAY_NUMBER = 7;

std::vector<Client> generateClients(std::queue<Client *> &waiters,
                                    pthread_rwlock_t &mutex) {
  std::random_device random_dev;
  std::mt19937 generator(random_dev());
  std::uniform_int_distribution<> client_number_distr(10, MAX_CLIENT_NUMBER);
  std::uniform_int_distribution<> day_distr(1, MAX_DAY_NUMBER);

  int n = client_number_distr(generator);
  std::vector<Client> clients;
  for(size_t i = 0; i< n;++i){
    int checkin = day_distr(generator);
    int duration = day_distr(generator);
        std::cout<<i<<' '<<checkin<<' '<<duration<<'\n';
    clients.emplace_back(i,checkin,duration,waiters, mutex);
  }

  return clients;
}
```

```std::mt19937``` считается лучшим генератором, чем обычное распределение в библиотеке c++

Ограничения распределения обусловлены удобством.

## Вывод в файл

Вывод в файл совершается путем дубликации ```frpintf(fptr, ...)``` рядом с каждым ```printf(...)```

## Решение с использованием других синхропримитивов

Было решено использовать синхропримитив ```pthread_rwlock_t```, потому что он предоставляет те же возможности что и мьютекс, только работает по другому

Таким образом сильно менять бизнес-логику не пришлось. Только заменить часть локов и анлоков на закрытие на чтение, а часть других локов на закрытие на запись.

И конечно, ответ получился тем же, что является еще одним доказательством что первые вывод был правильным.

## Доказательство удовлетворения критериям

## 4-5

### В отчете должен быть приведен сценарий, описывающий одновременное поведение представленных в условии задания сущностей в терминах предметной области. То есть, описан сценарий, задающий ролевое поведение субъектов и объектов задачи (интерпретация условия с большей степенью детализации происходящего), а не то, как это будет реализовано в программе.

Описал выше.

### Описана модель параллельных вычислений, используемая при разработке многопоточной программы.

Там же

### Описаны входные данные программы, включающие вариативные диапазоны, возможные при многократных запусках.

В разделе про флаги.

### Реализовано консольное приложение, решающее поставленную задачу с использованием одного варианта изученных синхропримитивов.

Да

### Ввод данных в приложение реализован с консоли во время выполнения программы (без использования аргументов командной строки).

Было решено не делать эту возможность, а сразу использовать методы более высокого уровня.

### Для используемых генераторов случайных чисел описаны их диапазоны и то, как интерпретируются данные этих генераторов.

Диапазоны обьяснил в разделе про генерацию, интерпретация понятна по названиям переменных.

### Вывод программы должен быть информативным, отражая все ключевые протекающие в ней события в терминах предметной области. Наблюдатель на основе вывода программы должен понимать, что в ней происходит в каждый момент времени ее работы.

Да, это можно увидеть в разделе тестов

### В программе присутствуют комментарии, поясняющие выполняемые действия и описание используемых объектов и переменных.

Да, например в файле [/mutexes/program.cpp](/mutexes/program.cpp)

### Результаты работы программы представлены в отчете.

Подробнее в разделе тесты

## 6-7

### В отчете подробно описан обобщенный алгоритм, используемый при реализации программы исходного словесного сценария. В котором показано, как на программу отображается каждый из субъектов предметной области.

Описал его выше в части решения.

### В программу добавлена генерация случайных данных в допустимых диапазонах.

Да, описал выше.

### Реализован ввод исходных данных из командной строки при запуске программы вместо ввода параметров с консоли во время выполнения программы.

Да, описал выше

### Результаты изменений отражены в отчете.

Да

## 8

### В программу, наряду с выводом в консоль, добавлен вывод результатов в файл. Имя файла для вывода данных задается в командной строке как один из ее параметров.

Да

### Результаты работы программы должны выводиться на экран и записываться в файл.

Да

### Наряду с вводом исходных данных через командную строку добавлен альтернативный вариант их ввода из файла, который по сути играет роль конфигурационного файла. Имя этого файла задается в командной строке вместо параметров, которые в этом случае не вводятся. Управление вводом в командной строке осуществляется с использованием соответствующих ключей.

Да

### Приведено не менее трех вариантов входных и выходных файлов с различными исходным данными и результатами выполнения программы.

Да, и они разоьраны в разделе тесты

### Ввод данных из командной строки расширен с учетом введенных изменений.

Да

### Результаты изменений отражены в отчете.

Да

## 9

### Разработано альтернативное решение, построенное на других синхропримитивах. При этом следует учесть, что мьютексы и семафоры — это по сути один тип синхропримитивов. Поэтому мьютексы на семафоры и обратно не обменивать. При их наличии в первой программе следует использовать что-то еще. Но допускается использовать мьютексы (как простейшие синхропримитивы) наряду с другими новыми синхропримитивами.

Разработал и написал об этом в разделе второе решение.

### Приведен сравнительный анализ поведения с ранее разработанной программой. Он заключается в проверке идентичности поведения при одинаковых данных.

Провел, но не записал в отчет, так как тесты и так занимают много места в отчете. Но как я уже сказал, рвлоки предоставляют такой же интерфейс как мьютексы, так что ничего существенно не меняется.

### Результаты изменений отражены в отчете.

Ну вот.

## 10

Первая домашка сделанная не на 10... Дедлайны сейчас поджимают на последней неделе...

Вроде, как я посчитал, если все домашки на 10 и последнее идз на 9, то все-таки должна выйти 10.