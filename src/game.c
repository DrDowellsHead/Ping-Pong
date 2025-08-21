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

    // Стокнеовение с левой ракеткой
    if (state->ballX == 3) {
        info.type = 1;     // Тип: ракетка
        info.is_left = 1;  // Левая ракетка

        if (state->ballY == state->lRacketY) {
            info.is_center = 1;  // Попадание в центр
        } else if (state->ballY == state->lRacketY - 1 ||
                   state->ballY == state->lRacketY + 1) {
            info.is_center = 0;  // Мяч попал в край ракетки
        } else {
            info.type = 0;  // Не попали в ракетку
        }

        if (info.type != 0)
            return info;
    }

    // Столкновение с правой ракеткой
    if (state->ballX == 75) {
        info.type = 1;     // Тип: ракетка
        info.is_left = 0;  // Правая ракетка

        if (state->ballX == state->rRacketY) {
            info.is_center = 1;  // Попадание в центр
        } else if (state->ballY == state->rRacketY - 1 ||
                   state->ballY == state->rRacketY + 1) {
            info.is_center = 0;  // Мяч попал в край ракетки
        } else {
            info.type = 0;  // Не попали в ракетку
        }

        if (info.type != 0)
            return info;
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
    if (state->ballX >= 78) {
        info.type = 3;     // Тип: гол
        info.is_left = 0;  // Правая ракетка
        return info;
    }

    return info;  // Изменение коллизии отсутствует
}

void game_update_ball(game_state_t *state, int *velX, int *velY) {
    collision_into_t collision = game_check_collision(state);

    // Обработка коллизии
    switch (collision.type) {
        case 1:                // Столкновение с ракеткой
            *velX = -(*velX);  // При столкновении с ракеткой всегда меняется
                               // координата X

            if (collision.is_center) {
                *velY = 0;  // Попадание в центр - прямой отскок
            } else {
                *velY = -(*velY);  // Попадание по краю - диагональный отскок
            }
            break;

        case 2:  // Столкновение со стенкой
            *velY =
                -(*velY);  // При столкновении со стенкой меняется Y координата
            break;

        case 3:  // Гол
            game_reset_after_score(state, collision.is_left);
            return;
    }

    state->ballX += *velX;
    state->ballY += *velY;
}

void game_reset_after_score(game_state_t *state, int is_left_goal) {
    if (is_left_goal) {
        // Гол в левые ворота
        state->rScore++;
        state->ballX = 3;  // Возвращаем мяч левому игроку
    } else {
        // Гол в правые ворота
        state->lScore++;
        state->ballX = 75;  // Возвращаем мяч правому игроку
    }
    state->ballY = 13;  // Мяч по центру
}

// Завершение игры при достижении максимального количества очков у одного из
// игроков
int game_is_over(const game_state_t *state) {
    return state->lScore >= MAX_SCORE || state->rScore >= MAX_SCORE;
}