#include "http.h"
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

extern connection connections[];
extern int num_conns;

char *get_header_val(connection *conn, char *key) {
  for (int i = 0; i < conn->header_count; i++) {
    if (strcmp(conn->headers[i].key, key) == 0) {
      return strdup(conn->headers[i].value);
    }
  }

  return NULL;
}

int route_request(connection *conn) {
  if (strcmp(conn->method, "GET") == 0) {
    if (strcmp(conn->path, "/ws") == 0) {
      upgrade_conn(conn);
    } else {
      // Read public asset to buffer
      char path[128] = "public";

      strncat(path, conn->path, sizeof(path) - sizeof("public"));
      if (conn->path[strlen(conn->path) - 1] == '/') {
        strcat(path, "index.html");
      }
      FILE *asset_path = fopen(path, "r");
      if (!asset_path) {
        if (errno == ENOENT || errno == ENOTDIR) {
          send_http_response(conn->connfd, NOT_FOUND, "Not Found", NULL);
        }
        printf("Error opening file: %s\n", path);
        return 1;
      }

      size_t ret;
      char asset_buffer[1024];
      ret = fread(asset_buffer, 1, sizeof(asset_buffer) - 1, asset_path);
      asset_buffer[ret] = '\0';
      fclose(asset_path);

      send_http_response(conn->connfd, OK, "OK", asset_buffer);
    }
  }

  return 0;
}

char *build_websocket_accept_header(connection *conn) {
  char *encoded;
  char ws_accept[256] = {0};
  unsigned char ws_accept_hash[SHA_DIGEST_LENGTH] = {0};

  char *ws_key = get_header_val(conn, "Sec-WebSocket-Key");
  if (ws_key == NULL) {
    return NULL;
  }

  strncpy(ws_accept, ws_key, strlen(ws_key));
  strcat(ws_accept, WS_GUID);

  SHA1((unsigned char *)ws_accept, strlen(ws_accept), ws_accept_hash);

  // Encoding
  base64_encodestate encode_state;
  base64_init_encodestate(&encode_state);

  int input_length = SHA_DIGEST_LENGTH;
  int output_length = (input_length + 2) / 3 * 4 + 1; // +1 for null terminator
  encoded = (char *)malloc(output_length);
  bzero(encoded, output_length);

  int encoded_size = base64_encode_block((char *)ws_accept_hash, input_length,
                                         encoded, &encode_state);
  encoded_size += base64_encode_blockend(encoded + encoded_size, &encode_state);
  encoded[encoded_size] = '\0';

  printf("Hex encoded: ");
  for (int i = 0; i < encoded_size; i++) {
    printf("%02x ", encoded[i]);
  }
  printf("\n");

  size_t len = strlen(encoded);
  if (len > 0 && encoded[len - 1] == '\n') {
    encoded[len - 1] = '\0';
  }

  return encoded;
}

void send_ws_close(connection *conn) {
  unsigned char frame[2] = {0}; // 2 bytes for header, max 125 bytes for payload

  frame[0] = 0x88; // fin = 1, opcode = 8 for close

  size_t nsent = send(conn->connfd, frame, 2, 0);
  printf("data sent: ");
  for (int i = 0; i < nsent; i++) {
    printf("%02x ", frame[i]);
  }
  printf("\n");
  printf("sent %zu bytes\n", nsent);
}

void send_ws_message(connection *conn, char *message, int n) {
  if (n > 125) {
    fprintf(stderr, "Error sending ws data: data size %d too large\n", n);
    return;
  }

  unsigned char frame[125] = {
      0}; // 2 bytes for header, max 125 bytes for payload

  frame[0] = 0x81;     // fin = 1, opcode = 1 for text
  frame[1] = n & 0x7f; // length of data (mask bit always 0) 0111 1111

  memcpy(frame + 2, message, n + 2);

  size_t nsent = write(conn->connfd, frame, n + 2);
  printf("data sent: ");
  for (int i = 0; i < nsent; i++) {
    printf("%02x ", frame[i]);
  }
  printf("\n");
  printf("sent %zu bytes\n", nsent);
}

int receive_ws_data(connection *conn) {
  unsigned char buf[1024] = {0};
  int nread = recv(conn->connfd, buf, sizeof(buf), 0);
  if (nread == -1) {
    fprintf(stderr, "Error reading message\n");
    return 0;
  }

  int len = buf[1] & 0x7f;
  unsigned char *mask = buf + 2;
  unsigned char *message = mask + 4;

  printf("data frame Hex: ");
  for (int i = 0; i < nread; i++) {
    printf("%02x ", buf[i]);
  }
  printf("\n");

  int opcode = buf[0] & 0x0F;
  switch (opcode) {
  case 1:
    printf("data frame message: ");
    for (int i = 0; i < len; i++) {
      char val = message[i] ^ mask[i % 4];
      printf("%c", val);
    }
    printf("\n");

    sleep(5);
    char message[] = "Hello There";
    send_ws_message(conn, message, strlen(message));
    break;
  case 9:
    printf("Received Ping\n");

    unsigned char pong[] = {0x8A, 0x00};
    write(conn->connfd, pong, 2);
    break;
  case 8:
    printf("Websocket close request received\n");
    send_ws_close(conn);
    break;
  default:
    break;
  }

  return opcode;
}

