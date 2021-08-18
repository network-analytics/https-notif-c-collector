#ifndef H_UNYTE_HTTPS_COLLECTOR
#define H_UNYTE_HTTPS_COLLECTOR

#include <stdint.h>
#include <stdbool.h>
#include "unyte_https_queue.h"
#include "unyte_https_utils.h"
#include "unyte_server_daemon.h"

typedef struct
{
  unyte_https_queue_t *queue;
  struct unyte_daemon *https_daemon;
  unyte_https_sock_t *sock_conn;
  // unyte_https_queue_t *monitoring_queue;
  // pthread_t *main_thread;
  // int *sockfd;
} unyte_https_collector_t;

typedef struct
{
  char *address;
  char *port;
  char *key_pem;
  char *cert_pem;
  uint output_queue_size;     // output queue size
  bool disable_json_encoding; // disable json encoding messages
  bool disable_xml_encoding;  // disable xml encoding messages
  uint64_t sock_buff_size;    // socket buffer size
  uint sock_listen_backlog;   // socket listen backlog parameter (max lenght of queue pending connections)
} unyte_https_options_t;

typedef struct
{
  int socket_fd; // socket file descriptor
  char *key_pem;
  char *cert_pem;
  uint output_queue_size;     // output queue size
  bool disable_json_encoding; // disable json encoding messages
  bool disable_xml_encoding;  // disable xml encoding messages
} unyte_https_sk_options_t;

/**
 * Starts all threads of the collector with the given options.
 */
unyte_https_collector_t *unyte_https_start_collector(unyte_https_options_t *options);

/**
 * Starts all the threads of the collector on the given socket file descriptor.
 */
unyte_https_collector_t *unyte_https_start_collector_sk(unyte_https_sk_options_t *options);

int unyte_https_stop_collector(unyte_https_collector_t *collector);
int unyte_https_free_collector(unyte_https_collector_t *collector);
int unyte_https_free_msg(unyte_https_msg_met_t *msg);
char *unyte_udp_notif_version();

#endif
