#ifndef UNYTE_SERVER_DAEMOM_H
#define UNYTE_SERVER_DAEMOM_H

#include <sys/types.h>
#include <stdint.h>
#include "unyte_https_queue.h"
#include "unyte_https_capabilities.h"

#define UHTTPS_NOT_IMPLEMENTED "{\"error\": \"Not implemented\"}"
#define UHTTPS_BAD_REQUEST "{\"error\": \"Not valid body\"}"

#define UHTTPS_CONTENT_TYPE "Content-Type"
#define UHTTPS_MIME_JSON "application/json"
#define UHTTPS_MIME_XML "application/xml"

#define UHTTPS_POST_BUFFER_SIZE 65536
#define UHTTPS_POST_BODY_SIZE 65536

typedef struct unyte_socket
{
  struct sockaddr_in *addr; // The socket addr
  int *sockfd;              // The socket file descriptor
} unyte_https_sock_t;

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

struct unyte_daemon *start_https_server_daemon(unyte_https_sock_t *conn, unyte_https_queue_t *output_queue, const char *key_pem, const char *cert_pem, bool disable_json, bool disable_xml);
int stop_https_server_daemon(struct unyte_daemon *daemon);
void free_https_server_daemon(struct unyte_daemon *daemon);

#endif