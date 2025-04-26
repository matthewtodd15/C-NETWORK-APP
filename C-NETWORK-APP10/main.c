#include "structs.h"

client clients[MAX_CLIENTS];

#include "structs.h"

server_ctx server;

/**
 *  Created by: Kevin, Matt, and Isaac
 *  Date: 17 February 2025
 *  Server using IO multiplexing (select)
 *  To compile: gcc -Wall main.c -o main
 *  To run: ./main
 */

#include "game.h"
#include "http.h"
#include "structs.h"
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/select.h>

/*
 * Prepares a socket for use within the program.
 * pre: sockfd_ptr is the address to an integer
 * post: sockfd_ptr points to a listening socket
 */
void setup_socket(int *sockfd_ptr);

/*
 * Accept clients on the provided listening fd
 * pre: sockfd is a socket setup using the setup_socket function,
 *      clients is an array of client* to store the new client
 * post: a client connection has been allocated set up
 */
client *accept_conn(server_ctx* ctx);

/*
 * Closes and deallocates a client struct
 * pre: client is an open connection and filled struct
 * post: client* is freed and socket is closed, removed from io mult set
 */
void close_client(server_ctx* ctx, int idx);

/*
 * Processes clients listening on the provided fd
 * pre: sockfd is a socket setup using the setup_socket function
 * post: the program is done processing clients
 */
void process_clients(server_ctx* ctx);

int handle_http_conn(client *conn);
int handle_ws_conn(server_ctx* ctx, client *conn);

int main(int argc, char **argv)
{
  int serverfd = -1;

  setup_socket(&serverfd);
  printf("Server listening for connections: %d\n", serverfd);

  server_ctx* ctx = malloc(sizeof(server_ctx));
  bzero(ctx, sizeof(server_ctx));
  ctx->fd = serverfd;
  process_clients(ctx);

  return 0;
}

void setup_socket(int *sockfd_ptr)
{
  struct sockaddr_in serv_addr;

  *sockfd_ptr = socket(AF_INET, SOCK_STREAM, 0);
  if (*sockfd_ptr < 0) {
    perror("Error creating socket");
    exit(EXIT_FAILURE);
  }

  bzero(&serv_addr, sizeof(serv_addr));

  // for development, so we dont get port/address already in use
  if (setsockopt(*sockfd_ptr, SOL_SOCKET, SO_REUSEADDR, &(int){1},
                 sizeof(int)) < 0) {
    perror("socket option failed");
    exit(EXIT_FAILURE);
  }

  // for development, so we dont get port/address already in use
#ifdef SO_REUSEPORT
  if (setsockopt(*sockfd_ptr, SOL_SOCKET, SO_REUSEPORT, &(int){1},
                 sizeof(int)) < 0) {
    perror("setsockopt(SO_REUSEPORT) failed");
    exit(EXIT_FAILURE);
  }
#endif

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr.sin_port = htons(PORT);

  int err = bind(*sockfd_ptr, (SA *)&serv_addr, sizeof(serv_addr));
  if (err < 0) {
    perror("Error binding socket");
    exit(EXIT_FAILURE);
  }

  err = listen(*sockfd_ptr, LISTENQ);
  if (err < 0) {
    perror("Error listening socket");
    exit(EXIT_FAILURE);
  }
}

client *accept_conn(server_ctx* ctx)
{
  client *new_client = malloc(sizeof(client));
  bzero(new_client, sizeof(client));
  new_client->cli_addr_len = sizeof(new_client->cli_addr);
  new_client->fd = accept(ctx->fd, (SA *)&(new_client->cli_addr),
                          &(new_client->cli_addr_len));

  // store ip str
  memcpy(new_client->ip, inet_ntoa(new_client->cli_addr.sin_addr),
         sizeof(new_client->ip));

  int i = 0; // find an unused client to store the socket id
  while (ctx->clients[i] != NULL && ctx->clients[i]->fd > 0 && i < FD_SETSIZE) {
    i++;
  }

  if (i < FD_SETSIZE) {
    ctx->clients[i] = new_client;
  }
  else {
    fprintf(stderr, "Too many clients!\n");
    close(new_client->fd);
    free(new_client);
    return NULL;
  }

  if (i > ctx->maxi)
    ctx->maxi = i;

  if (new_client->fd > ctx->maxfd) {
    ctx->maxfd = new_client->fd;
  }

  new_client->state = FD_HTTP;
  new_client->game_id = -1;

  return new_client;
}

