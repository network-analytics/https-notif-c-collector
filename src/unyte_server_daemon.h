#ifndef UNYTE_SERVER_DAEMOM_H
#define UNYTE_SERVER_DAEMOM_H

#include <sys/types.h>
#include <stdint.h>
#include "unyte_https_queue.h"
#include "unyte_https_capabilities.h"

#define NOT_IMPLEMENTED "{\"error\": \"Not implemented\"}"

#define CONTENT_TYPE "Content-Type"
#define MIME_JSON "application/json"
#define MIME_XML "application/xml"

#define POST_BUFFER_SIZE 65536
#define POST_BODY_SIZE 65536

typedef struct daemon_input
{
  unyte_https_queue_t *output_queue;
  unyte_https_capabilities_t *capabilities;
} daemon_input_t;

struct unyte_daemon
{
  daemon_input_t *daemon_in;
  struct MHD_Daemon *daemon;
};

struct unyte_https_body
{
  char *buffer;
  size_t buffer_size;
};

struct unyte_daemon *start_https_server_daemon(uint port, unyte_https_queue_t *output_queue);
int stop_https_server_daemon(struct unyte_daemon *daemon);

#endif