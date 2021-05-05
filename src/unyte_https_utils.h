#ifndef UNYTE_HTTPS_UTILS_H
#define UNYTE_HTTPS_UTILS_H

#include <stdio.h>
#include "unyte_server_daemon.h"

typedef struct unyte_msg_with_metadata
{
  uint16_t src_port;     // source port
  uint32_t src_addr;     // source address IPv4
  char *payload;         // payload buffer
  size_t payload_length; // payload buffer size
  char *content_type;    // payload buffer content type (application/json | application/xml)
} unyte_https_msg_met_t;

void print_https_notif_msg(unyte_https_msg_met_t *msg, FILE *std);

#endif