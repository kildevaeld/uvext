#ifndef UTILS_H
#define UTILS_H
#include <dirent.h>
#include <stdbool.h>
#include <stdlib.h>

#define PATH_MAX 1024 /* max bytes in pathname */

typedef struct entry {
    char path[PATH_MAX + 1];
    int64_t size;
} entry_t; 

typedef bool (*listdir_cb)(struct dirent*, const char* path, void *data);

bool listdir(const char *path, listdir_cb fn, void *data);

#define PATH_MAX 1024 /* max bytes in pathname */

void append_path(char *buf, size_t size, const char *l, const char *r);

uint64_t size_of_path(const char *path);

#endif