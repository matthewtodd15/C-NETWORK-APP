#ifndef MATCHMAKING_H
#define MATCHMAKING_H

#include "structs.h"

void matchmaking_init();
void handle_player_ready(int fd);
void handle_disconnect(int fd);

#endif // MATCHMAKING_H