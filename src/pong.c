/*
в каждом кейсе обработать столкновение с левой, правой ракетками, верхней, нижней границами и выход за границу поля
в случае выхода левой границы выводи 6
в случае выхода правой границы выводи 7
в случае попадания в середину ракетки выводи 5
направление 1 - вверх-вправо
направление 2 - вниз-вправо
направление 3 - вниз-влево
направление 4 - вверх-влево
*/
#include <stdio.h>

#define HEIGHT 25
#define WIDTH 80

void display(int lRacketY, int rRacketY, int ballX, int ballY, int lScore, int rScore);
int collision(int lRacketY,int rRacketY, int ballX, int ballY, int direction);

int counter(int lScore, int rScore) {
    int i = 0;
    lScore = ++i;
    return 0;
}

// в случае выхода левой границы выводи 6
// в случае выхода правой границы выводи 7
// в случае попадания в середину ракетки выводи 5
// направление 1 - вверх-вправо
// направление 2 - вниз-вправо
// направление 3 - вниз-влево
// направление 4 - вверх-влево
int collision(int lRacketY,int rRacketY, int ballX, int ballY, int direction) {

    // Удары о верхнюю/нижнюю границу
    if (ballY <= 0) { // Удар о верхнюю границу
        switch (direction) {
            case 1: return 2; // Мяч летит вправо
            break;
            case 3: return 4; // Мяч летит влево
            break;
        default:
            break;
        }
    }

    if (ballY == WIDTH - 1) { // Удар о нижнюю границу
        switch (direction) {
        case 2: return 1; // Мяч летит вправо
            break;
        case 3: return 4; // Мяч летит влево
            break;
        }
    
    // Выход за правую границу
    if (ballX == WIDTH - 2) { // Выход за правую границу
        return 7;
    }

    // Выход за левую границу
    if (ballX == 1) {
        return 6;
    }

    // Удар о правую ракетку
    if (ballY == rRacketY - 1) { // Касание по верхней границей
        switch (direction) {
        case 1: return 4; // Мяч летит вверх
            break;
        case 2: return 3; // Мяч летит вниз
            break;
        }
    }

    if (ballY == rRacketY || ballY == lRacketY) { // Касание по центру
        switch (direction) {
        return 5;
            break;
        }
    }
        
    // Удар по левой ракетке
    if (ballY == lRacketY - 1) { // Касание по верхней границей
        switch (direction) {
        case 4: return 1; // Мяч летит вверх
            break;
            case 3: return 2; // Мяч летит вниз
            break;
        }
    }

    if (ballY == lRacketY + 1) { // Касание по верхней границей
        switch (direction) {
        case 4: return 1; // Мяч летит вверх
            break;
        case 3: return 2; // Мяч летит вниз
    }
    return direction;
    }
}