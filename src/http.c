#include "http.h"
#include <openssl/sha.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

extern connection connections[];
extern int num_conns;

char *get_header_val(HttpRequest *req, char *key) {
  return NULL;
}

int route_request(int fd, HttpRequest *req) {
  return 0;
}

// upgrade connection, play game
void upgrade_conn(int fd, HttpRequest *req) {
}

int send_ws_upgrade_response(int fd, char *encoded_key) {
  return 0;
}

int send_http_response(int fd, int status_code, char *content) {
  return 0;
}

int parse_http_request(int fd, HttpRequest *req) {
  return 0;
}

void *handle_ws_conn(void *arg) {
  return NULL;
}

void base64_encode(char *data, int n, char *encoded) {
}
