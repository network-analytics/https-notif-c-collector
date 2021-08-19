#ifndef UNYTE_HTTPS_UTILS_H
#define UNYTE_HTTPS_UTILS_H

#include <stdio.h>
#include "unyte_server_daemon.h"

typedef struct unyte_msg_with_metadata
{
  struct sockaddr_storage *src; // source address
  char *payload;                // payload buffer
  size_t payload_length;        // payload buffer size
  char *content_type;           // payload buffer content type (application/json | application/xml)
} unyte_https_msg_met_t;

void print_https_notif_msg(unyte_https_msg_met_t *msg, FILE *std);

struct sockaddr_storage *unyte_https_get_src(unyte_https_msg_met_t *msg);
char *unyte_https_get_payload(unyte_https_msg_met_t *msg);
size_t unyte_https_get_payload_length(unyte_https_msg_met_t *msg);
char *unyte_https_get_content_type(unyte_https_msg_met_t *msg);

#endif