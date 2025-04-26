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
#define MAX_COOKIE_VAL_SIZE 256
#define MAX_HEADERS 30
#define MAX_FORM_FIELDS 20
#define MAX_REQUEST_SIZE 4096
#define MAX_RESPONSE_SIZE 4096 * 5
#define MAX_RESPONSE_FILE_SIZE 4096 * 4
#define MAX_MIME_TYPE_LEN 32

#define MAX_WS_FRAME_SIZE 131 // header (2) + mask (4) + msg (125)
#define MAX_WS_MSG_SIZE 125   // message at most 125 bytes

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

// http methods allowed
enum HTTP_METHOD { GET, POST } typedef http_method;

#define P1 0
#define P2 1
#define bool int
#define TRUE 1
#define FALSE 0
#define NUM_SHIPS 6 // 1a, 2a, 2b, 3a, 3b, 4
#define MAX_GAMES 20

#endif

#define MAX_CLIENTS 128
