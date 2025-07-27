#include <stdio.h>

#define HEIGHT 25
#define WIDTH 80

void display(int lRacketY, int rRacketY, int ballX, int ballY, int lScore, int rScore);
int collision(int lRacketY, int rRacketY, int ballX, int ballY);

int main() {
    int lScore = 0, rScore = 0;
    int lRacketY = 13;  // start possition of left racket
    int rRacketY = 13;  // start possition of right racket
    int ballX = 3;      // start possition of ball
    int ballY = 13;     // start possition of ball
    int velX = 1;
    int velY = 1;

    while (lScore < 21 && rScore < 21) {
        printf("\033[H\033[2J");
        display(lRacketY, rRacketY, ballX, ballY, lScore, rScore);

        char command = 0;
        command = getchar();
        getchar();

        switch (command) {
            case 'A':
                if (lRacketY >= 3) {
                    lRacketY--;
                }
                break;
            case 'Z':
                if (lRacketY <= 21) {
                    lRacketY++;
                }
                break;
            case 'K':
                if (rRacketY >= 3) {
                    rRacketY--;
                }
                break;
            case 'M':
                if (rRacketY <= 21) {
                    rRacketY++;
                }
                break;
            case ' ':
                break;
            default:
                printf("Wrong command!!!\n");
                break;
        }

        int col = collision(lRacketY, rRacketY, ballX, ballY);
        switch (col) {
            case 0:
                ballX += velX;
                ballY += velY;
                break;
            case 1:  // left
                velX = 1;
                if (velY == 0) {
                    velY = 1;
                }
                ballX += velX;
                ballY += velY;
                break;
            case 2:  // top
                velY = 1;
                ballX += velX;
                ballY += velY;
                break;
            case 3:  // right
                velX = -1;
                if (velY == 0) {
                    velY = -1;
                }
                ballX += velX;
                ballY += velY;
                break;
            case 4:  // bottom
                velY = -1;
                ballX += velX;
                ballY += velY;
                break;
            case 5:  // left center
                velX = 1;
                velY = 0;
                ballX += velX;
                ballY += velY;
                break;
            case 6:  // right center
                velX = -1;
                velY = 0;
                ballX += velX;
                ballY += velY;
                break;
            case 7:  // left win
                rScore++;
                lRacketY = 13;
                rRacketY = 13;
                ballX = 3;
                ballY = 13;
                break;
            case 8:  // right win
                lScore++;
                lRacketY = 13;
                rRacketY = 13;
                ballX = 75;
                ballY = 13;
                break;
            default:
                break;
        }
    }

    if (lScore == 21) {
        // LEFT WIN
        printf("LEFT WIN\n");
    } else {
        // RIGHT WIN
        printf("RIGHT WIN\n");
    }

    return 0;
}

void display(int lRacketY, int rRacketY, int ballX, int ballY, int lScore, int rScore) {
    for (int y = 0; y < 25; y++) {
        for (int x = 0; x < 110; x++) {
            if (x < 80) {
                if (y == 0 || y == 24) {
                    printf("#");
                } else {
                    if (x == 0 || x == 79) {
                        printf("#");
                    } else if (((x == 2) && (y == lRacketY || y == lRacketY - 1 || y == lRacketY + 1)) ^
                               ((x == 76) && (y == rRacketY || y == rRacketY - 1 || y == rRacketY + 1))) {
                        printf("|");
                        continue;

                    } else if (ballX == x && ballY == y) {
                        printf("*");
                        continue;
                    }
                    printf(" ");
                }
            }
            if (x == 109 && y == 4) {
                printf("\t      SCORE");
            }
            if (x == 109 && y == 5) {
                printf("\t|Left %d | Right %d|", lScore, rScore);
            }
        }
        printf("\n");
    };
}

int collision(int lRacketY, int rRacketY, int ballX, int ballY) {
    // left-racket
    if (ballX == 3 && (lRacketY + 1 == ballY || lRacketY - 1 == ballY)) {
        return 1;  // left coll
    } else if (ballX == 3 && ballY == lRacketY) {
        return 5;  // invert velX
    }
    // riht-racket
    if (ballX == 75 && (rRacketY + 1 == ballY || rRacketY - 1 == ballY)) {
        return 3;  // left coll
    } else if (ballX == 75 && ballY == rRacketY) {
        return 6;  // invert velX
    }
    // top
    if (ballY == 1) {
        return 2;
    }
    // bottom
    if (ballY == 23) {
        return 4;
    }

    // left win
    if (ballX <= 0) {
        return 7;
    }
    // right win
    if (ballX >= 78) {
        return 8;
    }
    return 0;
}
