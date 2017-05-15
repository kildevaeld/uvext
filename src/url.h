#ifndef UVEXT_URL_H
#define UVEXT_URL_H

/**
 * Dependencies
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * url.h version
 */

#define URL_VERSION 0.0.2

/**
 * `url_data` struct that defines parts
 * of a parsed URL such as host and protocol
 */

typedef struct uv_url_data_s {
  char *href;
  char *protocol;
  char *host;
  char *auth;
  char *hostname;
  char *pathname;
  char *search;
  char *path;
  char *hash;
  char *query;
  char *port;
} uv_url_data_t;

uv_url_data_t *url_parse(char *url);

bool url_is_protocol(char *str);

bool url_is_ssh(char *str);

char *url_get_protocol(char *url);

char *url_get_hostname(char *url);

char *url_get_host(char *url);

char *url_get_pathname(char *url);

char *url_get_path(char *url);

char *url_get_search(char *url);

char *url_get_query(char *url);

char *url_get_hash(char *url);

char *url_get_port(char *url);

void url_inspect(char *url);

void url_data_inspect(uv_url_data_t *data);

void url_free(uv_url_data_t *data);

#endif