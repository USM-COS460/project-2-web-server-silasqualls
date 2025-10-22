#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <time.h>

#define BUFFER_SIZE 4096

char document_root[512];

const char* get_mime_type(const char* path) {
    const char* ext = strrchr(path, '.');
    if (!ext) return "application/octet-stream";
    if (strcmp(ext, ".html") == 0) return "text/html";
    if (strcmp(ext, ".htm") == 0) return "text/html";
    if (strcmp(ext, ".css") == 0) return "text/css";
    if (strcmp(ext, ".jpg") == 0) return "image/jpeg";
    if (strcmp(ext, ".jpeg") == 0) return "image/jpeg";
    if (strcmp(ext, ".png") == 0) return "image/png";
    if (strcmp(ext, ".gif") == 0) return "image/gif";
    if (strcmp(ext, ".js") == 0) return "application/javascript";
    if (strcmp(ext, ".txt") == 0) return "text/plain";
    return "application/octet-stream";
}

void get_date_header(char* buffer, size_t size) {
    time_t now = time(NULL);
    struct tm tm = *gmtime(&now);
    strftime(buffer, size, "%a, %d %b %Y %H:%M:%S GMT", &tm);
}

void send_404(int client_fd) {
    char response[BUFFER_SIZE];
    const char* body = "<html><body><h1>404 Not Found</h1></body></html>\n";
    char date[128];
    get_date_header(date, sizeof(date));

    snprintf(response, sizeof(response),
        "HTTP/1.1 404 Not Found\r\n"
        "Date: %s\r\n"
        "Server: SilasServer/1.0\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: %zu\r\n"
        "\r\n"
        "%s",
        date, strlen(body), body);

    write(client_fd, response, strlen(response));
}

void send_200(int client_fd, const char* filepath) {
    int fd = open(filepath, O_RDONLY);
    if (fd < 0) {
        send_404(client_fd);
        return;
    }

    struct stat st;
    fstat(fd, &st);
    size_t filesize = st.st_size;
    const char* mime = get_mime_type(filepath);

    char date[128];
    get_date_header(date, sizeof(date));

    char header[BUFFER_SIZE];
    snprintf(header, sizeof(header),
        "HTTP/1.1 200 OK\r\n"
        "Date: %s\r\n"
        "Server: SilasServer/1.0\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %zu\r\n"
        "\r\n",
        date, mime, filesize);

    write(client_fd, header, strlen(header));

    char buffer[BUFFER_SIZE];
    ssize_t bytes;
    while ((bytes = read(fd, buffer, sizeof(buffer))) > 0) {
        write(client_fd, buffer, bytes);
    }

    close(fd);
}

void* handle_client(void* arg) {
    int client_fd = *(int*)arg;
    free(arg);

    char buffer[BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));

    int bytes = read(client_fd, buffer, sizeof(buffer) - 1);
    if (bytes <= 0) {
        close(client_fd);
        pthread_exit(NULL);
    }

    char method[8], path[512], version[16];
    sscanf(buffer, "%s %s %s", method, path, version);

    if (strcmp(method, "GET") != 0) {
        close(client_fd);
        pthread_exit(NULL);
    }

    char filepath[1024];
    snprintf(filepath, sizeof(filepath), "%s%s", document_root, path);

    if (path[strlen(path) - 1] == '/') {
        strcat(filepath, "index.html");
    }

    struct stat st;
    if (stat(filepath, &st) == -1 || S_ISDIR(st.st_mode)) {
        send_404(client_fd);
    } else {
        send_200(client_fd, filepath);
    }

    close(client_fd);
    pthread_exit(NULL);
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <port> <document_root>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);
    strncpy(document_root, argv[2], sizeof(document_root) - 1);

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 10) < 0) {
        perror("listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("HTTP server running on port %d\n", port);
    printf("Serving files from: %s\n", document_root);

    while (1) {
        int* client_fd = malloc(sizeof(int));
        *client_fd = accept(server_fd, NULL, NULL);
        if (*client_fd < 0) {
            perror("accept");
            free(client_fd);
            continue;
        }

        pthread_t tid;
        pthread_create(&tid, NULL, handle_client, client_fd);
        pthread_detach(tid);
    }

    close(server_fd);
    return 0;
}