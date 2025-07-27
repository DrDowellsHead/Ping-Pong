#include <stdio.h>

#define HEIGHT 25
#define WIDTH 80

void display(int lRacketY, int rRacketY, int ballX, int ballY, int lScore, int rScore);
int collision(int lRacketY, int rRacketY, int ballX, int ballY, int direction);

int main() {
    int lScore =0, rScore = 0;
    int lRacketY = 13;  // start possition of left racket
    int rRacketY = 13;  // start possition of right racket
    int ballX = 4;      // start possition of ball
    int ballY = 13;     // start possition of ball
    int player = 1;     // left player starts "1" - left|| "-1" - right
    int serve = 1;      // подача

    while (lScore < 21 && rScore < 21) {
        display(lRacketY, rRacketY, ballX, ballY, lScore, rScore);

        char command = 0;
        command = getchar();
        getchar();

        switch (command) {
            case 'A':
                if (lRacketY >= 2) {
                    lRacketY--;
                }
                break;
            case 'Z':
                if (lRacketY <= 23) {
                    rRacketY++;
                }
                break;
            case 'K':
                if (rRacketY >= 2) {
                    rRacketY--;
                }
                break;
            case 'M':
                if (rRacketY <= 23) {
                    rRacketY++;
                }
                break;
            case ' ':
                break;
            default:
                printf("Wrong command!!!\n");
                break;
        }

        int direction = collision(lRacketY, rRacketY, ballX, ballY, direction);

        switch (direction) {
            case 1:
                ballX++;
                ballY++;
                break;
            case 2:
                ballX++;
                ballY--;
                break;
            case 3:
                ballX--;
                ballY--;
                break;
            case 4:
                ballX--;
                ballY++;
                break;
            case 5:
                break;
            case 6:
                lScore++;
                serve++;
                lRacketY = rRacketY = ballY = 13;
                // передача подачи
                if (serve == 3) {
                    player = -player;
                    serve = 1;
                }
                if (player == 1) {
                    ballX = 4;
                } else {
                    ballX = 76;
                }

                break;
            case 7:
                rScore++;
                serve++;
                lRacketY = rRacketY = ballY = 13;
                // передача подачи
                if (serve == 3) {
                    player = -player;
                    serve = 1;
                }
                if (player == 1) {
                    ballX = 4;
                } else {
                    ballX = 76;
                }
                break;
        }
    }

    if (lScore == 21) {
        // LEFT WIN
        printf("LEFT WIN");
    } else {
        // RIGHT WIN
        printf("RIGHT WIN");
    }

    return 0;
}

void display (int lRacketY,int rRacketY, int ballX, int ballY, int lScore, int rScore) {
    for (int y = 0; y<25; y++){
        for (int x = 0; x<110; x++){
            if (x < 80){
                if (y==0||y==24) {
                    printf("#"); 
                } 
                else {
                    if (x==0 || x==79){
                        printf("#");
                    } 
                    else if ((x == 2 || x == 76) && (y == rRacketY || y == rRacketY-1 || y == rRacketY+1 || y == lRacketY || y == lRacketY-1 || y == lRacketY+1)) {
                            printf("|");
                            continue;
                    }
                    else if (ballX == x && ballY == y) {
                        printf("*");
                        continue;
                    }
                        printf(" ");
                }
            }
            if (x == 109 && y == 4){
                printf("\t      SCORE");
            }
            if (x == 109 && y == 5){
                printf("\t|Left %d | Right %d|", lScore, rScore);
            }
        }
    printf("\n");
    };
}


// int collision(int lRacketY, int rRacketY, int ballX, int ballY, int direction) { return 0; }
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