// queue.c
#include <stdio.h>
#include "queue.h"

static client_t* queue[MAX_CLIENTS];
static int queue_size = 0;

static game_t games[MAX_GAMES];
static int game_count = 0;

void init_queue() {
    queue_size = 0;
    game_count = 0;
}

void add_to_queue(client_t* client) {
    if (queue_size < MAX_CLIENTS) {
        queue[queue_size++] = client;
        printf("[+] Added %s to matchmaking queue\n", client->name);
    }
}

int check_and_match() {
    if (queue_size < 2)
        return -1;

    // Look for two ready players
    int i = 0;
    while (i < queue_size - 1) {
        if (queue[i]->ready && queue[i + 1]->ready) {
            // Found two ready players
            client_t* p1 = queue[i];
            client_t* p2 = queue[i + 1];

            // Shift queue left
            for (int j = i; j < queue_size - 2; j++) {
                queue[j] = queue[j + 2];
            }
            queue_size -= 2;

            // Create game
            int game_id = game_count;
            games[game_id].id = game_id;
            games[game_id].player1 = p1;
            games[game_id].player2 = p2;
            p1->game_id = game_id;
            p2->game_id = game_id;

            printf("[+] Matched %s and %s in game %d\n", p1->name, p2->name, game_id);

            game_count++;
            return game_id;
        }
        i++;
    }

    return -1;
}

game_t* get_game_by_index(int index) {
    if (index < 0 || index >= game_count) return NULL;
    return &games[index];
}
