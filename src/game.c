#include "game.h"

#include <stdlib.h>

void game_init(game_state_t *state) {
    state->lRacketY = 13;
    state->rRacketY = 13;
    state->ballX = 3;
    state->ballY = 13;
    state->lScore = 0;
    state->rScore = 0;
    state->command = 0;
}

collision_into_t game_check_collision(const game_state_t *state) {
    collision_into_t info = {0, 0, 0};

    // Столкновение с левой ракеткой
    if (state->ballX == 2) {
        if (state->ballY >= state->lRacketY - 1 &&
            state->ballY <= state->lRacketY + 1) {
            info.type = 1;     // Тип: ракетка
            info.is_left = 1;  // Левая ракетка
            info.is_center = (state->ballY == state->lRacketY) ? 1 : 0;
            return info;
        } else {
            info.type = 3;
            info.is_left = 1;
            return info;
        }
    }

    // Столкновение с правой ракеткой
    if (state->ballX == 76) {
        if (state->ballY >= state->rRacketY - 1 &&
            state->ballY <= state->rRacketY + 1) {
            info.type = 1;     // Тип: ракетка
            info.is_left = 0;  // Правая ракетка
            info.is_center = (state->ballY == state->rRacketY) ? 1 : 0;
            return info;
        } else {
            info.type = 3;
            info.is_left = 0;
            return info;
        }
    }

    // Стокновение с верхней стенкой (Y = 1)
    if (state->ballY == 1) {
        info.type = 2;  // Тип: стенка
        return info;
    }

    // Стокновение с нижней стенкой (Y = 1)
    if (state->ballY == 23) {
        info.type = 2;  // Тип: стенка
        return info;
    }

    // Мяч ушёл за левую стенку
    if (state->ballX <= 0) {
        info.type = 3;     // Тип: гол
        info.is_left = 1;  // Левая ракетка
        return info;
    }

    // Мяч ушёл за правую стенку
    if (state->ballX >= 79) {
        info.type = 3;     // Тип: гол
        info.is_left = 0;  // Правая ракетка
        return info;
    }

    return info;  // Изменение коллизии отсутствует
}

void game_update_ball(game_state_t *state, int *velX, int *velY) {
    

    state->ballX += *velX;
    state->ballY += *velY;

    collision_into_t collision = game_check_collision(state);
    

    // Обработка коллизии
    switch (collision.type) {
        case 1:  // Столкновение с ракеткой
            if (collision.is_center) {
                *velX =
                    -(*velX);  // При столкновении с ракеткой X всегда меняется
                *velY = 0;
            } else {
                // Удар в край
                *velX = -(*velX);

                if (collision.is_left) {
                    // Удар в верхнюю часть
                    if (state->ballY == state->lRacketY - 1) {
                        *velY = -1;
                        // Удар в нижнюю часть
                    } else if (state->ballY == state->lRacketY + 1) {
                        *velY = 1;
                    }
                    // Правая ракетка
                } else {
                    // Удар в верхнюю часть
                    if (state->ballY == state->rRacketY - 1) {
                        *velY = -1;
                        // Удар в нижнюю часть
                    } else if (state->ballY == state->rRacketY + 1) {
                        *velY = 1;
                    }
                }
            }
            break;

        case 2:  // Столкновение со стенкой
            *velY = -(*velY);
            if (state->ballY == 1) state->ballY = 2;
            if (state->ballY == 23) state->ballY = 22;
            break;
        case 3:  // Гол
            state->ballX -= *velX;
            state->ballY -= *velY;
            game_reset_after_score(state, velX, velY, collision.is_left);
            return;
    }


}

void game_reset_after_score(game_state_t *state, int *velX, int *velY, int is_left_goal) {
    if (is_left_goal) {
        // Гол в левые ворота
        state->rScore++;
        state->ballX = 3;  // Возвращаем мяч левому игроку
        *velX = -1;
    } else {
        // Гол в правые ворота
        state->lScore++;
        state->ballX = 75;  // Возвращаем мяч правому игроку
        *velX = 1;
    }
    state->ballY = 13;
    state->lRacketY = 13;
    state->rRacketY = 13;

    *velY = 0;
}

// Завершение игры при достижении максимального количества очков у одного из
// игроков
int game_is_over(const game_state_t *state) {
    return state->lScore >= MAX_SCORE || state->rScore >= MAX_SCORE;
}