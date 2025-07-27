#include <ncurses.h>
#include <stdio.h>

void display(int lRacketY, int rRacketY, int ballX, int ballY, int lScore, int rScore);
int collision(int lRacketY, int rRacketY, int ballX, int ballY);

int main() {
    initscr();
    cbreak();
    noecho();
    nodelay(stdscr, TRUE);
    curs_set(0);
    keypad(stdscr, TRUE);

    int lScore = 0, rScore = 0;
    int lRacketY = 13;
    int rRacketY = 13;
    int ballX = 3;
    int ballY = 13;
    int velX = 1;
    int velY = 1;

    while (lScore < 21 && rScore < 21) {
        clear();

        display(lRacketY, rRacketY, ballX, ballY, lScore, rScore);

        char command = getch();
        mvaddstr(30, 10, &command);

        switch (command) {
            case 'A':
            case 'a':
                if (lRacketY >= 3) {
                    lRacketY--;
                }
                break;
            case 'Z':
            case 'z':
                if (lRacketY <= 21) {
                    lRacketY++;
                }
                break;
            case 'K':
            case 'k':
                if (rRacketY >= 3) {
                    rRacketY--;
                }
                break;
            case 'M':
            case 'm':
                if (rRacketY <= 21) {
                    rRacketY++;
                }
                break;
            case 'Q':
            case 'q':
                endwin();
                return 0;
            case ERR:
                break;
            default:
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

        refresh();

        for (int i = 0; i < 100000000 / 2000; i++) {
        }
    }

    clear();
    if (lScore == 21) {
        mvprintw(12, 35, "LEFT WIN");
    } else {
        mvprintw(12, 35, "RIGHT WIN");
    }

    mvprintw(14, 30, "Press any key to exit...");
    nodelay(stdscr, FALSE);
    getch();

    endwin();
    return 0;
}

void display(int lRacketY, int rRacketY, int ballX, int ballY, int lScore, int rScore) {
    for (int y = 0; y < 25; y++) {
        for (int x = 0; x < 110; x++) {
            if (x < 80) {
                if (y == 0 || y == 24) {
                    mvaddch(y, x, '#');

                } else if (x == 0 || x == 79) {
                    mvaddch(y, x, '#');
                } else {
                    if (x == 0 || x == 79) {
                        mvaddch(y, x, '#');
                    } else if (((x == 2) && (y == lRacketY || y == lRacketY - 1 || y == lRacketY + 1)) ||
                               ((x == 76) && (y == rRacketY || y == rRacketY - 1 || y == rRacketY + 1))) {
                        mvaddch(y, x, '|');
                        continue;
                    } else if (ballX == x && ballY == y) {
                        mvaddch(y, x, '*');
                        continue;
                    }
                    mvaddch(y, x, ' ');
                }
            }
            if (x == 109 && y == 4) {
                mvaddstr(y, x, "\t      SCORE");
            }
            if (x == 109 && y == 5) {
                char score_text[50];
                sprintf(score_text, "\t|Left %d | Right %d|", lScore, rScore);
                mvaddstr(y, x, score_text);
            }
        }
    }
}

int collision(int lRacketY, int rRacketY, int ballX, int ballY) {
    // left-racket
    if (ballX == 3 && (lRacketY + 1 == ballY || lRacketY - 1 == ballY)) {
        return 1;  // left coll
    } else if (ballX == 3 && ballY == lRacketY) {
        return 5;  // invert velX
    }
    // right-racket
    if (ballX == 75 && (rRacketY + 1 == ballY || rRacketY - 1 == ballY)) {
        return 3;  // right coll
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