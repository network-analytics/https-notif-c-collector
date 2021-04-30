#ifndef UNYTE_SERVER_DAEMOM_H
#define UNYTE_SERVER_DAEMOM_H

#include <sys/types.h>
#include "unyte_https_queue.h"
#include "unyte_https_capabilities.h"

#define NOT_IMPLEMENTED "{\"error\": \"Not implemented\"}"

#define CONTENT_TYPE "Content-Type"
#define MIME_JSON "application/json"
#define MIME_XML "application/xml"

typedef struct daemon_input
{
  unyte_https_queue_t *output_queue;
  unyte_https_capabilities_t *capabilities;
} daemon_input_t;

struct MHD_Daemon *start_https_server_daemon(uint port, unyte_https_queue_t *output_queue);

#endif