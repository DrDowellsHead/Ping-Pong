#define _POSIX_C_SOURCE 200112L
#define _DEFAULT_SOURCE
#define _BSD_SOURCE

#include "network_ssl.h"

#include <arpa/inet.h>  // Для работы с IP-адресами
#include <errno.h>
#include <ncurses.h>
#include <netdb.h>    // Для работы с сетевыми именами
#include <pthread.h>  // Многопоточность
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>  // Для работы с сокетами (socket, bind, listen, accept)
#include <unistd.h>  // POSIX API (close, read, write, gethostname)

// Инициализация OpenSSL
int init_openssl() {
    SSL_library_init();  // Инициализация библиотеки SSL
    SSL_load_error_strings();  // Текстовые описания ошибок SSL
    OpenSSL_add_ssl_algorithms();  // Регистрация доступных SSL алгоритмов
    return 1;
}

// Инициализация сетевого контекста для сервера
SSL_CTX *create_server_ssl_context() {
    const SSL_METHOD *method = TLS_server_method();  // Метод SSL/TLS
    SSL_CTX *ctx =
        SSL_CTX_new(method);  // Контекст SSL (хранит настройки шифрования)

    // Проверка успешности создания контекста
    if (!ctx) {
        perror("Unable to create SSL context");
        ERR_print_errors_fp(stderr);  // Вывод ошибок
        exit(EXIT_FAILURE);
    }

    // Загрузка сертификата для сервера
    if (SSL_CTX_use_certificate_file(ctx, "cert.pem", SSL_FILETYPE_PEM) <= 0) {
        fprintf(stderr, "Error loading certificate\n");
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    // Загрузка приватного ключа для сервера
    if (SSL_CTX_use_PrivateKey_file(ctx, "key.pem", SSL_FILETYPE_PEM) <= 0) {
        fprintf(stderr, "Error loading private key\n");
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    // Проверка соответствия ключа и сертификата
    if (!SSL_CTX_check_private_key(ctx)) {
        fprintf(stderr, "Private key does not match the certificate\n");
        exit(EXIT_FAILURE);
    }

    return ctx;

    // Дополнительные настройки контекста (можно добавить сертификаты и ключи
    // здесь)
    // SSL_CTX_set_ecdh_auto(ctx,
    //                       1);  // Включение ECDH для Perfect Forward Secrecy
    // SSL_CTX_use_certificate_file(ctx, "cert.pem",
    //                              SSL_FILETYPE_PEM);  // Загрузка сертификата
    // SSL_CTX_use_PrivateKey_file(ctx, "key.pem",
    //                             SSL_FILETYPE_PEM);  // Загрузка приватного
    //                             ключа

    // return ctx;
}

// Создание SSL контекста для клиента
SSL_CTX *create_client_ssl_context() {
    const SSL_METHOD *method = TLS_client_method();  // Клиентский метод TLS
    SSL_CTX *ctx = SSL_CTX_new(method);  // Создание контекста

    if (!ctx) {
        perror("Unable to create SSL context");
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    return ctx;
}

// Очистка ресурсов OpenSSL
void cleanup_openssl() { EVP_cleanup(); }

// Инициализация сетевого контекста
void network_init(network_context_t *ctx) {
    ctx->listening_socket = -1;  // -1 - "не инициализирован"
    ctx->connected_socket = -1;  // -1 - "не инициализирован"
    ctx->is_server_mode = 0;     // По умолчанию не сервер
    ctx->game_ready = 0;         // Игра не готова к началу
    ctx->ssl_ctx = NULL;
    ctx->ssl = NULL;

    // Инициализация OpenSSL
    init_openssl();
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

// Поток прослушивания SSL
static void *listener_thread(void *arg) {
    network_context_t *ctx =
        (network_context_t *)arg;  // Получение контекста из аргумента
    struct sockaddr_in address;  // Структура для адреса сокета
    int opt = 1;  // Значение для setsockopt (включение опции)
    int addrlen = sizeof(address);  // Длина адресной структуры
    int ssl_err;

    // Создание SSL контекста для сервера
    ctx->ssl_ctx = create_server_ssl_context();

    // Создание TCP сокета
    // AF_INET - IPv4, SOCK_STREAM - TCP, 0 - протокол по умолчанию
    if ((ctx->listening_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Настройка опций сокета: разрешение повторного использования адреса
    // Это позволяет перезапускать сервер без ожидания таймаута освобождения
    // порта
    if (setsockopt(ctx->listening_socket, SOL_SOCKET, SO_REUSEADDR, &opt,
                   sizeof(opt))) {
        perror("setsockopt");
        close(ctx->listening_socket);
        exit(EXIT_FAILURE);
    }

    // Заполнение структуры адреса для привязки сокета
    address.sin_family = AF_INET;  // IPv4
    address.sin_addr.s_addr =
        INADDR_ANY;  // Принимать соединения на все сетевые интерфейсы
    address.sin_port =
        htons(PORT);  // Порт 8080 (преобразованный в сетевой порядок байт)

    // Привязка сокета к адресу и порту
    if (bind(ctx->listening_socket, (struct sockaddr *)&address,
             sizeof(address)) < 0) {
        perror("bind failed");
        close(ctx->listening_socket);
        exit(EXIT_FAILURE);
    }

    // Начало прослушивания входящих подключений
    // 3 - максимальная длина очереди ожидающих подключений
    if (listen(ctx->listening_socket, 3) < 0) {
        perror("listen\n");
        close(ctx->listening_socket);
        exit(EXIT_FAILURE);
    }

    // Таймаут для ACCEPT
    struct timeval timeout;
    timeout.tv_sec = 30;
    timeout.tv_usec = 0;
    setsockopt(ctx->listening_socket, SOL_SOCKET, SO_RCVTIMEO, &timeout,
               sizeof(timeout));

    // Принятие входящего подключения (блокирующая операция)
    // accept ждет, пока клиент подключится к серверу
    if ((ctx->connected_socket =
             accept(ctx->listening_socket, (struct sockaddr *)&address,
                    (socklen_t *)&addrlen)) < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            printf("Accept timeout\n");
        } else {
            perror("accept\n");
        }
        close(ctx->listening_socket);
        pthread_exit(NULL);
    }

    // === SSL HANDSHAKE (Рукопожатие) - Начинается шифрование ===

    // Создание SSL структуры для нового соединения
    ctx->ssl = SSL_new(ctx->ssl_ctx);

    // Привязка SSL структуры к файловому дескриптору сокета
    SSL_set_fd(ctx->ssl, ctx->connected_socket);

    // Сервер принимает SSL соединение (инициирует handshake)
    if (SSL_accept(ctx->ssl) <= 0) {
        // Ошибка SSL Handshake
        ssl_err = SSL_get_error(ctx->ssl, -1);
        printf("SSL accept failed: %d\n", ssl_err);
        ERR_print_errors_fp(stderr);
        close(ctx->connected_socket);
        close(ctx->listening_socket);
        pthread_exit(NULL);
    }

    // Handshake удался, соединение зашифровано
    printf("SSL соединение установлено. Используется шифр: %s\n",
           SSL_get_cipher(ctx->ssl));  // Вывод информации о используемом шифре

    ctx->is_server_mode = 1;  // Теперь мы в режиме сервера
    ctx->game_ready = 1;      // Игра готова к началу

    return NULL;  // Завершение потока
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
    struct sockaddr_in serv_addr;  // Структура адреса пира
    int ssl_err;

    ctx->ssl_ctx = create_client_ssl_context();

    // Создание TCP сокета для подключения
    if ((ctx->connected_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        mvprintw(17, 25, "Socket creation error: %s", strerror(errno));
        refresh();
        ctx->game_ready = 0;
        return;
    }

    // Установка таймаута для соединения
    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    setsockopt(ctx->connected_socket, SOL_SOCKET, SO_SNDTIMEO, &timeout,
               sizeof(timeout));
    setsockopt(ctx->connected_socket, SOL_SOCKET, SO_RCVTIMEO, &timeout,
               sizeof(timeout));

    // Заполнение структуры адреса пира
    serv_addr.sin_family = AF_INET;    // IPv4
    serv_addr.sin_port = htons(PORT);  // Порт 8080

    // Преобразование текстового IP в бинарный формат
    if (inet_pton(AF_INET, peer_ip, &serv_addr.sin_addr) <= 0) {
        mvprintw(17, 25, "Invalid IP address: %s", peer_ip);
        refresh();
        close(ctx->connected_socket);
        ctx->connected_socket = -1;
        ctx->game_ready = 0;
        return;
    }

    // Установка соединения с пиром
    if (connect(ctx->connected_socket, (struct sockaddr *)&serv_addr,
                sizeof(serv_addr)) < 0) {
        mvprintw(17, 25, "Connection failed to %s", peer_ip);
        refresh();
        close(ctx->connected_socket);
        ctx->connected_socket = -1;
        ctx->game_ready = 0;
        return;
    }

    // === SSL HANDSHAKE со стороны клиента ===

    // Создание SSL структуры
    ctx->ssl = SSL_new(ctx->ssl_ctx);

    // Привязка SSL структуры к файловому дескриптору сокета
    SSL_set_fd(ctx->ssl, ctx->connected_socket);

    // Клиент инициализирует SSL соединение
    if (SSL_connect(ctx->ssl) <= 0) {
        // Ошибка SSL Handshake
        ssl_err = SSL_get_error(ctx->ssl, -1);
        mvprintw(17, 25, "SSL handshake failed: %d", ssl_err);
        refresh();
        ERR_print_errors_fp(stderr);
        close(ctx->connected_socket);
        ctx->connected_socket = -1;
        ctx->game_ready = 0;
        return;
    }

    // Handshake успешен, соединение зашифровано
    mvprintw(17, 25, "SSL connection established! Cipher: %s",
             SSL_get_cipher(ctx->ssl));
    refresh();
    // printf("SSL соединение установлено. Используется шифр: %s\n",
    //        SSL_get_cipher(ctx->ssl));  // Вывод информации о используемом
    //        шифре

    ctx->game_ready = 1;  // Соединение установлено, игра готова
}

// Отправка состояния игры пиру с SSL шифрованием
void network_send_game_state(const network_context_t *ctx,
                             const game_state_t *state, ball_velocity_t *vel) {
    char buffer[BUFFER_SIZE];  // Буфер данных

    // Форматирование состояния в строку
    // Форматируются X,Y положения ракеток, положение X,Y мяча, очки и комманды
    snprintf(buffer, BUFFER_SIZE, "%d,%d,%d,%d,%d,%d,%d,%d,%d", state->lRacketY,
             state->rRacketY, state->ballX, state->ballY, state->lScore,
             state->rScore, state->command, vel->velX, vel->velY);

    // Отправка данных через SSL, данные автоматически шифруются
    // SSL_write обрабатывает всё шифрование прозрачно для нас
    int bytes_sent = SSL_write(ctx->ssl, buffer, strlen(buffer));

    // Проверка успешности отправки
    if (bytes_sent <= 0) {
        // Обработка ошибки SSL
        int err = SSL_get_error(ctx->ssl, bytes_sent);
        printf("Ошибка отправки SSL: %d\n", err);
    }
}

// Получение состояния игры от пира с SSL дешифрованием
int network_receive_game_state(const network_context_t *ctx,
                               game_state_t *state, ball_velocity_t *vel) {
    char buffer[BUFFER_SIZE] = {0};  // Буфер для приема данных

    // Чтение данных из через SSL - данные автоматически дешифруются
    int bytes_received = SSL_read(ctx->ssl, buffer, BUFFER_SIZE - 1);

    // Если данные получены
    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';  // Завершающий ноль

        // Преобразование строки обратно в структуру состояния
        sscanf(buffer, "%d,%d,%d,%d,%d,%d,%d,%d,%d", &state->lRacketY,
               &state->rRacketY, &state->ballX, &state->ballY, &state->lScore,
               &state->rScore, &state->command, &vel->velX, &vel->velY);
        return 1;  // Данные успешно получены
    } else if (bytes_received == 0) {
        // Соединение закрыто пиром
        printf("SSL соединение закрыто\n");
    } else {
        // Ошибка чтения через SSL
        int err = SSL_get_error(ctx->ssl, bytes_received);
        printf("Ошибка чтения SSL: %d\n", err);
    }

    return 0;  // Ошибка получения данных
}

// Очистка сетевых ресурсов
void network_cleanup(network_context_t *ctx) {
    // Закрытие SSL соединения
    if (ctx->ssl) {
        SSL_shutdown(ctx->ssl);  // Корректное завершение SSL
        SSL_free(ctx->ssl);  // Освобождение SSL структуры
        ctx->ssl = NULL;
    }

    // Закрытие сокетов
    // Закрытие сокета соединения, если он был открыт
    if (ctx->connected_socket >= 0)
        close(ctx->connected_socket);
    // Закрытие сокета прослушивания, если он был открыт
    if (ctx->listening_socket >= 0)
        close(ctx->listening_socket);

    // Очистка SSL контекста
    if (ctx->ssl_ctx) {
        SSL_CTX_free(ctx->ssl_ctx);  // Освобождение памяти SSL контекста
        ctx->ssl_ctx = NULL;  // Обнуление указателя
    }

    // Финальная очистка OpenSSL
    cleanup_openssl();
}
