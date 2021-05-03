#ifndef UNYTE_HTTPS_UTILS_H
#define UNYTE_HTTPS_UTILS_H

#include <stdio.h>
#include "unyte_server_daemon.h"

typedef struct unyte_msg_with_metadata
{
  uint16_t src_port;
  uint32_t src_addr;
  char *payload;
  size_t payload_length;
} unyte_https_msg_met_t;

void print_https_notif_msg(unyte_https_msg_met_t *msg, FILE *std);

#endif