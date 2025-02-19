#ifndef HTTP_H
#define HTTP_H

#include "structs.h"

/*
 * Finds the value of a request header by key
 * @return value of header, NULL if header not found
 * @param request object
 * @param header key
 */
char *get_header_val(client *conn, char *key);

/*
 * Finds the value of a request form field by key
 * @return value of field, NULL if field not found
 * @param request object
 * @param field key
 */
char *get_form_val(client *conn, char *key);

/*
 * Handles sending protocol upgrade response via the HTTP protocol
 * @return error code (0 if successful)
 * @param socket file descriptor
 * @param base64 encoded ws key
 */
int send_ws_upgrade_response(int fd, char *encoded_key);

/*
 * Handles sending redirect via the HTTP protocol
 * @return error code (0 if successful)
 * @param socket file descriptor
 * @param url to redirect to
 */
int send_http_redirect(int fd, char *url);

/*
 * Handles sending text/html content via the HTTP protocol
 * @return error code (0 if successful)
 * @param socket file descriptor
 * @param text or html content
 */
int send_http_response(int fd, int status_code, char *message, char *content);

/*
 * Handles sending text/html content via the HTTP protocol
 * @return error code (0 if successful)
 * @param socket file descriptor
 * @param request struct to be populated
 */
int parse_http_request(client *conn);

/*
 * Routes requests to static assets or websocket upgrade
 * @return error code (0 if successful)
 * @param socket file descriptor
 * @param request struct to be populated
 */
int route_request(client *conn);

/**
 * Upgrades the connection to websocket protocol
 * @return void*
 * @param socket file descriptor
 * @param Httprequest struct
 */
void upgrade_conn(client *conn);

#endif
