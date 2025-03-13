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

#endif
