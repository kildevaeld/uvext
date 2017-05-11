#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

bool listdir(const char *path, listdir_cb fn, void *data) {
  struct dirent *entry;
  DIR *dp;

  dp = opendir(path);
  if (dp == NULL) {
    perror("opendir");
    return false;
  }

  while ((entry = readdir(dp))) {
    if (strcmp(".", entry->d_name) == 0 || strcmp("..", entry->d_name) == 0)
      continue;
    if (!fn(entry, path, data))
      return 0;
  }

  closedir(dp);
  return true;
}

void append_path(char *buf, size_t size, const char *l, const char *r) {
  if (l[strlen(l) - 1] != '/') {
    snprintf(buf, size, "%s/%s", l, r);
  } else {
    snprintf(buf, size, "%s%s", l, r);
  }
}

uint64_t size_of_path(const char *path) {

  struct stat file_stats;
  if (stat(path, &file_stats) == -1)
    return -1;
  if (!S_ISDIR(file_stats.st_mode)) {
    return file_stats.st_size;
  }

  struct dirent *entry;
  DIR *dp;
  int64_t size = 0;
  dp = opendir(path);
  if (dp == NULL) {
    perror("opendir");
    return false;
  }

  while ((entry = readdir(dp))) {
    if (strcmp(".", entry->d_name) == 0 || strcmp("..", entry->d_name) == 0)
      continue;
    char buf[PATH_MAX + 1];
    append_path(buf, sizeof buf, path, entry->d_name);
    if (entry->d_type == DT_DIR) {

      size += size_of_path(buf);
    } else {
      if (stat(buf, &file_stats) == -1) {
        
        size = -1;
        break;
      }
      
      size += file_stats.st_size;
        
    }
  }

  closedir(dp);
  return size;
}