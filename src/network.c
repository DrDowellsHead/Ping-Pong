#include "network.h"

#include <arpa/inet.h>  // Для работы с IP-адресами
#include <netdb.h>    // Для работы с сетевыми именами
#include <pthread.h>  // Многопоточность
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>  // Для работы с сокетами
#include <unistd.h>      // POSIX API (close, read, write)

// Инициализация сетевого контекста
void network_init(network_context_t *ctx) {
    ctx->listening_socket = -1;  // -1 - "не инициализирован"
    ctx->connected_socket = -1;  // -1 - "не инициализирован"
    ctx->is_server_mode = 0;     // По умолчанию не сервер
    ctx->game_ready = 0;         // Игра не готова к началу
}

// Получение локального IP-адреса
void network_get_local_ip(char *buffer, size_t buffer_size) {
    struct hostent *host;  // Структура для информации о хосте
    char hostname[256];  // Буфер для имени хоста

    // Имя текущего компьютера
    gethostname(hostname, sizeof(hostname));

    // Информация о хосте по имени
    host = gethostbyname(hostname);

    if (host == NULL) {
        strcpy(buffer, "127.0.0.1");
        return;
    }

    // Преобразование бинарный IP в текствовый формат
    // host->h_addr_list[0] - первый из списка адресов
    inet_ntop(AF_INET, host->h_addr_list[0], buffer, buffer_size);
}

// Вспомогательная функция для потока прослушивания
static void *listener_thread(void *arg) {
    network_context_t *ctx = (network_context_t *)arg;  // Получение контекста
    struct sockaddr_in address;  // Для адреса сокета
    int opt = 1;                 // Значение для setsockopt
    int addrlen = sizeof(address);  // Длина адресной структуры

    // Создание TCP сокета
    // AF_INET - IPv4, SOCK_STREAM - TCP, 0 - протокол по умолчанию
    if ((ctx->listening_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Повторное получение адреса
    // Позволяет перезапустить сервер без ожидания таймаута
    if (setsockopt(ctx->listening_socket, SOL_SOCKET, SO_REUSEADDR, &opt,
                   sizeof(opt))) {
        perror("setsockopt");
        close(ctx->listening_socket);
        exit(EXIT_FAILURE);
    }

    // Заполнение структуры адреса для привязки
    address.sin_family = AF_INET;  // IPv4
    address.sin_addr.s_addr = INADDR_ANY;  // Принимать на всех интерфейсах
    address.sin_port =
        htons(PORT);  // Порт 8080, преобразованный в сетевой порядок

    // Привязка сокета к адресу и порту
    if (bind(ctx->listening_socket, (struct sockaddr *)&address,
             sizeof(address)) < 0) {
        perror("bind failde");
        close(ctx->listening_socket);
        exit(EXIT_FAILURE);
    }

    // Прослушивание входящих подключений
    // 3 - максимальное количество ожидающих подключений в очереди
    if (listen(ctx->listening_socket, 3) < 0) {
        perror("listen");
        close(ctx->listening_socket);
        exit(EXIT_FAILURE);
    }

    // Приём входящих подключений (блокирующая операция)
    if ((ctx->connected_socket =
             accept(ctx->listening_socket, (struct sockaddr *)&address,
                    (socklen_t *)&addrlen)) < 0) {
        perror("accept");
        close(ctx->listening_socket);
        exit(EXIT_FAILURE);
    }

    // Флаги состояния
    ctx->is_server_mode = 1;  // Режим сервера
    ctx->game_ready = 1;      // Игра готова к началу

    return NULL;
}

// Запуск прослушивающего потока в фоновом режиме
void network_start_listener(network_context_t *ctx) {
    pthread_t thread_id;  // Индетификатор потока
    // Создание нового потока
    pthread_create(&thread_id, NULL, listener_thread, ctx);
    // Отделение потока, он завершится при выходе из программы
    pthread_detach(thread_id);
}

// Подключение к другому пиру
void network_connect_to_peer(network_context_t *ctx, const char *peer_ip) {
    struct sockaddr_in serv_addr;  // Адрес пира

    // Создание TCP сокета для подключения
    if ((ctx->connected_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Ошибка создания сокета");
        exit(EXIT_FAILURE);
    }

    // Заполнение структуры адреса сервера
    serv_addr.sin_family = AF_INET;    // IPv4
    serv_addr.sin_port = htons(PORT);  // Порт 8080

    // Преобразование текстового IP в бинарный формат
    if (inet_pton(AF_INET, peer_ip, &serv_addr.sin_addr) <= 0) {
        printf("\nНеверный адрес: %s", peer_ip);
        close(ctx->connected_socket);
        exit(EXIT_FAILURE);
    }

    // Установка соединения с сервером
    if (connect(ctx->connected_socket, (struct sockaddr *)&serv_addr,
                sizeof(serv_addr)) < 0) {
        printf("´\nОшибка подключения к %s", peer_ip);
        close(ctx->connected_socket);
        exit(EXIT_FAILURE);
    }

    ctx->game_ready = 1;  // Соединение установлено, игра готова
}

// Отправка состояния игры пиру
void network_send_game_state(const network_context_t *ctx,
                             const game_state_t *state) {
    char buffer[BUFFER_SIZE];  // Буфер данных

    // Форматирование состояния в строку
    // Форматируются X,Y положения ракеток, положение X,Y мяча, очки и комманды
    snprintf(buffer, BUFFER_SIZE, "%d, %d, %d, %d, %d, %d, %d", state->lRacketY,
             state->rRacketY, state->ballX, state->ballY, state->lScore,
             state->rScore, state->command);

    // Отправка данных через сокет
    // MSG_NOSIGNAL - избежание сигнала SIGPIPE при разрыве соединения
    send(ctx->connected_socket, buffer, strlen(buffer), MSG_NOSIGNAL);
}

// Получение состояния игры от пира
int network_receive_game_state(const network_context_t *ctx,
                               game_state_t *state) {
    char buffer[BUFFER_SIZE] = {0};  // Буфер для обмена

    // Чтение данных из сокета (неблокирубщее чтение)
    int bytes_received = read(ctx->connected_socket, buffer, BUFFER_SIZE - 1);

    // Если данные получены
    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';  // Завершающий ноль
        // Преобразование строки обратно в структуру состояния
        sscanf(buffer, "%d, %d, %d, %d, %d, %d, %d", &state->lRacketY,
               &state->rRacketY, &state->ballX, &state->ballY, &state->lScore,
               &state->rScore, &state->command);
        return 1;  // Данные успешно получены
    }
    return 0;  // Ошибка получения данных
}

// Очистка сетевых ресурсов
void network_cleanup(network_context_t *ctx) {
    // Закрытие сокета соединения, если он был открыт
    if (ctx->connected_socket >= 0)
        close(ctx->connected_socket);
    // Закрытие сокета прослушивания, если он был открыт
    if (ctx->listening_socket >= 0)
        close(ctx->connected_socket);
}
