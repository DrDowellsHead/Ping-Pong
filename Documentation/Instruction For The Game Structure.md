# 🔐 Детальное объяснение сетевого кода с SSL-шифрованием для игры

## 🎯 Содержание
1.  Что такое SSL/TLS и зачем это нужно?
2.  Ключевые термины OpenSSL
3.  Структура network_context_t (с SSL)
4.  Функции инициализации OpenSSL
5.  Модифицированные сетевые функции с SSL
6.  Процесс SSL-рукопожатия (Handshake)
7.  Как теперь работает обмен данными
8.  Инструкция по сборке и использованию

## 📖 1. Что такое SSL/TLS? Простая аналогия

Раньше ваши игроки общались по незащищенному соединению — это как пересылать **открытки**. Любой почтальон (провайдер, хакер в сети) мог прочитать или изменить сообщение.

**SSL/TLS** — это **бронированный конверт с секретным замком**:
-   **Шифрование:** Сообщение внутри зашифровано
-   **Аутентификация:** Подтверждает личность отправителя
-   **Целостность данных:** Гарантия, что сообщение не изменено

## 🔑 2. Ключевые термины OpenSSL

-   **OpenSSL** — библиотека, реализующая SSL/TLS
-   **SSL_CTX (SSL Context)** — "фабрика по производству замков" (настройки)
-   **SSL (SSL Session)** — "конкретный замок" на одно соединение
-   **SSL Handshake** — "рукопожатие": процесс установки безопасного соединения

## 🏗️ 3. Структура network_context_t (с SSL)

```c
typedef struct {
    int listening_socket;  // Сокет для прослушивания (как "телефон")
    int connected_socket;  // Сокет соединения (как "трубка")
    int is_server_mode;    // 1 = сервер, 0 = клиент
    int game_ready;        // 1 = соединение установлено
    SSL_CTX *ssl_ctx;      // Фабрика SSL-замков (контекст)
    SSL *ssl;              // Конкретный SSL-замок соединения
} network_context_t;
```

# ⚙️ 4. Функции инициализации OpenSSL
**network_ssl.h**

```c
#ifndef NETWORK_SSL_H
#define NETWORK_SSL_H

#include <openssl/err.h> // Функции обработки ошибок OpenSSL
#include <openssl/ssl.h> // Основные функции SSL/TLS
#include <stdlib.h>

#include "game.h"

#define PORT 8080         // Номер порта для соединения
#define BUFFER_SIZE 1024  // Размер буфера для данных
#define MAX_IP_LENGTH 16  // Максимальная длина IP-адреса

// Структура для хранения сетевого контекста с SSL
typedef struct {
    int listening_socket;  // Сокет для прослушивания входящих соединений
    int connected_socket;  // Сокет для установленного соединения
    int is_server_mode;    // Режим работы: 1-сервер, 0-клиент
    int game_ready;        // Флаг готовности: 1-соединение установлено
    SSL_CTX *ssl_ctx;      // Контекст SSL (содержит настройки шифрования)
    SSL *ssl;              // SSL сессия (для конкретного соединения)
} network_context_t;

// Прототипы функций для работы с SSL
int init_openssl();                    // Инициализация библиотеки OpenSSL
SSL_CTX *create_ssl_context();         // Создание SSL контекста
void cleanup_openssl();                // Очистка ресурсов OpenSSL

// Прототипы сетевых функций
void network_init(network_context_t *ctx);                 // Инициализация
void network_get_local_ip(char *buffer, size_t buffer_size); // Получение IP
void network_start_listener(network_context_t *ctx);       // Запуск сервера
void network_connect_to_peer(network_context_t *ctx,       // Подключение
                             const char *peer_ip);         
void network_send_game_state(const network_context_t *ctx, // Отправка данных
                             const game_state_t *state);    
int network_receive_game_state(const network_context_t *ctx, // Получение данных
                               game_state_t *state);        
void network_cleanup(network_context_t *ctx);              // Очистка ресурсов

#endif
```

**network_ssl.c (часть 1 - инициализация)**

```c
#include "network_ssl.h"
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

// Инициализация библиотеки OpenSSL (подготовка инструментов)
int init_openssl() {
    OpenSSL_add_all_algorithms();  // Загрузка всех алгоритмов шифрования
    SSL_load_error_strings();      // Загрузка строк ошибок SSL
    return SSL_library_init();     // Инициализация библиотеки SSL
}

// Создание и настройка фабрики SSL-замков (SSL Context)
SSL_CTX *create_ssl_context() {
    const SSL_METHOD *method; // Метод (протокол) шифрования
    SSL_CTX *ctx;             // Контекст SSL (фабрика замков)

    // Выбираем метод шифрования - TLS для сервера
    method = TLS_server_method(); // Универсальный метод для сервера

    // Создаем сам контекст SSL (строим фабрику замков)
    ctx = SSL_CTX_new(method);
    if (!ctx) {
        perror("Не удалось создать SSL контекст");
        ERR_print_errors_fp(stderr); // Печать ошибок OpenSSL
        exit(EXIT_FAILURE);
    }

    return ctx; // Возвращаем готовую фабрику SSL-замков
}

// Очистка ресурсов OpenSSL (уборка инструментов)
void cleanup_openssl() {
    EVP_cleanup(); // Выгрузка алгоритмов из памяти
}
```

