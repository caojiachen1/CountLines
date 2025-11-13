#ifndef WEBSERVER_H
#define WEBSERVER_H

#include "countlines.h"

#define WEB_PORT 8080
#define WEB_DIR "web"
#define MAX_REQUEST_SIZE 8192
#define MAX_RESPONSE_SIZE 65536

// Start the web server
int start_web_server(int port);

// HTTP request handler
void handle_http_request(int client_socket);

// API endpoint handlers
void handle_api_count(int client_socket, const char *query_string);

// Helper functions
void send_http_response(int client_socket, const char *status, const char *content_type, const char *body);
void send_file_response(int client_socket, const char *filepath);
void url_decode(char *dst, const char *src);
char* get_query_param(const char *query_string, const char *param);

#endif // WEBSERVER_H
