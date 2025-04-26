#ifndef HTTP_H
#define HTTP_H

#include "structs.h"
#include <stdio.h>

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
 * Handles sending text/html content via the HTTP protocol. This function
 * frees the response memory for ease of use
 * @return error code (0 if successful)
 * @param socket file descriptor
 * @param response object
 */
int send_http_response(int fd, http_response *res);

/*
 * Helper for building an http response object
 * Adds some headers by default for our protocol
 */
http_response *build_http_response(int status_code, char *message,
                                   char *content, int content_len,
                                   char *content_type);

/*
 * Adds a header to an http response object
 */
void add_header(http_response *response, char *key, char *value);

/**
 * validates that the request is authorized with a c-game-auth cookie
 * in the request
 */
int validate_request(client* conn);

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
