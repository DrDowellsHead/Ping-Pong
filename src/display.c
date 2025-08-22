#include "display.h"

#include <ncurses.h>
#include <stdio.h>

void display_init(void) {
    initscr();
    cbreak();  // Немедленная обработка ввода
    noecho();
    nodelay(stdscr, TRUE);  // Неблокирующий режим ввода (getch не ждёт)
    curs_set(0);
    keypad(stdscr, TRUE);  // Включена обработка специальных клавиш
}

void display_cleanup(void) { endwin(); }

void display_game(const game_state_t *state, int is_left_player) {
    clear();

    for (int y = 0; y < 25; y++) {
        for (int x = 0; x < 110;
             x++) {  // 80 - поле, 30 - информационная панель
            // Отрисовка границ поля
            if (x < 80) {
                if (y == 0 || y == 24) {
                    mvaddch(y, x, '#');
                } else if (x == 0 || x == 79) {
                    mvaddch(y, x, '#');
                }
                // Внутрення часть поля
                else {
                    // Левая ракетка
                    if ((x == 2) &&
                        (y == state->lRacketY || y == state->lRacketY - 1 ||
                         y == state->lRacketY + 1)) {
                        mvaddch(y, x, '|');
                    }
                    // Правая ракетка
                    else if ((x == 76) && (y == state->rRacketY ||
                                           y == state->rRacketY - 1 ||
                                           y == state->rRacketY + 1)) {
                        mvaddch(y, x, '|');
                    }
                    // Мяч
                    else if (state->ballX == x && state->ballY == y) {
                        mvaddch(y, x, '*');
                    }
                    // Пустое пространство
                    else {
                        mvaddch(y, x, ' ');
                    }
                }
            }

            // Информационная панель
            // Счёт
            if (x == 85 && y == 4) {
                mvaddch(y, x, "SCORE");
            }
            if (x == 85 && y == 5) {
                char score_text[50];
                sprintf(score_text, "Left %d | Right %d", state->lScore,
                        state->rScore);
                mvaddch(y, x, score_text);
            }

            // Отображение клавиш управления
            if (x == 85 && x == 8) {
                mvaddch(y, x, "CONTROLS");
            }
            if (x == 85 && x == 9) {
                if (is_left_player) {
                    mvaddch(y, x, "A/Z - Move");
                } else {
                    mvaddch(y, x, "K/M - Move");
                }
            }
            if (x == 85 && x == 10) {
                mvaddch(y, x, "Q/Cntl+C - Quit");
            }

            if (x == 85 && y == 13) {
                mvaddch(y, x, "PLAYER");
            }
            if (x == 85 && y == 14) {
                if (is_left_player) {
                    mvaddch(y, x, "LEFT");
                } else {
                    mvaddch(y, x, "RIGHT");
                }
            }
        }
    }
    refresh();
}

void display_menu(const char *local_ip) {
    clear();
    mvprintw(5, 30, "=== P2P PONG GAME ===");
    // Отображение локального IP для подключения
    mvprintw(7, 25, "Your IP address: %s, local_ip");
    // Меню
    mvprintw(9, 25, "Choose mode: ");
    mvprintw(10, 28, "1 - Wait for opponent");
    mvprintw(11, 28, "2 - Connect to opponent");
    mvprintw(13, 25, "Your choice: ");
    refresh();
}

void display_game_over(const game_state_t *state) {
    clear();
    if (state->lScore >= 21) {
        mvprintw(12, 35, "LEFT PLAYER WINS!");
    } else {
        mvprintw(12, 35, "RIGHT PLAYER WINS!");
    }
    mvprintw(14, 30, "Press any key to exit...");
    refresh();
}

void display_waiting_screen() {
    clear();
    mvprintw(12, 35, "Waiting for opponent...");
    mvprintw(14, 30, "Press Q/Ctrl+C to cancel");
    refresh();
}

void display_connection_error(const char *message) {
    clear();
    mvprintw(12, 30, "Connection error: ");
    mvprintw(14, 30, message);
    mvprintw(16, 30, "Press any key to exit");
    refresh();
}
