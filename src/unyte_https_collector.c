#include <stdio.h>
#include <stdlib.h>
#include "unyte_https_collector.h"
#include "unyte_https_version.h"
#include "unyte_https_defaults.h"
#include "unyte_https_queue.h"
#include "unyte_server_daemon.h"

unyte_https_collector_t *unyte_start_collector(unyte_https_options_t *options)
{
  printf("Starting collector on %s:%d\n", options->address, options->port);

  unyte_https_collector_t *collector = (unyte_https_collector_t *)malloc(sizeof(unyte_https_collector_t));

  //TODO: output_queue size from options
  unyte_https_queue_t *queue = unyte_https_queue_init(DF_OUTPUT_QUEUE_SIZE);
  
  collector->queue = queue;
  collector->https_daemon = start_https_server_daemon(options->port, queue);

  return collector;
}

int unyte_free_collector(unyte_https_collector_t *collector)
{
  while (unyte_https_is_queue_empty(collector->queue) != 0)
    free(unyte_https_queue_read(collector->queue));

  free(collector->queue->data);
  free(collector->queue);
  free(collector);
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
