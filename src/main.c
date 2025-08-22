#include <ncurses.h>
#include <unistd.h>

#include "display.h"
#include "game.h"
#include "network.h"

int main() {
    game_state_t local_state;  // Состояние компьютера игрока
    game_state_t remote_state;  // Состояние пира

    int velX = 1;  // 1 пиксель за кадр
    int velY = 1;

    // Сетевой контекст
    network_context_t network_ctx;

    char local_ip[MAX_IP_LENGTH];
    char peer_ip[MAX_IP_LENGTH];

    display_init();           // инициализация ncurses
    game_init(&local_state);  // Инициализация состояния главного компьютера
    game_init(&remote_state);  // Инициализация состояния пира
    network_init(&network_ctx);  // Инициализация сетевого контекста

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
            if (cancel == 'Q' || cancel == 'q')
                ;
            return 0;
        }
        usleep(100000);
    } else {
        // Подключение к пиру
        echo();  // Временное включение отображения ввода
        mvprintw(15, 25, "Enter opponent IP: ");
        refresh();

        // Чтение IP пира
        getnstr(peer_ip, MAX_IP_LENGTH - 1);
        noecho();

        // Попытка подключения
        network_connect_to_peer(&network_ctx, peer_ip);
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
                    goto cleanup
            }
        } else {
            // Правый игрок управляет ракеткой
            switch (command) {
                case 'K':
                case 'k':  // Движение вверх
                    if (local_state.rRacketY <= 3)
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
        network_send_game_state(&network_ctx, &local_state);

        // Получение состояние пира
        if (network_receive_game_state(&network_ctx, &remote_state)) {
            // Синхронизация состояний
            if (is_left_player) {
                // У левого игрока обновляется только правая ракетка и счёт
                local_state.rRacketY = remote_state.rRacketY;
                local_state.rScore = remote_state.rScore;
            } else {
                local_state.lRacketY = remote_state.lRacketY;
                local_state.lScore = remote_state.lScore;
            }
        }

        // Коллизия мяча обрабатывается только левым игроком для синхронизации
        if (is_left_player) {
            game_update_ball(&local_state, &velX, &velY);
        }

        // Задержка для плавности анимации (50 FPS ~ 20ms на кадр)
        usleep(20000);
    }

    display_game_over(&local_state);
    nodelay(stdscr, FALSE);
    getch();

cleanup:
    display_cleanup();
    network_cleanup(&network_ctx);

    return 0;
}