void upgrade_conn(connection *conn) {
  char *ws_accept_encoded = build_websocket_accept_header(conn);
  if (ws_accept_encoded == NULL) {
    perror("Error upgrading connection, ws key not found");
    close(conn->connfd);
    exit(EXIT_FAILURE);
  }

  // upgrade handshake
  send_ws_upgrade_response(conn->connfd, ws_accept_encoded);
  for (;;) {
    if (receive_ws_data(conn) == 8) {
      break;
    }
  }
  printf("Closing connection\n");
}

void add_header(http_response *response, char *key, char *value) {
  strncpy(response->headers[response->header_count].key, key,
          MAX_HEADER_KEY_SIZE);
  strncpy(response->headers[response->header_count].value, value,
          MAX_HEADER_VAL_SIZE);
  response->header_count++;
}

http_response *build_http_response(int status_code, char *message) {
  http_response *res = malloc(sizeof(http_response));
  bzero(res, sizeof(http_response));
  sprintf(res->version_line, "HTTP/1.1 %d %s", status_code, message);
  return res;
}

int send_ws_upgrade_response(int fd, char *encoded_key) {
  char response[1024] = {0};
  int n = sprintf(
      response,
      "HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: "
      "Upgrade\r\n"
      "Sec-WebSocket-Accept: %s\r\n\r\n",
      encoded_key);
  int nsent = write(fd, response, n);
  printf("--- Sent %d byte upgrade response ---\n%s\n", nsent, response);
  return 0;
}

char *http_res_tostr(http_response *res) {
  char *response = malloc(1024); // TODO: make constant #define
  bzero(response, 1024);
  int nwritten = 0;
  int n = 0;
  nwritten += sprintf(response, "%s\r\n", res->version_line);
  while (n < res->header_count && nwritten < 1024) {
    printf("header");
    nwritten += sprintf(response + nwritten, "%s: %s\r\n", res->headers[n].key,
                        res->headers[n].value);
    n++;
  }

  // end headers with extra \r\n, body ends with 2 \r\n
  nwritten += sprintf(response + nwritten, "\r\n");

  if (res->body != NULL) {
    nwritten += sprintf(response + nwritten, "%s\r\n\r\n", res->body);
  }

  response[nwritten] = '\0';
  return response;
}

int send_http_response(int fd, int status_code, char *message, char *content) {
  http_response *res = build_http_response(status_code, message);
  char content_len[20]; // long between 2 and 20 bytes

  res->body = content;

  sprintf(content_len, "%ld", strlen(content));

  add_header(res, "Accept-Ranges", "bytes");
  add_header(res, "Content-Length", content_len);
  add_header(res, "Content-Type", "text/html");
  add_header(res, "Connection", "close");
  char *response = http_res_tostr(res);
  int n = write(fd, response, strlen(response));
  printf("--- Sent %d byte Response ---\n%s\n", n, response);

  free(response);
  return 0;
}

int parse_http_request(connection *conn) {
  char buf[1024] = {0};
  size_t nread = read(conn->connfd, buf, sizeof(buf));
  printf("Received request: %s\n", buf);

  // open buffer for parsing headers
  FILE *stream = fmemopen(buf, nread, "r");
  if (stream == NULL) {
    perror("Error opening client stream");
    return 1;
  }

  conn->header_count = 0;

  char *line;
  size_t line_len;
  getline(&line, &line_len, stream);
  sscanf(line, "%s %s", conn->method, conn->path);

  // read headers into memory
  while (getline(&line, &line_len, stream) > 0 &&
         conn->header_count < MAX_HEADERS) {
    header *h = &(conn->headers[conn->header_count]);
    sscanf(line, "%s %s", h->key, h->value);
    h->key[strlen(h->key) - 1] = '\0'; // remove colon from key
    conn->header_count++;
  }

  fclose(stream);
  return 0;
}

void *handle_conn(void *arg) {
  int conn_id = *(int *)arg;
  connection conn_info = connections[conn_id];
  printf("%d, %d\n", conn_id, conn_info.connfd);

  if (conn_info.connfd < 0) {
    perror("Connection fd not found");
    pthread_exit((void *)1);
  }

  parse_http_request(&conn_info);

  // handle routing
  int result = route_request(&conn_info);
  if (result != 0) {
    perror("Error routing request");
  }

  close(conn_info.connfd);
  pthread_exit(NULL);
}
