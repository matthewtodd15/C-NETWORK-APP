/**
 *  Created by: Kevin, Matt, and Isaac
 *  Date: 17 February 2025
 *  Server using IO multiplexing (select)
 *  To compile: gcc -Wall main.c -o main
 *  To run: ./main
 */

#include "defs.h"
#include "http.h"
#include "structs.h"
#include "ws.h"
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

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
client *accept_conn(int sockfd, client **clients, int *maxi);

/*
 * Closes and deallocates a client struct
 * pre: client is an open connection and filled struct
 * post: client* is freed and socket is closed, removed from allset
 */
void close_client(client *client, fd_set *allset);

/*
 * Processes clients listening on the provided fd
 * pre: sockfd is a socket setup using the setup_socket function
 * post: the program is done processing clients
 */
void process_clients(int sockfd);

int main(int argc, char **argv)
{
  int serverfd = -1;

  setup_socket(&serverfd);
  printf("Server listening for connections: %d\n", serverfd);
  process_clients(serverfd);

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

client *accept_conn(int sockfd, client **clients, int *maxi)
{
  // check param
  if (sockfd < 0) {
    fprintf(stderr, "Error invalid fd at accept_conn: %d\n", sockfd);
    exit(EXIT_FAILURE);
  }

  client *newClient = malloc(sizeof(client));
  newClient->cli_addr_len = sizeof(newClient->cli_addr);
  newClient->fd =
      accept(sockfd, (SA *)&(newClient->cli_addr), &(newClient->cli_addr_len));

  // store ip str
  memcpy(newClient->ip, inet_ntoa(newClient->cli_addr.sin_addr),
         sizeof(newClient->ip));

  // handle http
  parse_http_request(newClient);

  int result = route_request(newClient);
  if (result == 0) { // if 0, request was handled and can be closed
    free(newClient);
    return NULL;
  }

  int i = 0; // find an unused client to store the socket id
  while (clients[i] != NULL && clients[i]->fd > 0 && i < FD_SETSIZE)
    i++;
  if (i < FD_SETSIZE) {
    clients[i] = newClient;
  }
  else {
    fprintf(stderr, "Too many clients!\n");
    exit(EXIT_FAILURE);
  }

  if (i > *maxi)
    *maxi = i;
  printf("Connection accepted:\n\tIndex:%d\n\tFile Desc: %d\n", i,
         newClient->fd);

  return newClient;
}

void close_client(client *client, fd_set *allset)
{
  close(client->fd);
  FD_CLR(client->fd, allset);
  free(client);
}

void process_clients(int sockfd)
{
  // check param
  if (sockfd < 0) {
    fprintf(stderr, "Error invalid fd at process_clients: %d\n", sockfd);
    exit(EXIT_FAILURE);
  }

  int i, maxi, maxfd;
  int nready;
  client *clients[FD_SETSIZE];
  fd_set rset, allset;

  printf("The number of clients = %d\n", FD_SETSIZE);

  maxfd = sockfd;
  maxi = -1;

  FD_ZERO(&allset);
  FD_SET(sockfd, &allset);

  for (;;) {
    rset = allset;
    nready = select(maxfd + 1, &rset, NULL, NULL, NULL);

    // handle new connections
    if (FD_ISSET(sockfd, &rset)) {
      // setup new client
      client *newClient = accept_conn(sockfd, clients, &maxi);

      // if websocket (persistent), add to multiplexing set
      if (newClient != NULL) {
        FD_SET(newClient->fd, &allset);
        if (newClient->fd > maxfd)
          maxfd = newClient->fd;
        if (--nready <= 0)
          continue; // no more readable descriptors
      }
    }

    // read from current connections
    for (i = 0; i <= maxi; i++) {
      if (clients[i] == NULL || clients[i]->fd < 0)
        continue;
      if (FD_ISSET(clients[i]->fd, &rset)) {
        // check for end of connection, close socket and deallocate
        ws_frame *frame = malloc(sizeof(ws_frame));
        bzero(frame, sizeof(ws_frame));
        if (receive_ws_data(frame, clients[i]) == OP_CLOSE) {
          close_client(clients[i], &allset);
          clients[i] = NULL;
        }
        else {
          switch (frame->opcode) {
          case OP_TEXT:
            printf("data frame message: ");
            for (int i = 0; i < frame->msg_len; i++) {
              char val = frame->message[i] ^ frame->mask[i % 4];
              printf("%c", val);
            }
            printf("\n");
            char message[] = "Hello There";
            send_ws_message(clients[i], message, strlen(message));
            break;
          case OP_PING:
            printf("Received Ping\n");

            unsigned char pong[] = {0x8A, 0x00};
            write(clients[i]->fd, pong, 2);
            break;
          case OP_CLOSE:
            printf("Websocket close request received\n");
            send_ws_close(clients[i]);
            close_client(clients[i], &allset);
            break;
          default:
            break;
          }
        }

        free(frame);
        if (--nready <= 0)
          break;
      }
    }
  }
}
