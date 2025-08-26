#define _POSIX_C_SOURCE 199309L

#include <ctype.h>
#include <ncurses.h>
#include <time.h>
#include <unistd.h>

#include "display.h"
#include "game.h"
#include "network_ssl.h"

void msleep(int milliseconds) {
    struct timespec ts = {.tv_sec = milliseconds / 1000,
                          .tv_nsec = (milliseconds % 1000) * 1000000};
    nanosleep(&ts, NULL);
}

int main() {
    game_state_t local_state;  // Состояние компьютера игрока
    game_state_t remote_state;  // Состояние пира

    ball_velocity_t ball_vel = {1, 1};  // 1 пиксель за кадр по X и Y

    // Сетевой контекст
    network_context_t network_ctx;

    char local_ip[MAX_IP_LENGTH];
    char peer_ip[MAX_IP_LENGTH] = {0};

    display_init();           // инициализация ncurses
    game_init(&local_state);  // Инициализация состояния главного компьютера
    game_init(&remote_state);  // Инициализация состояния пира
    network_init(&network_ctx);  // Инициализация сетевого контекста

    network_get_local_ip(local_ip, sizeof(local_ip));
    display_menu(local_ip);

    // Выбор режима игры
    char choice = getch();
    while (choice != '1' && choice != '2') {
        choice = getch();
    }

    // Определение позиции игрока
    int is_left_player = (choice == '1');

    if (is_left_player) {
        // Ожидание пира
        display_waiting_screen();
        network_start_listener(&network_ctx);

        while (!network_ctx.game_ready) {
            char cancel = getch();
            if (cancel == 'Q' || cancel == 'q') {
                display_cleanup();
                network_cleanup(&network_ctx);
                return 0;
            }
            msleep(20);
        }

    } else {
        // Подключение к пиру
        nodelay(stdscr, FALSE);
        echo();  // Временное включение отображения ввода

        int ch;
        while ((ch = getch()) != '\n' && ch != ERR);

        mvprintw(15, 25, "Enter opponent IP: ");
        refresh();

        // Ввод IP
        char ip_input[MAX_IP_LENGTH] = {0};
        getnstr(ip_input, MAX_IP_LENGTH - 1);

        strncpy(peer_ip, ip_input, MAX_IP_LENGTH - 1);
        peer_ip[MAX_IP_LENGTH - 1] = '\0';

        noecho();
        nodelay(stdscr, TRUE);

        // Попытка подключения
        if (strlen(peer_ip) > 0) {
            network_connect_to_peer(&network_ctx, peer_ip);

            if (!network_ctx.game_ready) {
                mvprintw(17, 25, "Connection failed! Press any key to exit");
                refresh();
                msleep(3000);
                getch();
                goto cleanup;
            }
        } else {
            mvprintw(17, 25,
                     "IP address cannot be empty! Press any key to exit");
            refresh();
            msleep(3000);
            getch();
            goto cleanup;
        }
    }

    // Главный игровой цикл
    while (!game_is_over(&local_state)) {
        display_game(&local_state, is_left_player);

        char command = getch();
        local_state.command = 0;

        // Обработка управления
        if (is_left_player) {
            // Левый игрок управляет ракеткой
            switch (command) {
                case 'A':
                case 'a':  // Движение вверх
                    if (local_state.lRacketY >= 3)
                        local_state.lRacketY--;
                    local_state.command = 1;
                    break;
                case 'Z':
                case 'z':  // Движение вниз
                    if (local_state.lRacketY <= 21)
                        local_state.lRacketY++;
                    local_state.command = 2;
                    break;
                case 'Q':
                case 'q':  // Выход
                    goto cleanup;
            }
        } else {
            // Правый игрок управляет ракеткой
            switch (command) {
                case 'K':
                case 'k':  // Движение вверх
                    if (local_state.rRacketY >= 3)
                        local_state.rRacketY--;
                    local_state.command = 1;
                    break;
                case 'M':
                case 'm':  // Движение вниз
                    if (local_state.rRacketY <= 21)
                        local_state.rRacketY++;
                    local_state.command = 2;
                    break;
                case 'Q':
                case 'q':  // Выход
                    goto cleanup;
            }
        }

        // Сетевой обмен данными
        // network_send_game_state(&network_ctx, &local_state, &ball_vel);

        // if (is_left_player) {
        //     game_update_ball(&local_state, &ball_vel.velX, &ball_vel.velY);
        // }



        // // Получение состояние пира
        // if (network_receive_game_state(&network_ctx, &remote_state, &ball_vel)) {
        //     // Синхронизация состояний
        //     if (is_left_player) {
        //         // У левого игрока обновляется только правая ракетка и счёт
        //         local_state.rRacketY = remote_state.rRacketY;
        //         local_state.rScore = remote_state.rScore;
        //     } else {
        //         local_state.lRacketY = remote_state.lRacketY;
        //         local_state.lScore = remote_state.lScore;
        //     }
        // }

        // Коллизия мяча обрабатывается только на сервере для синхронизации
        if (is_left_player) {
            game_update_ball(&local_state, &ball_vel.velX, &ball_vel.velY);
        }

        // Сетевой обмен данными
        network_send_game_state(&network_ctx, &local_state, &ball_vel);

        // Получение velocity из функции приёма
        // if (network_receive_game_state(&network_ctx, &remote_state,
        //                                &ball_vel)) {
            if (is_left_player) {
                ball_velocity_t ignore_vel;
                if(network_receive_game_state(&network_ctx, &remote_state, &ignore_vel)) {
                    // Сервер получет положение правой ракетки
                    local_state.rRacketY = remote_state.rRacketY;
                    local_state.rScore = remote_state.rScore;  // Синхронизация счёта
                }
            } else {
                // Клиент получает все состояния от сервера
                if (network_receive_game_state(&network_ctx, &remote_state, &ball_vel)) {
                    local_state.lRacketY = remote_state.lRacketY;
                    local_state.rRacketY = remote_state.rRacketY;
                    local_state.ballX = remote_state.ballX;
                    local_state.ballY = remote_state.ballY;
                    local_state.lScore = remote_state.lScore;
                    // local_state.rScore = remote_state.rScore;
                }
            }
        // }

        // Задержка для плавности анимации (50 FPS ~ 20ms на кадр)
        msleep(40);
    }

    display_game_over(&local_state);
    nodelay(stdscr, FALSE);
    getch();

cleanup:
    display_cleanup();
    network_cleanup(&network_ctx);

    return 0;
}