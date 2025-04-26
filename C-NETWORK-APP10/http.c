#include "http.h"
#include "defs.h"
#include <stdio.h>
#include <string.h>

char *get_header_val(client *conn, char *key)
{
  for (int i = 0; i < conn->header_count; i++) {
    if (strcmp(conn->headers[i].key, key) == 0) {
      return strdup(conn->headers[i].value);
    }
  }

  return NULL;
}

char *get_cookie_val(client *conn, char *key)
{
  // retrieve the cookie header
  char *cookies = get_header_val(conn, "Cookie");

  char *name = malloc(strlen(key) + 1);
  char *value = malloc(MAX_COOKIE_VAL_SIZE);
  bzero(name, strlen(key) + 1);
  bzero(value, MAX_COOKIE_VAL_SIZE);

  for (;;) {
    char *equal_idx = strstr(cookies, "=");
    char *colon_idx = strstr(cookies, ";");

    if (equal_idx == NULL) {
      return NULL;
    }

    *equal_idx = '\0';
    if (colon_idx != NULL) {
      *colon_idx = '\0';
    }

    strncpy(name, cookies, strlen(key));

    if (strcmp(name, key) == 0) {
      strncpy(value, equal_idx + 1, MAX_COOKIE_VAL_SIZE);
      break;
    }

    cookies = colon_idx + 1;
    
    // if no ";" then we've parsed the last cookie
    if (colon_idx == NULL) {
      return NULL;
    }
  }

  free(name);
  return value;
}

char *get_form_val(client *conn, char *key)
{
  for (int i = 0; i < conn->field_count; i++) {
    if (strcmp(conn->form[i].key, key) == 0) {
      return strdup(conn->form[i].value);
    }
  }

  return NULL;
}

// helper to get the mime type from the file -i unix command
char *get_mime_type(const char *filename)
{
  char command[64];
  // TODO: vulnerable to command injection... add checks against valid options
  snprintf(command, sizeof(command), "file --mime-type -b %s", filename);
  FILE *fp = popen(command, "r");
  if (fp == NULL) {
    perror("popen failed");
    return NULL;
  }

  char *mime_type = malloc(MAX_MIME_TYPE_LEN);
  int i;
  for (i = 0; i < MAX_MIME_TYPE_LEN; i++) {
    int c = fgetc(fp);
    if (c == EOF) {
      break;
    }

    mime_type[i] = (char)c;
  }
  mime_type[i - 1] = '\0';

  pclose(fp);
  return mime_type;
}

int validate_request(client *conn)
{
  char *auth_cookie = get_cookie_val(conn, "c-game-auth");
  printf("Cookie: %s\n", auth_cookie);
  if (auth_cookie == NULL || strcmp(auth_cookie, "") == 0) {
    return 1;
  }

  // decrypt b64
  // encrypt new hash with data
  // check hashes

  return 0;
}

int route_request(client *conn)
{
  http_response *res;
  switch (conn->method) {
  case GET:
    if (strcmp(conn->path, "/ws") == 0) {
      upgrade_conn(conn);
      return 1; // tell the caller we have a websocket connection
    }
    else {
      // Read public asset to buffer
      char path[128] = "public";

      strncat(path, conn->path, sizeof(path) - sizeof("public"));
      if (conn->path[strlen(conn->path) - 1] ==
          '/') { // handle paths that end in slash
        strcat(path, "index.html");
      }
      else if (strstr(conn->path, ".") ==
               NULL) { // handle paths that dont end in slash
        strcat(path, "/index.html");
      }

      // User must be authorized to access dashboard
      if (strcmp(path, "public/dashboard/index.html") == 0) {
        int err = validate_request(conn);
        if (err != 0) {
          res = build_http_response(302, "Found", NULL, 0, NULL);
          add_header(res, "Location", "/"); // send back to login page
          send_http_response(conn->fd, res);
        }
      }

      FILE *asset_path = fopen(path, "r");
      if (asset_path == NULL) {
        printf("Error opening file: %s\n", path);
        res = build_http_response(NOT_FOUND, "Not Found", NULL, 0, NULL);
        send_http_response(conn->fd, res);
        return 0;
      }

      size_t num_read;
      char *asset_buffer = malloc(MAX_RESPONSE_FILE_SIZE);
      num_read = fread(asset_buffer, 1, MAX_RESPONSE_FILE_SIZE - 1, asset_path);
      asset_buffer[num_read] = '\0';
      fclose(asset_path);

      // get the mime type from the file -i unix command
      char *content_type = get_mime_type(path);
      if (content_type == NULL) {
        printf("Error opening file: %s\n", path);
        res = build_http_response(INTERNAL_SERVER_ERROR,
                                  "Internal Server Error", NULL, 0, NULL);
        send_http_response(conn->fd, res);
        return 0;
      }

      res = build_http_response(OK, "OK", asset_buffer, num_read, content_type);
      send_http_response(conn->fd, res);
      free(content_type);
      free(asset_buffer);
    }
    break;
  case POST:
    if (strcmp(conn->path, "/") == 0 ||
        strcmp(conn->path, "/index.html") == 0) {
      // authenticate
      char *username = get_form_val(conn, "username");
      char *password = get_form_val(conn, "password");

      printf("Credentials: %s %s\n", username, password);
      if (username == NULL || password == NULL) {
        res = build_http_response(401, "Invalid Credentials", NULL, 0, NULL);
        send_http_response(conn->fd, res);
        return 0;
      }
      if (strlen(username) <= 0 || strlen(password) <= 0) {
        res = build_http_response(401, "Invalid Credentials", NULL, 0, NULL);
        send_http_response(conn->fd, res);
        return 0;
      }

      // set cookie
      res = build_http_response(302, "Found", NULL, 0, NULL);

      add_header(res, "Location", "/dashboard");
      add_header(res, "Set-Cookie", "c-game-auth=1234; Secure; HttpOnly");
      send_http_response(conn->fd, res);
      printf("logged in!\n");
    }
  default:
    res = build_http_response(METHOD_NOT_ALLOWED, "Method Not Allowed", NULL, 0,
                              NULL);
    send_http_response(conn->fd, res);
    break;
  }

  return 0;
}

