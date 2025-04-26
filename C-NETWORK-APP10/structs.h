#ifndef STRUCTS_H
#define STRUCTS_H

#include "common.h"
#include "defs.h"

// for determining how to handle connection
enum { FD_HTTP, FD_WS } typedef fd_state;

struct {
  char key[MAX_HEADER_KEY_SIZE];
  char value[MAX_HEADER_VAL_SIZE];
} typedef header;

struct {
  char key[MAX_FORM_KEY_SIZE];
  char value[MAX_FORM_VAL_SIZE];
} typedef form_field;

struct {
  char version_line[MAX_VLINE_SIZE];
  header headers[MAX_HEADERS];
  int header_count;
  char *body;
  int body_len;
} typedef http_response;

struct {
  int fd;
  char ip[INET_ADDRSTRLEN];
  http_method method;
  char path[64];
  header headers[MAX_HEADERS];
  int header_count;
  form_field form[MAX_FORM_FIELDS];
  int field_count;
  struct sockaddr_in cli_addr;
  socklen_t cli_addr_len;
  int game_id;
  int player_idx;
  fd_state state;
  int is_ready;
  int is_queued;
} typedef client;

struct {
  int opcode;
  int msg_len;
  unsigned char buf[MAX_WS_FRAME_SIZE];
  unsigned char mask[4];
  unsigned char msg[MAX_WS_MSG_SIZE];
} typedef ws_frame;

struct {
  uint64_t p1_board;
  uint64_t p1_shot_board;
  uint64_t p1_hit_board; // might need this to track hits specifically
  uint64_t p1_ships[NUM_SHIPS];

  uint64_t p2_board;
  uint64_t p2_shot_board;
  uint64_t p2_hit_board;
  uint64_t p2_ships[NUM_SHIPS];

  int8_t winner; // default to -1
  client *players[2];

  int player_turn; // default to 0 for player 1

  // other stuff we talked about here
} typedef game_data;

// server state to be passed down into functions instead of global variables
struct {
  int num_games;
  client *clients[FD_SETSIZE];
  game_data *games[MAX_GAMES];
  fd_set conn_io_set;
  int maxfd;
  int maxi;
  int fd;
} typedef server_ctx;

#endif
