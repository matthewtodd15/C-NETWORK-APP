#include "http.h"
#include <stdio.h>
#include <stdlib.h>

connection connections[MAX_CONNECTIONS];
int num_conns = 0;

int main(int argc, char **argv) {
  int serverfd, clientfd;
  struct sockaddr_in server_addr;
  socklen_t serv_addr_len = sizeof(server_addr);

  serverfd = socket(AF_INET, SOCK_STREAM, 0);
  if (serverfd < 0) {
    perror("socket failed");
    exit(EXIT_FAILURE);
  }

  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(PORT);

  if (bind(serverfd, (struct sockaddr *)&server_addr, serv_addr_len) < 0) {
    perror("bind failed");
    exit(EXIT_FAILURE);
  }

  if (listen(serverfd, 50) < 0) {
    perror("listen failed");
    exit(EXIT_FAILURE);
  }

  printf("Server listening on port: %d\n", PORT);

  // handle client connections
  for (;;) {
    connection *conn = &(connections[num_conns]);
    clientfd = accept(serverfd, (struct sockaddr *)&conn->client_addr,
                      &conn->client_addr_len);

    if (clientfd < 0) {
      perror("Error accepting connection\n");
      continue;
    }

    if (num_conns >= MAX_CONNECTIONS) {
      perror("Error accepting connection: max connections reached, dropping "
             "connection\n");
      continue;
    }

    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(conn->client_addr.sin_addr), client_ip,
              INET_ADDRSTRLEN);
    printf("Client connected: %s\n", client_ip);

    // thread the connection
    int *conn_id = malloc(sizeof(int));
    conn->connfd = clientfd;
    strncpy(conn->ip, client_ip, INET_ADDRSTRLEN);
    *conn_id = num_conns;

    pthread_t *thread_id = malloc(sizeof(pthread_t));
    pthread_create(thread_id, NULL, handle_conn, (void *)conn_id);
    pthread_detach(*thread_id);

    num_conns++;
  }

  close(serverfd);
  return 0;
}
