#include "webserver.h"
#include <ctype.h>
#include <time.h>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef int socklen_t;
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #define closesocket close
    #define SOCKET int
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
#endif

// URL decode helper
void url_decode(char *dst, const char *src) {
    char a, b;
    while (*src) {
        if ((*src == '%') && ((a = src[1]) && (b = src[2])) && (isxdigit(a) && isxdigit(b))) {
            if (a >= 'a') a -= 'a'-'A';
            if (a >= 'A') a -= ('A' - 10);
            else a -= '0';
            if (b >= 'a') b -= 'a'-'A';
            if (b >= 'A') b -= ('A' - 10);
            else b -= '0';
            *dst++ = 16*a+b;
            src+=3;
        } else if (*src == '+') {
            *dst++ = ' ';
            src++;
        } else {
            *dst++ = *src++;
        }
    }
    *dst++ = '\0';
}

// Get query parameter value
char* get_query_param(const char *query_string, const char *param) {
    if (!query_string || !param) return NULL;
    
    static char value[1024];
    char search[256];
    snprintf(search, sizeof(search), "%s=", param);
    
    const char *start = strstr(query_string, search);
    if (!start) return NULL;
    
    start += strlen(search);
    const char *end = strchr(start, '&');
    
    size_t len;
    if (end) {
        len = end - start;
    } else {
        len = strlen(start);
    }
    
    if (len >= sizeof(value)) len = sizeof(value) - 1;
    strncpy(value, start, len);
    value[len] = '\0';
    
    char decoded[1024];
    url_decode(decoded, value);
    strcpy(value, decoded);
    
    return value;
}

// Send HTTP response
void send_http_response(int client_socket, const char *status, const char *content_type, const char *body) {
    char response[MAX_RESPONSE_SIZE];
    int body_len = body ? strlen(body) : 0;
    
    snprintf(response, sizeof(response),
        "HTTP/1.1 %s\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %d\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Connection: close\r\n"
        "\r\n"
        "%s",
        status, content_type, body_len, body ? body : "");
    
    send(client_socket, response, strlen(response), 0);
}

// Send file response
void send_file_response(int client_socket, const char *filepath) {
    FILE *file = fopen(filepath, "rb");
    if (!file) {
        const char *error_body = "<html><body><h1>404 Not Found</h1></body></html>";
        send_http_response(client_socket, "404 Not Found", "text/html", error_body);
        return;
    }
    
    // Determine content type
    const char *content_type = "text/html";
    const char *ext = strrchr(filepath, '.');
    if (ext) {
        if (strcmp(ext, ".html") == 0) content_type = "text/html";
        else if (strcmp(ext, ".css") == 0) content_type = "text/css";
        else if (strcmp(ext, ".js") == 0) content_type = "application/javascript";
        else if (strcmp(ext, ".json") == 0) content_type = "application/json";
    }
    
    // Read file content
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char *content = malloc(file_size + 1);
    if (!content) {
        fclose(file);
        const char *error_body = "<html><body><h1>500 Internal Server Error</h1></body></html>";
        send_http_response(client_socket, "500 Internal Server Error", "text/html", error_body);
        return;
    }
    
    size_t bytes_read = fread(content, 1, file_size, file);
    content[bytes_read] = '\0';
    fclose(file);
    
    if (bytes_read != (size_t)file_size) {
        free(content);
        const char *error_body = "<html><body><h1>500 Internal Server Error</h1></body></html>";
        send_http_response(client_socket, "500 Internal Server Error", "text/html", error_body);
        return;
    }
    
    // Send response
    char header[1024];
    snprintf(header, sizeof(header),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %ld\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Connection: close\r\n"
        "\r\n",
        content_type, file_size);
    
    send(client_socket, header, strlen(header), 0);
    send(client_socket, content, file_size, 0);
    
    free(content);
}

