#ifndef H_UNYTE_HTTPS_COLLECTOR
#define H_UNYTE_HTTPS_COLLECTOR

#include <stdint.h>
#include "unyte_https_queue.h"
#include "unyte_https_utils.h"

typedef struct
{
  unyte_https_queue_t *queue;
  struct unyte_daemon *https_daemon;
  // unyte_https_queue_t *monitoring_queue;
  // pthread_t *main_thread;
  // int *sockfd;
} unyte_https_collector_t;

typedef struct
{
  char *address;
  uint16_t port;
  char *key_pem;
  char *cert_pem;
} unyte_https_options_t;

unyte_https_collector_t *unyte_start_collector(unyte_https_options_t *options);
int unyte_stop_collector(unyte_https_collector_t *collector);
int unyte_free_collector(unyte_https_collector_t *collector);
int unyte_https_free_msg(unyte_https_msg_met_t * msg);
char *unyte_udp_notif_version();

#endif
