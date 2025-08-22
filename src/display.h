#ifndef DISPLAY_H
#define DISPLAY_H

#include "game.h"

void display_init(void);
void display_cleanup(void);
void display_game(const game_state_t *state, int is_left_player);
void display_menu(const char *local_ip);
void display_game_over(const game_state_t *state);
void display_waiting_screen();
void display_connection_error(const char *message);

#endif