// Handle API count endpoint
void handle_api_count(int client_socket, const char *query_string) {
    char *path_param = get_query_param(query_string, "path");
    
    if (!path_param || strlen(path_param) == 0) {
        const char *error_json = "{\"error\":\"Missing path parameter\"}";
        send_http_response(client_socket, "400 Bad Request", "application/json", error_json);
        return;
    }
    
    // Create exclude list
    ExcludeList *exclude_list = create_exclude_list();
    if (!exclude_list) {
        const char *error_json = "{\"error\":\"Failed to initialize exclude list\"}";
        send_http_response(client_socket, "500 Internal Server Error", "application/json", error_json);
        return;
    }
    
    // Add default exclusions
    add_exclude_pattern(exclude_list, ".git");
    add_exclude_pattern(exclude_list, ".svn");
    add_exclude_pattern(exclude_list, ".hg");
    add_exclude_pattern(exclude_list, "node_modules");
    add_exclude_pattern(exclude_list, "__pycache__");
    add_exclude_pattern(exclude_list, ".vs");
    add_exclude_pattern(exclude_list, ".vscode");
    
    // Parse exclude patterns from query string
    char *query_copy = malloc(strlen(query_string) + 1);
    strcpy(query_copy, query_string);
    
    char *param_start = query_copy;
    while (param_start) {
        if (strncmp(param_start, "exclude=", 8) == 0) {
            char *value_start = param_start + 8;
            char *param_end = strchr(value_start, '&');
            
            char exclude_value[256];
            if (param_end) {
                size_t len = param_end - value_start;
                if (len >= sizeof(exclude_value)) len = sizeof(exclude_value) - 1;
                strncpy(exclude_value, value_start, len);
                exclude_value[len] = '\0';
            } else {
                strncpy(exclude_value, value_start, sizeof(exclude_value) - 1);
                exclude_value[sizeof(exclude_value) - 1] = '\0';
            }
            
            char decoded[256];
            url_decode(decoded, exclude_value);
            add_exclude_pattern(exclude_list, decoded);
        }
        
        param_start = strchr(param_start, '&');
        if (param_start) param_start++;
    }
    
    free(query_copy);
    
    // Check if path exists
#ifdef _WIN32
    DWORD attributes = GetFileAttributes(path_param);
    if (attributes == INVALID_FILE_ATTRIBUTES) {
        free_exclude_list(exclude_list);
        const char *error_json = "{\"error\":\"Path does not exist\"}";
        send_http_response(client_socket, "404 Not Found", "application/json", error_json);
        return;
    }
    if (!(attributes & FILE_ATTRIBUTE_DIRECTORY)) {
        free_exclude_list(exclude_list);
        const char *error_json = "{\"error\":\"Path is not a directory\"}";
        send_http_response(client_socket, "400 Bad Request", "application/json", error_json);
        return;
    }
#else
    struct stat path_stat;
    if (stat(path_param, &path_stat) != 0) {
        free_exclude_list(exclude_list);
        const char *error_json = "{\"error\":\"Path does not exist\"}";
        send_http_response(client_socket, "404 Not Found", "application/json", error_json);
        return;
    }
    if (!S_ISDIR(path_stat.st_mode)) {
        free_exclude_list(exclude_list);
        const char *error_json = "{\"error\":\"Path is not a directory\"}";
        send_http_response(client_socket, "400 Bad Request", "application/json", error_json);
        return;
    }
#endif
    
    // Count lines
    CountResult result = {0, 0, 0, 0, 0};
    clock_t start_time = clock();
    count_lines_in_directory(path_param, exclude_list, &result);
    clock_t end_time = clock();
    double elapsed_time = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
    
    // Build JSON response
    char json_response[4096];
    snprintf(json_response, sizeof(json_response),
        "{"
        "\"total_files\":%llu,"
        "\"total_lines\":%llu,"
        "\"code_lines\":%llu,"
        "\"comment_lines\":%llu,"
        "\"blank_lines\":%llu,"
        "\"processing_time\":%.3f,"
        "\"target_path\":\"%s\""
        "}",
        result.total_files,
        result.total_lines,
        result.code_lines,
        result.comment_lines,
        result.blank_lines,
        elapsed_time,
        path_param
    );
    
    send_http_response(client_socket, "200 OK", "application/json", json_response);
    
    free_exclude_list(exclude_list);
}

// Handle HTTP request
void handle_http_request(int client_socket) {
    char request[MAX_REQUEST_SIZE];
    int received = recv(client_socket, request, sizeof(request) - 1, 0);
    
    if (received <= 0) {
        closesocket(client_socket);
        return;
    }
    
    request[received] = '\0';
    
    // Parse request line
    char method[16], path[1024], version[16];
    if (sscanf(request, "%15s %1023s %15s", method, path, version) != 3) {
        const char *error_body = "<html><body><h1>400 Bad Request</h1></body></html>";
        send_http_response(client_socket, "400 Bad Request", "text/html", error_body);
        closesocket(client_socket);
        return;
    }
    
    // Only handle GET requests
    if (strcmp(method, "GET") != 0) {
        const char *error_body = "<html><body><h1>405 Method Not Allowed</h1></body></html>";
        send_http_response(client_socket, "405 Method Not Allowed", "text/html", error_body);
        closesocket(client_socket);
        return;
    }
    
    // Separate path and query string
    char *query_string = strchr(path, '?');
    if (query_string) {
        *query_string = '\0';
        query_string++;
    }
    
    // Route request
    if (strncmp(path, "/api/count", 10) == 0) {
        handle_api_count(client_socket, query_string ? query_string : "");
    } else if (strcmp(path, "/") == 0 || strcmp(path, "/index.html") == 0) {
        char filepath[MAX_PATH_LEN];
        snprintf(filepath, sizeof(filepath), "%s/index.html", WEB_DIR);
        send_file_response(client_socket, filepath);
    } else {
        char filepath[MAX_PATH_LEN];
        snprintf(filepath, sizeof(filepath), "%s%s", WEB_DIR, path);
        send_file_response(client_socket, filepath);
    }
    
    closesocket(client_socket);
}

// Start web server
int start_web_server(int port) {
#ifdef _WIN32
    WSADATA wsa_data;
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
        fprintf(stderr, "Failed to initialize Winsock\n");
        return 1;
    }
#endif
    
    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) {
        fprintf(stderr, "Failed to create socket\n");
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }
    
    // Set socket options to reuse address
    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
    
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        fprintf(stderr, "Failed to bind socket to port %d\n", port);
        closesocket(server_socket);
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }
    
    if (listen(server_socket, 10) == SOCKET_ERROR) {
        fprintf(stderr, "Failed to listen on socket\n");
        closesocket(server_socket);
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }
    
    printf("\n");
    printf("======================================\n");
    printf("  CountLines Web Server Started\n");
    printf("======================================\n");
    printf("  Port: %d\n", port);
    printf("  URL:  http://localhost:%d\n", port);
    printf("======================================\n");
    printf("Press Ctrl+C to stop the server\n\n");
    
    // Main server loop
    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        
        SOCKET client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
        if (client_socket == INVALID_SOCKET) {
            continue;
        }
        
        handle_http_request(client_socket);
    }
    
    closesocket(server_socket);
#ifdef _WIN32
    WSACleanup();
#endif
    
    return 0;
}
