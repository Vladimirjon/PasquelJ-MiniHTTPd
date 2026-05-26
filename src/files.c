#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <limits.h>
#include "files.h"

static int build_candidate_path(char *dst, unsigned long dst_size,
                                const char *root_real,
                                const char *url_path)
{
    unsigned long root_len = strlen(root_real);
    unsigned long path_len = strlen(url_path);
    const char *suffix;

    if (strcmp(url_path, "/") == 0)
        suffix = "/index.html";
    else
        suffix = url_path;

    path_len = strlen(suffix);

    if (root_len + path_len + 1 > dst_size)
        return -1;

    memcpy(dst, root_real, root_len);
    memcpy(dst + root_len, suffix, path_len);
    dst[root_len + path_len] = '\0';

    return 0;
}

int resolve_file_path(const char *root_dir, const char *url_path,
                      char *out_path, unsigned long out_size)
{
    char root_real[PATH_MAX];
    char candidate[PATH_MAX];
    char candidate_real[PATH_MAX];
    struct stat st;
    unsigned long root_len;

    if (url_path == NULL || url_path[0] != '/')
        return 400;

    if (strlen(url_path) >= 1024)
        return 400;

    if (strstr(url_path, "..") != NULL)
        return 403;

    if (realpath(root_dir, root_real) == NULL)
        return 500;

    if (build_candidate_path(candidate, sizeof(candidate), root_real, url_path) < 0)
        return 400;

    if (realpath(candidate, candidate_real) == NULL)
        return 404;

    root_len = strlen(root_real);

    if (strncmp(candidate_real, root_real, root_len) != 0)
        return 403;

    if (candidate_real[root_len] != '/' && candidate_real[root_len] != '\0')
        return 403;

    if (stat(candidate_real, &st) < 0)
        return 404;

    if (!S_ISREG(st.st_mode))
        return 403;

    if (strlen(candidate_real) + 1 > out_size)
        return 500;

    memcpy(out_path, candidate_real, strlen(candidate_real) + 1);

    return 200;
}
