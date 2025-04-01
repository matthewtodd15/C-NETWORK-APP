// queue.h
#ifndef QUEUE_H
#define QUEUE_H

#include "structs.h"

#define MAX_CLIENTS 100
#define MAX_GAMES 50

void init_queue();
void add_to_queue(client_t* client);
int check_and_match(); // returns game index if match made, else -1
game_t* get_game_by_index(int index);

#endif