void close_client(server_ctx* ctx, int idx)
{
  close(ctx->clients[idx]->fd);
  free(ctx->clients[idx]);
  ctx->clients[idx] = NULL;
}

void process_clients(server_ctx* ctx)
{
  int i;
  int nready;

  ctx->maxfd = ctx->fd;
  ctx->maxi = -1;

  for (;;) {
    // rebuild the set each loop
    FD_ZERO(&ctx->conn_io_set);
    FD_SET(ctx->fd, &ctx->conn_io_set);
    for (int i = 0; i <= ctx->maxi; ++i) {
      if (ctx->clients[i] && ctx->clients[i]->fd > 0)
        FD_SET(ctx->clients[i]->fd, &ctx->conn_io_set);
    }

    nready = select(ctx->maxfd + 1, &ctx->conn_io_set, NULL, NULL, NULL);
    if (nready == -1) {
      perror("select");
      exit(EXIT_FAILURE);
    }

    // handle new http connections
    if (FD_ISSET(ctx->fd, &ctx->conn_io_set)) {
      // setup new client
      client *new_client = accept_conn(ctx);
      if (new_client == NULL) {
        if (--nready <= 0)
          continue;
      }

      if (--nready <= 0)
        continue;
    }

    int res = 0;
    // read from current connections
    for (i = 0; i <= ctx->maxi; i++) {
      if (ctx->clients[i] == NULL || ctx->clients[i]->fd < 0)
        continue;
      if (FD_ISSET(ctx->clients[i]->fd, &ctx->conn_io_set)) {
        switch (ctx->clients[i]->state) {
        case FD_HTTP:
          res = handle_http_conn(ctx->clients[i]);
          if (res != 0) {
            close_client(ctx, i);
          }
          break;
        case FD_WS:
          res = handle_ws_conn(ctx, ctx->clients[i]);
          if (res != 0) {
            close_client(ctx, i);
          }
          break;
        default:
          break;
        }

        if (--nready <= 0) {
          break;
        }
      }
    }
  }
}

int handle_http_conn(client *conn)
{
  int result = parse_http_request(conn);
  if (result != 0) {
    return 1;
  }

  result = route_request(conn);
  if (result == 1) { // if 1, request was upgraded to WS
    conn->state = FD_WS;
    return 0;
  }

  return 2; // all our http connections can be closed
}

int handle_ws_conn(server_ctx* ctx, client* conn)
{
  ws_frame *frame = malloc(sizeof(ws_frame));
  bzero(frame, sizeof(ws_frame));
  if (receive_ws_data(frame, conn) == OP_CLOSE) {
    free(frame);
    return 1;
  }
  else {
    char *message = "Hello from server";
    switch (frame->opcode) {
    case OP_TEXT: // for testing
      send_ws_message(conn, message, strlen(message));
      break;
    case OP_BIN: // for game messages
      handle_game_msg(ctx, frame->msg, conn);
      break;
    case OP_PING:
      printf("Received Ping\n");
      unsigned char pong[] = {0x8A, 0x00};
      write(conn->fd, pong, 2);
      break;
    case OP_CLOSE:
      printf("Websocket close request received\n");
      send_ws_close(conn); // may not need this here
      free(frame);
      return 2;
      break;
    default:
      break;
    }
  }

  free(frame);
  return 0;
}
