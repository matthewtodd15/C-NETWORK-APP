#include "http.h"

extern int num_conns;

char *get_header_val(client *conn, char *key) {
  for (int i = 0; i < conn->header_count; i++) {
    if (strcmp(conn->headers[i].key, key) == 0) {
      return strdup(conn->headers[i].value);
    }
  }

  return NULL;
}

char *get_form_val(client *conn, char *key) {
  for (int i = 0; i < conn->field_count; i++) {
    if (strcmp(conn->form[i].key, key) == 0) {
      return strdup(conn->form[i].value);
    }
  }

  return NULL;
}

int route_request(client *conn) {
  if (strcmp(conn->method, "GET") == 0) {
    if (strcmp(conn->path, "/ws") == 0) {
      upgrade_conn(conn);
      return 1; // tell the caller we have a websocket connection
    } else {
      // Read public asset to buffer
      char path[128] = "public";

      strncat(path, conn->path, sizeof(path) - sizeof("public"));
      if (conn->path[strlen(conn->path) - 1] ==
          '/') { // handle paths that end in slash
        strcat(path, "index.html");
      } else if (strstr(conn->path, ".") ==
                 NULL) { // handle paths that dont end in slash
        printf("adding /index.html to path");
        strcat(path, "/index.html");
      }

      FILE *asset_path = fopen(path, "r");
      if (asset_path == NULL) {
        printf("Error opening file: %s\n", path);
        send_http_response(conn->fd, NOT_FOUND, "Not Found", NULL);
        return 0;
      }

      size_t ret;
      char asset_buffer[1024];
      ret = fread(asset_buffer, 1, sizeof(asset_buffer) - 1, asset_path);
      asset_buffer[ret] = '\0';
      fclose(asset_path);

      send_http_response(conn->fd, OK, "OK", asset_buffer);
    }
  } else if (strcmp(conn->method, "POST") == 0) {
    if (strcmp(conn->path, "/") == 0) {
      // authenticate
      char *username = get_form_val(conn, "username");
      char *password = get_form_val(conn, "password");

      printf("Credentials: %s %s\n", username, password);
      if (username == NULL || password == NULL) {
        send_http_response(conn->fd, 401, "Unauthorized", NULL);
        return 0;
      }
      if (strlen(username) <= 0 || strlen(password) <= 0) {
        send_http_response(conn->fd, 401, "Unauthorized", NULL);
        return 0;
      }

      // set cookie
      // redirect to dash
      send_http_redirect(conn->fd, "/dashboard");
      printf("logged in!\n");
      return 0;
    }
  } else {
    send_http_response(conn->fd, METHOD_NOT_ALLOWED, "Method Not Allowed",
                       NULL);
  }

  return 0;
}

char *build_websocket_accept_header(client *conn) {
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

  size_t len = strlen(encoded);
  if (len > 0 && encoded[len - 1] == '\n') {
    encoded[len - 1] = '\0';
  }

  return encoded;
}

void upgrade_conn(client *conn) {
  char *ws_accept_encoded = build_websocket_accept_header(conn);
  if (ws_accept_encoded == NULL) {
    perror("Error upgrading connection, ws key not found");
    close(conn->fd);
    exit(EXIT_FAILURE);
  }

  // upgrade handshake
  send_ws_upgrade_response(conn->fd, ws_accept_encoded);
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

int send_http_redirect(int fd, char *url) {
  http_response *res = build_http_response(302, "Found");

  add_header(res, "Location", url);
  add_header(res, "Connection", "close");
  char *response = http_res_tostr(res);
  int n = write(fd, response, strlen(response));
  printf("--- Sent %d byte Response ---\n%s\n", n, response);

  free(response);
  return 0;
}

int send_http_response(int fd, int status_code, char *message, char *content) {
  http_response *res = build_http_response(status_code, message);
  char content_len[20]; // long between 2 and 20 bytes

  res->body = content;

  if (content != NULL) {
    sprintf(content_len, "%ld", strlen(content));
  } else {
    sprintf(content_len, "%d", 0);
  }

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

int parse_http_request(client *conn) {
  char *buf = malloc(2048);
  bzero(buf, 2048);
  size_t nread = read(conn->fd, buf, 2048);
  if (nread == -1) {
    perror("Error reading request");
    return 1;
  }
  // (because of io multiplexing)
  printf("Received request: %s\n", buf);

  // open buffer for parsing headers
  FILE *stream = fmemopen(buf, nread, "r");
  if (stream == NULL) {
    perror("Error opening client stream");
    return 2;
  }

  conn->header_count = 0;
  conn->field_count = 0;

  char *line;
  size_t line_len = 0;
  getline(&line, &line_len, stream);
  sscanf(line, "%s %s", conn->method, conn->path);

  // read headers into memory
  // if read is not greater than 1 then we have a double new line
  while (getline(&line, &line_len, stream) > 2 &&
         conn->header_count < MAX_HEADERS) {
    header *h = &(conn->headers[conn->header_count]);
    sscanf(line, "%s %s", h->key, h->value);
    h->key[strlen(h->key) - 1] = '\0'; // remove colon from key
    conn->header_count++;
  }

  // if post, we want to parse the form
  if (strcmp(conn->method, "POST") == 0) {
    // skip extra new line
    getline(&line, &line_len, stream);
    // form should be encoded into 1 line
    getline(&line, &line_len, stream);

    while (conn->field_count < MAX_FORM_FIELDS) {
      form_field *f = &(conn->form[conn->field_count]);
      char *equal_idx = strstr(line, "=");
      char *and_idx = strstr(line, "&");
      // end string at equals for parsing
      *equal_idx = '\0';
      if (and_idx != NULL) {
        *and_idx = '\0';
      }
      strncpy(f->key, line, MAX_FORM_KEY_SIZE);
      strncpy(f->value, equal_idx + 1, MAX_FORM_KEY_SIZE);
      conn->field_count++;

      // if no &, we've parsed the last field
      if (and_idx == NULL) {
        break;
      }
      // else move to next value
      line = and_idx + 1;
    }
  }

  fclose(stream); // Must close stream first
  free(buf);
  // free(line);
  return 0;
}
