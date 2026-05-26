#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <limits.h>
#include "http.h"
#include "files.h"
#include "mime.h"

#define REQ_SIZE 8192
#define HDR_SIZE 1024

static int send_all(int fd, const void *buf, unsigned long len)
{
    const char *p = buf;
    unsigned long sent = 0;
    long n;

    while (sent < len) {
        n = write(fd, p + sent, len - sent);
        if (n <= 0)
            return -1;
        sent += n;
    }

    return 0;
}

static int header_exists(const char *req, const char *name)
{
    const char *p = req;
    unsigned long name_len = strlen(name);

    while ((p = strchr(p, '\n')) != NULL) {
        p++;

        while (*p == ' ' || *p == '\t' || *p == '\r')
            p++;

        if (strncasecmp(p, name, name_len) == 0)
            return 1;
    }

    return 0;
}

static int request_wants_close(const char *req, const char *version)
{
    if (header_exists(req, "Connection:")) {
        if (strcasestr(req, "Connection: close") != NULL)
            return 1;
    }

    if (strcmp(version, "HTTP/1.0") == 0)
        return 1;

    return 0;
}

static void send_error(int fd, int code, const char *reason, int close_conn)
{
    char body[256];
    char header[HDR_SIZE];

    snprintf(body, sizeof(body),
             "<html><body><h1>%d %s</h1></body></html>\n",
             code, reason);

    snprintf(header, sizeof(header),
             "HTTP/1.1 %d %s\r\n"
             "Content-Type: text/html\r\n"
             "Content-Length: %lu\r\n"
             "Connection: %s\r\n"
             "\r\n",
             code, reason, strlen(body),
             close_conn ? "close" : "keep-alive");

    send_all(fd, header, strlen(header));
    send_all(fd, body, strlen(body));
}

static int send_file(int fd, const char *path, int close_conn)
{
    int file_fd;
    char header[HDR_SIZE];
    char buf[4096];
    struct stat st;
    long n;
    const char *mime;

    file_fd = open(path, O_RDONLY);
    if (file_fd < 0)
        return -1;

    if (fstat(file_fd, &st) < 0) {
        close(file_fd);
        return -1;
    }

    mime = get_mime_type(path);

    snprintf(header, sizeof(header),
             "HTTP/1.1 200 OK\r\n"
             "Content-Type: %s\r\n"
             "Content-Length: %ld\r\n"
             "Connection: %s\r\n"
             "\r\n",
             mime, (long)st.st_size,
             close_conn ? "close" : "keep-alive");

    if (send_all(fd, header, strlen(header)) < 0) {
        close(file_fd);
        return -1;
    }

    while ((n = read(file_fd, buf, sizeof(buf))) > 0) {
        if (send_all(fd, buf, n) < 0) {
            close(file_fd);
            return -1;
        }
    }

    close(file_fd);
    return 0;
}

int handle_http_request(int client_fd, const char *root_dir)
{
    char req[REQ_SIZE];
    char method[16];
    char path[1024];
    char version[32];
    char file_path[PATH_MAX];
    int n;
    int status;
    int close_conn;

    n = read(client_fd, req, sizeof(req) - 1);
    if (n <= 0)
        return 0;

    req[n] = '\0';

    if (n == (int)sizeof(req) - 1 && strstr(req, "\r\n\r\n") == NULL) {
        send_error(client_fd, 400, "Bad Request", 1);
        return 0;
    }

    if (sscanf(req, "%15s %1023s %31s", method, path, version) != 3) {
        send_error(client_fd, 400, "Bad Request", 1);
        return 0;
    }

    if (strcmp(version, "HTTP/1.1") != 0 && strcmp(version, "HTTP/1.0") != 0) {
        send_error(client_fd, 400, "Bad Request", 1);
        return 0;
    }

    if (strcmp(version, "HTTP/1.1") == 0 && !header_exists(req, "Host:")) {
        send_error(client_fd, 400, "Bad Request", 1);
        return 0;
    }

    close_conn = request_wants_close(req, version);

    if (strcmp(method, "GET") != 0) {
        send_error(client_fd, 405, "Method Not Allowed", 1);
        return 0;
    }

    status = resolve_file_path(root_dir, path, file_path, sizeof(file_path));

    if (status == 400) {
        send_error(client_fd, 400, "Bad Request", 1);
        return 0;
    }

    if (status == 403) {
        send_error(client_fd, 403, "Forbidden", 1);
        return 0;
    }

    if (status == 404) {
        send_error(client_fd, 404, "Not Found", close_conn);
        return close_conn ? 0 : 1;
    }

    if (status != 200) {
        send_error(client_fd, 500, "Internal Server Error", 1);
        return 0;
    }

    if (send_file(client_fd, file_path, close_conn) < 0) {
        send_error(client_fd, 500, "Internal Server Error", 1);
        return 0;
    }

    return close_conn ? 0 : 1;
}
