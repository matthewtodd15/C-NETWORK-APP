#ifndef WS_H
#define WS_H

#include "structs.h"

/*
 * Handles receiving a websocket frame and parsing via the WebSocket protocol
 * @return opcode
 * @param allocated ws_frame struct
 * @param persistently connected client struct
 */
int receive_ws_data(ws_frame *dst_frame, client *conn);

/*
 * Handles sending a websocket text frame via the WebSocket protocol
 * @param persistently connected client struct
 * @param message (at most MAX_WS_MSG_SIZE bytes)
 * @param length (at most MAX_WS_MSG_SIZE)
 */
void send_ws_message(client *conn, char *message, int n);

/*
 * Handles sending a websocket connection-close frame via the WebSocket protocol
 * Connection should be closed by the user shortly thereafter
 * @param persistently connected client struct
 */
void send_ws_close(client *conn);

/**
 * helper for marshalling ws_frame struct into sendable ws frame byte string
 * @param frame struct to marshal
 * @param length of bytestring returned
 * @return byte string
 */
unsigned char *ws_frame_to_bytes(ws_frame *frame, int *len);

#endif
