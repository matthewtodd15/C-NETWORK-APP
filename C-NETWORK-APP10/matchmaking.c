#include "matchmaking.h"
#include "game.h"
#include "defs.h"
#include <string.h>
#include <stdlib.h>

#define MAX_QUEUE_SIZE 128

static int matchmaking_queue[MAX_QUEUE_SIZE];
static int front = 0;
static int rear = 0;
static int count = 0;

extern client clients[MAX_CLIENTS];
extern server_ctx server;

void matchmaking_init() {
    front = rear = count = 0;
    for (int i = 0; i < MAX_QUEUE_SIZE; ++i) {
        matchmaking_queue[i] = -1;
    }
}

void enqueue_client(int fd) {
    if (count < MAX_QUEUE_SIZE) {
        matchmaking_queue[rear] = fd;
        clients[fd].is_queued = 1;
        rear = (rear + 1) % MAX_QUEUE_SIZE;
        count++;
    }
}

int dequeue_client() {
    if (count == 0) return -1;
    int fd = matchmaking_queue[front];
    front = (front + 1) % MAX_QUEUE_SIZE;
    count--;
    clients[fd].is_queued = 0;
    return fd;
}


void handle_player_ready(int fd) {
    clients[fd].is_ready = 1;
    if (count == 0) {
        enqueue_client(fd);
    } else {
        int opponent_fd = dequeue_client();
        if (opponent_fd == -1 || clients[opponent_fd].is_ready == 0) {
            enqueue_client(fd);
        } else {
            int game_id = get_open_game_slot();
            if (game_id == -1) return;
            game_data *new_game = malloc(sizeof(game_data));
            memset(new_game, 0, sizeof(game_data));
            new_game->players[0] = &clients[opponent_fd];
            new_game->players[1] = &clients[fd];
            new_game->player_turn = 0;
            new_game->winner = -1;
            server.games[game_id] = new_game;
        }
    }
}

void handle_disconnect(int fd) {
    clients[fd].is_ready = 0;
    clients[fd].is_queued = 0;

    int tmp_queue[MAX_QUEUE_SIZE];
    int tmp_count = 0;

    for (int i = 0; i < count; ++i) {
        int idx = (front + i) % MAX_QUEUE_SIZE;
        if (matchmaking_queue[idx] != fd) {
            tmp_queue[tmp_count++] = matchmaking_queue[idx];
        }
    }

    memcpy(matchmaking_queue, tmp_queue, sizeof(int) * tmp_count);
    front = 0;
    rear = tmp_count;
    count = tmp_count;
}