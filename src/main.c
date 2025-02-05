#include <stdio.h>
#include <stdlib.h>
#include "http.h"

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

  if (listen(serverfd, 3) < 0) {
    perror("listen failed");
    exit(EXIT_FAILURE);
  }

  printf("Server listening on port: %d\n", PORT);

  // handle client connections
  for (;;) {
    HttpRequest *req = malloc(sizeof(HttpRequest));
    clientfd = accept(serverfd, (struct sockaddr *)&req->client_addr,
                      &req->client_addr_len);
    if (clientfd < 0) {
      perror("Error accepting connection\n");
      continue;
    }

    if (num_conns >= MAX_CONNECTIONS) {
      perror("Error accepting connection: max connections reached, dropping "
             "connection\n");
      continue;
    }

    parse_http_request(clientfd, req);

    // handle routing
    int result = route_request(clientfd, req);
    if (result != 0) {
      perror("Error routing request");
    }

    free(req);
  }

  close(serverfd);
  return 0;
}

