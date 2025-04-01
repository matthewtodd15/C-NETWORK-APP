#ifndef MATCHMAKER_H
#define MATCHMAKER_H

#include "structs.h"

void init_matchmaker();
void add_ready_client(client_t* client, game_t* games, int max_games);
void cleanup_matchmaker();

#endif
