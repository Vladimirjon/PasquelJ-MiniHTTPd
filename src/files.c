#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <limits.h>
#include "files.h"

int resolve_file_path(const char *root_dir, const char *url_path,
                      char *out_path, unsigned long out_size)
{
    char root_real[PATH_MAX];
    char candidate[PATH_MAX];
    char candidate_real[PATH_MAX];
    struct stat st;

    if (strstr(url_path, "..") != NULL)
        return 403;

    if (realpath(root_dir, root_real) == NULL)
        return 500;

    if (strcmp(url_path, "/") == 0) {
        snprintf(candidate, sizeof(candidate), "%s/index.html", root_real);
    } else {
        snprintf(candidate, sizeof(candidate), "%s%s", root_real, url_path);
    }

    if (realpath(candidate, candidate_real) == NULL)
        return 404;

    if (strncmp(candidate_real, root_real, strlen(root_real)) != 0)
        return 403;

    if (stat(candidate_real, &st) < 0)
        return 404;

    if (!S_ISREG(st.st_mode))
        return 403;

    snprintf(out_path, out_size, "%s", candidate_real);
    return 200;
}
