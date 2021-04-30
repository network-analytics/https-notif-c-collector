#include <stdio.h>
#include <stdlib.h>
#include "unyte_https_collector.h"
#include "unyte_https_version.h"
#include "unyte_https_defaults.h"
#include "unyte_https_queue.h"

// MHD
#include <string.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <microhttpd.h>
#define PORT 8888

#define PAGE "<html><head><title>libmicrohttpd demo</title>" \
             "</head><body>libmicrohttpd demo</body></html>"
// end MHD

static enum MHD_Result
ahc_echo(void *cls,
         struct MHD_Connection *connection,
         const char *url,
         const char *method,
         const char *version,
         const char *upload_data,
         size_t *upload_data_size,
         void **ptr)
{
  static int dummy;
  const char *page = cls;
  struct MHD_Response *response;
  int ret;

  if (0 != strcmp(method, "GET"))
  {
    printf("GETTT!\n");
    return MHD_NO; /* unexpected method */
  }
  if (&dummy != *ptr)
  {
    // The first time only the headers are valid, do not respond in the first round...
    *ptr = &dummy;
    return MHD_YES;
  }
  if (0 != *upload_data_size)
    return MHD_NO; /* upload data in a GET!? */
  *ptr = NULL;     /* clear context pointer */
  response = MHD_create_response_from_buffer(strlen(page),
                                             (void *)page,
                                             MHD_RESPMEM_PERSISTENT);
  ret = MHD_queue_response(connection,
                           MHD_HTTP_OK,
                           response);
  MHD_destroy_response(response);
  return ret;
}

unyte_https_collector_t *unyte_start_collector(unyte_https_options_t *options)
{
  printf("Starting collector on %s:%d\n", options->address, options->port);

  unyte_https_collector_t *collector = (unyte_https_collector_t *)malloc(sizeof(unyte_https_collector_t));

  //TODO: output_queue size from options
  collector->queue = unyte_https_queue_init(DF_OUTPUT_QUEUE_SIZE);
  struct MHD_Daemon *d = MHD_start_daemon(MHD_USE_THREAD_PER_CONNECTION,
                                          options->port,
                                          NULL,
                                          NULL,
                                          &ahc_echo,
                                          PAGE,
                                          MHD_OPTION_END);

  if (d == NULL)
    return NULL;

  //TODO: getc to not exit program
  (void)getc(stdin);

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
