#ifndef DEFS_H
#define DEFS_H

#define LISTENQ 1024
// #define FD_SETSIZE 10 or take the default of 1024
#define SA struct sockaddr

#define PORT 4242
#define MAX_CONNECTIONS 50
#define MAX_VLINE_SIZE 128
#define MAX_HEADER_KEY_SIZE 64
#define MAX_HEADER_VAL_SIZE 128
#define MAX_FORM_KEY_SIZE 64
#define MAX_FORM_VAL_SIZE 128
#define MAX_HEADERS 30
#define MAX_FORM_FIELDS 20
#define MAX_RESPONSE_SIZE 4096

#define MAX_WS_FRAME_SIZE 132 // header (2) + mask (4) + msg (126)
#define MAX_WS_MSG_SIZE 126   // message at most 126 bytes

#define WS_GUID "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"

#define OP_CLOSE 0x08 
#define OP_PING 0x09
#define OP_PONG 0x0A
#define OP_TEXT 0x01
#define OP_BIN 0x02

// http status codes
#define NOT_FOUND 404
#define INTERNAL_SERVER_ERROR 500
#define BAD_REQUEST 400
#define OK 200
#define METHOD_NOT_ALLOWED 405

// game defs
#define P1 0
#define P2 1
#define bool int
#define TRUE 1
#define FALSE 0

/**
* Below are some of the opcodes for our game 
* protocol (subject to change). Server and client
* will send messages back and forth with information
* of these types
*
* How do we secure this so users can't intercept or
* inject game messages? Do we send a token with the
* websocket msgs?
*
* TO SECURE PACKETS - encode the packet with an auth
* key given to the user at game time that includes the
* user_id and game_id.
*/

// client -> server (put me in a game)
#define GAME_MSG_READY 0 
// in case of disconnect or refresh (includes all game data needed client-side
#define GAME_MSG_SYNC 1 
// client -> server (my board is ready, validate it)
#define GAME_MSG_BOARD_SETUP 2 
// server -> client (all)
#define GAME_MSG_TURN 4 
// client -> server (x, y)
#define GAME_MSG_SHOT 5 
#define GAME_MSG_RESIGN 6 
// general error 
#define GAME_MSG_ERROR 7 
#define GAME_MSG_CLOSE 8 
// are we still in active game?
#define GAME_MSG_PING 9 
// game still active
#define GAME_MSG_PONG 10 

/**
  * General packet structure:
  * 
  * 4 bits - opcode
  * _ bits (defined by opcode) - args
  * ... more args
  * ____ ____ ____
  * opcd arg1 arg2
  *
  * Example from client: Shot packet with coordinates x=2, y=3
  *
  * 4 bits - opcode
  * 4 bits - x coord
  * 4 bits - y coord
  * 1 bit - hit true or false
  *
  * 0101 010 011 0
  *
  * Example response to all clients: hit
  *
  * 4 bits - opcode
  * 4 bits - x coord
  * 4 bits - y coord
  * 1 bit - hit true or false
  *
  * 0101 010 011 1
  * 
  *
  */

#endif
