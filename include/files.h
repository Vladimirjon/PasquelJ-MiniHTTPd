#ifndef FILES_H
#define FILES_H

int resolve_file_path(const char *root_dir, const char *url_path,
                      char *out_path, unsigned long out_size);

#endif
