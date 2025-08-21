#ifndef GAME_H
#define GAME_H

// Структура для информации о столкновении
typedef struct {
    int type; // 0-нет, 1-ракетка, 2-стенка, 3-гол
    int is_center; // 1-попадание в центр, 0-в край
    int is_left; // 1-левая ракетка, 0-правая
} collision_into_t;


// Инициализация структуры для хранения состояний игры
typedef struct {
    int lRacketY;
    int rRacketY;
    int ballX;
    int ballY;
    int lScore;
    int rScore;
    int command;
} game_state_t;

// Опрделение параметров игрового поля, ракеток и условие победы
#define MAX_SCORE 21
#define FIELD_WIDTH 80
#define FIELD_HEIGHT 25
#define RACKET_SIZE 3

void game_init(game_state_t *state);
collision_into_t game_check_collision(const game_state_t *state);
void game_update_ball(game_state_t *state, int *velX, int *velY);
void game_reset_after_score(game_state_t *state, int scorer);
int game_is_over(const game_state_t *state);

#endif
