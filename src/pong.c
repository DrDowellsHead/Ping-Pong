#include <stdio.h>

void display(int lRacketY, int rRacketY, int ballX, int ballY, int lScore, int rScore);
int collision(int lRacketY, int rRacketY, int ballX, int ballY, int direction);

int main() {
    int lScore, rScore = 0;
    int lRacketY = 13;  // start possition of left racket
    int rRacketY = 13;  // start possition of right racket
    int ballX = 4;      // start possition of ball
    int ballY = 13;     // start possition of ball
    int player = 1;     // left player starts "1" - left|| "-1" - right
    int serve = 1;      // подача

    while (lScore < 21 && rScore < 21) {
        display(lRacketY, rRacketY, ballX, ballY, lScore, rScore);

        char command = 0;
        scanf("%c\n",&command);

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

void display(int lRacketY, int rRacketY, int ballX, int ballY, int lScore, int rScore) {}

int collision(int lRacketY, int rRacketY, int ballX, int ballY, int direction) { return 0; }