char *build_websocket_accept_header(client *conn)
{
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

void upgrade_conn(client *conn)
{
  char *ws_accept_encoded = build_websocket_accept_header(conn);
  if (ws_accept_encoded == NULL) {
    fprintf(stderr, "Error upgrading connection: ws key not found\n");
    close(conn->fd);
    return;
  }

  // upgrade handshake
  http_response* res = build_http_response(101, "Switching Protocols", NULL, 0, NULL);
  add_header(res, "Upgrade", "websocket");
  add_header(res, "Connection", "Upgrade");
  add_header(res, "Sec-WebSocket-Accept", ws_accept_encoded);
  send_http_response(conn->fd, res);
}

void add_header(http_response *response, char *key, char *value)
{
  strncpy(response->headers[response->header_count].key, key,
          MAX_HEADER_KEY_SIZE);
  strncpy(response->headers[response->header_count].value, value,
          MAX_HEADER_VAL_SIZE);
  response->header_count++;
}

http_response *build_http_response(int status_code, char *message,
                                   char *content, int content_len,
                                   char *content_type)
{
  http_response *res = malloc(sizeof(http_response));
  bzero(res, sizeof(http_response));
  sprintf(res->version_line, "HTTP/1.1 %d %s", status_code, message);

  char *content_len_str = malloc(20); // long between 2 and 20 bytes

  if (content != NULL && content_type != NULL) {
    res->body = content;
    res->body_len = content_len;

    sprintf(content_len_str, "%d", content_len);
    add_header(res, "Content-Type", content_type);
    add_header(res, "Content-Length", content_len_str);
  }

  add_header(res, "Accept-Ranges", "bytes");
  add_header(res, "Connection", "close");
  return res;
}


// helper for converting response struct to a string with headers and data
char *http_res_tostr(http_response *res, int *res_len)
{
  char *response = malloc(MAX_RESPONSE_SIZE);
  bzero(response, MAX_RESPONSE_SIZE);
  int nwritten = 0;
  int n = 0;
  nwritten += sprintf(response, "%s\r\n", res->version_line);
  while (n < res->header_count && nwritten < MAX_RESPONSE_SIZE) {
    nwritten += sprintf(response + nwritten, "%s: %s\r\n", res->headers[n].key,
                        res->headers[n].value);
    n++;
  }

  // end headers with extra \r\n, body ends with 2 \r\n
  nwritten += sprintf(response + nwritten, "\r\n");

  if (res->body != NULL) {
    // must use memcpy because of possible null bytes in body data
    memcpy(response + nwritten, res->body, res->body_len);
    nwritten += res->body_len;
  }

  response[++nwritten] = '\0';
  *res_len = nwritten - 1;
  return response;
}

int send_http_response(int fd, http_response *res)
{
  int res_len = 0;
  char *response = http_res_tostr(res, &res_len);
  int n = write(fd, response, res_len);
  if (n <= 0) {
    perror("Error sending http response");
    return 1;
  }

  free(response);
  free(res);
  return 0;
}

int parse_http_request(client *conn)
{
  if (conn->fd < 0) {
    fprintf(stderr, "Error reading request: Bad file desc %d", conn->fd);
    return 1;
  }

  char *buf = malloc(MAX_REQUEST_SIZE);
  bzero(buf, MAX_REQUEST_SIZE);

  size_t nread = read(conn->fd, buf, MAX_REQUEST_SIZE);
  if (nread == -1) {
    perror("Error reading request");
    return 2;
  }
  else if (nread == 0) {
    printf("Client closed the connection\n");
    return 3;
  }

  // open buffer for parsing headers
  FILE *stream = fmemopen(buf, nread, "r");
  if (stream == NULL) {
    perror("Error opening client stream");
    return 4;
  }

  conn->header_count = 0;
  conn->field_count = 0;

  char *line;
  char *method = malloc(5); // big enough for "POST\0"
  bzero(method, 5);

  size_t line_len = 0;
  getline(&line, &line_len, stream);
  sscanf(line, "%s %s", method, conn->path);

  if (strcmp(method, "GET") == 0) {
    conn->method = GET;
  }
  else if (strcmp(method, "POST") == 0) {
    conn->method = POST;
  }
  else {
    conn->method = -1;
  }

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
  if (conn->method == POST) {
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
      strncpy(f->value, equal_idx + 1, MAX_FORM_VAL_SIZE);
      conn->field_count++;

      // else move to next value
      line = and_idx + 1;

      // if no &, we've parsed the last field
      if (and_idx == NULL) {
        break;
      }
    }
  }

  printf("Received request: %s %s on FD: %d\n", method, conn->path, conn->fd);

  fclose(stream); // Must close stream first
  free(buf);
  free(method);
  // free(line);
  return 0;
}