# 🔧 5. Модифицированные сетевые функции с SSL
**Инициализация сети с SSL**

```c
// Инициализация сетевого контекста с поддержкой SSL
void network_init(network_context_t *ctx) {
    // Инициализация обычных сетевых полей
    ctx->listening_socket = -1;  // -1 = "сокет не инициализирован"
    ctx->connected_socket = -1;  // -1 = "соединение не установлено"
    ctx->is_server_mode = 0;     // По умолчанию режим клиента
    ctx->game_ready = 0;         // Игра не готова к началу

    // ИНИЦИАЛИЗАЦИЯ OPENSSL
    if (!init_openssl()) {
        fprintf(stderr, "Ошибка инициализации OpenSSL\n");
        exit(EXIT_FAILURE);
    }

    // СОЗДАНИЕ SSL КОНТЕКСТА (фабрики замков)
    ctx->ssl_ctx = create_ssl_context();
    ctx->ssl = NULL; // Пока SSL сессия не создана
}
```

**Функция запуска сервера с SSL**

```c
// Вспомогательная функция для потока прослушивания с SSL
static void *listener_thread(void *arg) {
    network_context_t *ctx = (network_context_t *)arg;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // СОЗДАНИЕ И НАСТРОЙКА СОКЕТА (как в обычной версии)
    if ((ctx->listening_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Настройка сокета для повторного использования адреса
    if (setsockopt(ctx->listening_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        close(ctx->listening_socket);
        exit(EXIT_FAILURE);
    }

    // Привязка сокета к адресу и порту
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(ctx->listening_socket, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(ctx->listening_socket);
        exit(EXIT_FAILURE);
    }

    // Начало прослушивания входящих подключений
    if (listen(ctx->listening_socket, 3) < 0) {
        perror("listen");
        close(ctx->listening_socket);
        exit(EXIT_FAILURE);
    }

    // Принятие входящего подключения
    if ((ctx->connected_socket = accept(ctx->listening_socket, 
                                      (struct sockaddr *)&address,
                                      (socklen_t *)&addrlen)) < 0) {
        perror("accept");
        close(ctx->listening_socket);
        exit(EXIT_FAILURE);
    }

    // ##### НАЧАЛО SSL-МАГИИ #####
    // СОЗДАНИЕ НОВОЙ SSL СЕССИИ (конкретный замок)
    ctx->ssl = SSL_new(ctx->ssl_ctx);

    // ПРИВЯЗКА SSL СЕССИИ К СОКЕТУ (вешаем замок на телефонную трубку)
    SSL_set_fd(ctx->ssl, ctx->connected_socket);

    // ВЫПОЛНЕНИЕ SSL-РУКОПОЖАТИЯ (установка безопасного соединения)
    // Серверная сторона: принимаем и обрабатываем handshake
    if (SSL_accept(ctx->ssl) <= 0) {
        // Если рукопожатие не удалось - выводим ошибки
        ERR_print_errors_fp(stderr);
        close(ctx->connected_socket);
        close(ctx->listening_socket);
        exit(EXIT_FAILURE);
    }
    // ##### SSL-СОЕДИНЕНИЕ УСТАНОВЛЕНО #####

    ctx->is_server_mode = 1;
    ctx->game_ready = 1; // Игра готова к началу (с шифрованием!)

    return NULL;
}

// Запуск прослушивающего потока
void network_start_listener(network_context_t *ctx) {
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, listener_thread, ctx);
    pthread_detach(thread_id);
}
```
**Функция подключения клиента с SSL**

```c
// Подключение к другому пиру с SSL
void network_connect_to_peer(network_context_t *ctx, const char *peer_ip) {
    struct sockaddr_in serv_addr;

    // СОЗДАНИЕ СОКЕТА (как в обычной версии)
    if ((ctx->connected_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Ошибка создания сокета");
        exit(EXIT_FAILURE);
    }

    // Заполнение структуры адреса сервера
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Преобразование текстового IP в бинарный формат
    if (inet_pton(AF_INET, peer_ip, &serv_addr.sin_addr) <= 0) {
        printf("\nНеверный адрес: %s", peer_ip);
        close(ctx->connected_socket);
        exit(EXIT_FAILURE);
    }

    // Установка соединения с сервером
    if (connect(ctx->connected_socket, (struct sockaddr *)&serv_addr,
                sizeof(serv_addr)) < 0) {
        printf("\nОшибка подключения к %s", peer_ip);
        close(ctx->connected_socket);
        exit(EXIT_FAILURE);
    }

    // ##### НАЧАЛО SSL-МАГИИ НА КЛИЕНТЕ #####
    // СОЗДАНИЕ НОВОЙ SSL СЕССИИ (конкретный замок)
    ctx->ssl = SSL_new(ctx->ssl_ctx);

    // ПРИВЯЗКА SSL СЕССИИ К СОКЕТУ
    SSL_set_fd(ctx->ssl, ctx->connected_socket);

    // ВЫПОЛНЕНИЕ SSL-РУКОПОЖАТИЯ (клиентская сторона)
    // Клиент инициирует handshake с сервером
    if (SSL_connect(ctx->ssl) <= 0) {
        ERR_print_errors_fp(stderr);
        close(ctx->connected_socket);
        exit(EXIT_FAILURE);
    }
    // ##### SSL-СОЕДИНЕНИЕ УСТАНОВЛЕНО #####

    ctx->game_ready = 1; // Соединение установлено и защищено!
}
```

