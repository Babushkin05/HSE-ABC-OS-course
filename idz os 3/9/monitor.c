#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <errno.h>
#include <ncurses.h>
#include <locale.h>

#define BUFFER_SIZE 1024
#define REFRESH_DELAY 1

void print_gallery_status(int visitors, int paintings[5]) {
    clear();
    printw("=== Картинная галерея ===\n\n");
    printw("Общее количество посетителей: %d/25\n\n", visitors);
    
    printw("Посетители у картин:\n");
    for (int i = 0; i < 5; i++) {
        printw("Картина %d: %d/5 посетителей\n", i+1, paintings[i]);
    }
    
    printw("\nНажмите q для выхода\n");
    refresh();
}

int main(int argc, char *argv[]) {
    setlocale(LC_ALL, "");
    if (argc != 3) {
        printf("Использование: ./monitor <server_ip> <server_port>\n");
        return 1;
    }

    const char* server_ip = argv[1];
    int server_port = atoi(argv[2]);

    // Инициализация ncurses
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);
    curs_set(0);

    // Создание соединения с сервером
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        endwin();
        perror("Ошибка создания сокета");
        return 1;
    }

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(server_port);

    if (inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0) {
        endwin();
        printf("Неверный IP адрес\n");
        close(sock);
        return 1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        endwin();
        perror("Ошибка подключения");
        close(sock);
        return 1;
    }

    // Регистрируемся как монитор
    send(sock, "MONITOR", 7, 0);

    while (1) {
        // Проверка нажатия клавиши q для выхода
        int ch = getch();
        if (ch == 'q' || ch == 'Q') {
            break;
        }

        // Чтение статуса от сервера
        char response[BUFFER_SIZE] = {0};
        int bytes_read = recv(sock, response, BUFFER_SIZE - 1, 0);
        
        if (bytes_read > 0) {
            response[bytes_read] = '\0';
            
            // Парсинг ответа
            int visitors;
            int paintings[5];
            if (sscanf(response, "%d %d %d %d %d %d", 
                      &visitors, &paintings[0], &paintings[1], 
                      &paintings[2], &paintings[3], &paintings[4]) == 6) {
                print_gallery_status(visitors, paintings);
            }
        } else if (bytes_read == 0) {
            // Соединение закрыто сервером
            break;
        }

        napms(100); 
    }

    close(sock);
    endwin();
    return 0;
}