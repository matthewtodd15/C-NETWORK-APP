#ifndef STRUCTS_H
#define STRUCTS_H

#include "common.h"
#include "defs.h"

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
} typedef http_response;

struct {
  int fd;
  char ip[INET_ADDRSTRLEN];
  char method[8]; // big enough for UPDATE + \0
  char path[64];
  header headers[MAX_HEADERS];
  int header_count;
  form_field form[MAX_FORM_FIELDS];
  int field_count;
  struct sockaddr_in cli_addr;
  socklen_t cli_addr_len;
} typedef client;

struct {
  int opcode;
  int msg_len;
  unsigned char buf[MAX_WS_FRAME_SIZE];
  unsigned char mask[4];
  unsigned char message[MAX_WS_MSG_SIZE];
} typedef ws_frame;


// game structs
struct {
  uint64_t p1_board;
  uint64_t p1_shot_board;
  uint64_t p1_hit_board; // might need this to track hits specifically
  uint64_t *p1_ships;    // 1a, 1b, 2a, 2b, 3, 4

  uint64_t p2_board;
  uint64_t p2_shot_board;
  uint64_t p2_hit_board; 
  uint64_t *p2_ships;

  int8_t winner; // default to -1

  // other stuff we talked about here
} typedef game_t;


#endif
