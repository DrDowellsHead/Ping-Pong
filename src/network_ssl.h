#ifndef NETWORK_SSL_H
#define NETWORK_SSL_H

#include <openssl/err.h> // Функции обработки ошибок OpenSSL
#include <openssl/ssl.h> // Основные функции SSL/TLS
#include <stdlib.h>

#include "game.h"

#define PORT 8080  // Номер порта
#define BUFFER_SIZE 1024  // Размер буфера для сетевых данных
#define MAX_IP_LENGTH 16  // Максимальная длина IP-адреса (15 + "\0")

// Структура для хранения сетевого контекста
typedef struct {
    int listening_socket;  // Сокет прослушивает входящие соединения
    int connected_socket;  // Сокерт для установленного соединения
    int is_server_mode;  // Мод режима: 1-сервер, 0-клиент
    int game_ready;  // Флаг готовности: 1-соединение установлено
    SSL_CTX *ssl_ctx;  // Контекст SSL (содержит настройки шифрования)
    SSL *ssl;          // SSL сессия (для соединения)
} network_context_t;

int init_openssl();             // Инициализация OpenSSL
SSL_CTX *create_ssl_context();  // Создание SSL контекста
void cleanup_openssl();         // Очистка ресурсов OpenSSL
void network_init(network_context_t *ctx);  // Инициализация
void network_get_local_ip(char *buffer, size_t buffer_size);  // Получение IP
void network_start_listener(network_context_t *ctx);  // Запуск сервера
void network_connect_to_peer(network_context_t *ctx,
                             const char *peer_ip);  // Подключение
void network_send_game_state(const network_context_t *ctx,
                             const game_state_t *state);  // Отправка
int network_receive_game_state(const network_context_t *ctx,
                               game_state_t *state);  // Получение
void network_cleanup(network_context_t *ctx);  // Очистка ресурсов

#endif