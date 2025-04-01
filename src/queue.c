// queue.c
#include <stdio.h>
#include "queue.h"

// Static array to hold clients who are waiting to be matched
static client_t* queue[MAX_CLIENTS];
// Tracks the number of clients currently in the queue
static int queue_size = 0;

// Static array to hold all active games
static game_t games[MAX_GAMES];
// Tracks the total number of games created
static int game_count = 0;

void init_queue() {
    queue_size = 0;
    game_count = 0;
}
// Adds a client to the matchmaking queue
void add_to_queue(client_t* client) {
    if (queue_size < MAX_CLIENTS) {
        queue[queue_size++] = client; // Add client to the end of the queue
        printf("[+] Added %s to matchmaking queue\n", client->name);
    }
}

// Checks the queue to find two ready players and creates a game if possible
// Returns the index of the newly created game, or -1 if no match was made
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

            // Remove the matched players from the queue by shifting others left
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

            // Log the match
            printf("[+] Matched %s and %s in game %d\n", p1->name, p2->name, game_id);

            game_count++; // Increment total games created
            return game_id; // Return the index of the new game
        }
        i++;
    }

    return -1; // No match found
}

// Returns a pointer to a game struct by index
// Returns NULL if the index is out of bounds
game_t* get_game_by_index(int index) {
    if (index < 0 || index >= game_count) return NULL;
    return &games[index];
}
