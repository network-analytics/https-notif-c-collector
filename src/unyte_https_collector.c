#include <stdio.h>
#include <stdlib.h>
#include "unyte_https_collector.h"
#include "unyte_https_version.h"
#include "unyte_https_defaults.h"
#include "unyte_https_queue.h"
#include "unyte_server_daemon.h"

void set_defaults(unyte_https_options_t *options)
{
  if (options == NULL)
  {
    printf("Invalid options.\n");
    exit(EXIT_FAILURE);
  }
  if (options->address == NULL)
  {
    printf("Address is mandatory.\n");
    exit(EXIT_FAILURE);
  }
  if ((options->cert_pem == NULL) || (options->key_pem == NULL))
  {
    printf("TLS certs are not valid\n");
    exit(EXIT_FAILURE);
  }
}

unyte_https_collector_t *unyte_start_collector(unyte_https_options_t *options)
{
  unyte_https_collector_t *collector = (unyte_https_collector_t *)malloc(sizeof(unyte_https_collector_t));

  set_defaults(options);

  //TODO: output_queue size from options
  unyte_https_queue_t *queue = unyte_https_queue_init(DF_OUTPUT_QUEUE_SIZE);
  struct unyte_daemon *daemon = start_https_server_daemon(options->port, queue, options->key_pem, options->cert_pem);

  if (queue == NULL || collector == NULL)
  {
    printf("Malloc error\n");
    exit(EXIT_FAILURE);
  }

  if (daemon == NULL)
  {
    printf("Error creating daemon\n");
    exit(EXIT_FAILURE);
  }

  collector->queue = queue;
  collector->https_daemon = daemon;

  return collector;
}

int unyte_stop_collector(unyte_https_collector_t *collector)
{
  return stop_https_server_daemon(collector->https_daemon);
}

int unyte_free_collector(unyte_https_collector_t *collector)
{
  while (unyte_https_is_queue_empty(collector->queue) != 0)
    free(unyte_https_queue_read(collector->queue));

  free(collector->queue->data);
  free(collector->queue);
  free(collector->https_daemon->daemon_in->capabilities);
  free(collector->https_daemon->daemon_in);
  free(collector->https_daemon);
  free(collector);
  return 0;
}

int unyte_https_free_msg(unyte_https_msg_met_t *msg)
{
  free(msg->payload);
  free(msg);
  return 0;
}

int get_int_len(int value)
{
  int l = 1;
  while (value > 9)
  {
    l++;
    value /= 10;
  }
  return l;
}

char *unyte_udp_notif_version()
{
  int major_len = get_int_len(UNYTE_HTTPS_NOTIF_VERSION_MAJOR);
  int minor_len = get_int_len(UNYTE_HTTPS_NOTIF_VERSION_MINOR);
  int patch_len = get_int_len(UNYTE_HTTPS_NOTIF_VERSION_PATCH);
  uint len = major_len + minor_len + patch_len + 3; // 2 points and 1 end of string
  char *version = (char *)malloc(len * sizeof(char));
  sprintf(version, "%d.%d.%d", UNYTE_HTTPS_NOTIF_VERSION_MAJOR, UNYTE_HTTPS_NOTIF_VERSION_MINOR, UNYTE_HTTPS_NOTIF_VERSION_PATCH);
  return version;
}