**Функции отправки и получения данных с SSL**

```c
// Отправка состояния игры через защищенное соединение
void network_send_game_state(const network_context_t *ctx,
                             const game_state_t *state) {
    char buffer[BUFFER_SIZE];

    // Форматирование состояния игры в строку
    snprintf(buffer, BUFFER_SIZE, "%d, %d, %d, %d, %d, %d, %d", 
             state->lRacketY, state->rRacketY, state->ballX, 
             state->ballY, state->lScore, state->rScore, state->command);

    // ОТПРАВКА ДАННЫХ ЧЕРЕЗ SSL
    // SSL_write автоматически ШИФРУЕТ данные перед отправкой
    SSL_write(ctx->ssl, buffer, strlen(buffer));
}

// Получение состояния игры через защищенное соединение
int network_receive_game_state(const network_context_t *ctx,
                               game_state_t *state) {
    char buffer[BUFFER_SIZE] = {0};

    // ЧТЕНИЕ ДАННЫХ ЧЕРЕЗ SSL
    // SSL_read автоматически РАСШИФРОВЫВАЕТ полученные данные
    int bytes_received = SSL_read(ctx->ssl, buffer, BUFFER_SIZE - 1);

    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';
        // Преобразование строки обратно в структуру состояния
        sscanf(buffer, "%d, %d, %d, %d, %d, %d, %d", 
               &state->lRacketY, &state->rRacketY, &state->ballX, 
               &state->ballY, &state->lScore, &state->rScore, &state->command);
        return 1; // Данные успешно получены и расшифрованы
    }
    return 0; // Ошибка получения данных
}
```

**Функция очистки ресурсов с SSL**

```c
// Очистка сетевых и SSL ресурсов
void network_cleanup(network_context_t *ctx) {
    // Закрытие SSL сессии (корректное завершение шифрования)
    if (ctx->ssl) {
        SSL_shutdown(ctx->ssl); // Вежливое завершение SSL соединения
        SSL_free(ctx->ssl);     // Освобождение памяти SSL сессии
        ctx->ssl = NULL;
    }

    // Закрытие сокетов
    if (ctx->connected_socket >= 0)
        close(ctx->connected_socket);
    if (ctx->listening_socket >= 0)
        close(ctx->listening_socket);

    // Очистка SSL контекста
    if (ctx->ssl_ctx) {
        SSL_CTX_free(ctx->ssl_ctx);
        ctx->ssl_ctx = NULL;
    }

    // Финальная очистка OpenSSL
    cleanup_openssl();
}
```

**🤝 6. Процесс SSL-рукопожатия (Handshake)**

Клиент                 Сервер
   |                     |
   |--- Client Hello --->|  • Клиент предлагает методы шифрования
   |                     |  • Отправляет случайное число
   |                     |
   |<-- Server Hello ----|  • Сервер выбирает метод шифрования
   |<-- Certificate -----|  • Сервер отправляет сертификат (если есть)
   |<-- Server Done -----|  • Сервер завершает свою часть
   |                     |
   |--- Client Key ----> |  • Клиент генерирует ключ сессии
   |    Exchange         |  • Шифрует его публичным ключом сервера
   |                     |
   |--- Change Cipher -->|  • Клиент: "Теперь используем шифрование!"
   |       Spec          |
   |                     |
   |<-- Change Cipher ---|  • Сервер: "Согласен, используем шифрование!"
   |       Spec          |
   |                     |
   |--- Encrypted ------>|  • Первое зашифрованное сообщение
   |      Handshake      |
   |                     |
   |<-- Encrypted ------ |  • Ответ сервера (зашифрованный)
   |      Handshake      |
   |                     |
   |== Secure Channel == |  • Канал защищен, можно обмениваться данными
   
   
# 🔄 7. Как теперь работает обмен данными
1. Установка соединения:

    - Обычное TCP-соединение (как раньше)

    - SSL-рукопожатие поверх TCP

    - Создание защищенного канала

2. Отправка данных:

    - SSL_write() автоматически шифрует данные

    - Зашифрованные данные отправляются по сети

    - Противник видит только "мусор"

3. Получение данных:

    - SSL_read() автоматически расшифровывает данные

    - Вы получаете чистые, проверенные данные
