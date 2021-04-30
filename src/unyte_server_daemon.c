#include "unyte_server_daemon.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <microhttpd.h>
#include "unyte_https_queue.h"

#define PAGE "<html><head><title>libmicrohttpd demo</title>" \
             "</head><body>libmicrohttpd demo</body></html>"

enum MHD_Result not_implemented(struct MHD_Connection *connection)
{
  const char *page = NOT_IMPLEMENTED;
  struct MHD_Response *response = MHD_create_response_from_buffer(strlen(page), (void *)page, MHD_RESPMEM_PERSISTENT);
  int ret = MHD_queue_response(connection, MHD_HTTP_NOT_IMPLEMENTED, response);
  MHD_add_response_header(response, CONTENT_TYPE, MIME_JSON);
  MHD_destroy_response(response);
  return ret;
}

enum MHD_Result get_capabilities(struct MHD_Connection *connection, unyte_https_capabilities_t *capabilities)
{
  struct MHD_Response *response;
  const char *req_content_type = MHD_lookup_connection_value(connection, MHD_HEADER_KIND, CONTENT_TYPE);
  // if application/xml send xml format else json
  if (0 == strcmp(req_content_type, MIME_XML))
  {
    response = MHD_create_response_from_buffer(strlen(XML_CAPABILITIES), (void *)XML_CAPABILITIES, MHD_RESPMEM_PERSISTENT);
    MHD_add_response_header(response, CONTENT_TYPE, MIME_XML);
  }
  else
  {
    response = MHD_create_response_from_buffer(strlen(JSON_CAPABILITIES), (void *)JSON_CAPABILITIES, MHD_RESPMEM_PERSISTENT);
    MHD_add_response_header(response, CONTENT_TYPE, MIME_JSON);
  }
  int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
  MHD_destroy_response(response);
  return ret;
}

enum MHD_Result post_notification(struct MHD_Connection *connection)
{
  // TODO:
}

static enum MHD_Result dispatcher(void *cls,
                                  struct MHD_Connection *connection,
                                  const char *url,
                                  const char *method,
                                  const char *version,
                                  const char *upload_data,
                                  size_t *upload_data_size,
                                  void **ptr)
{
  static int dummy;
  daemon_input_t *input = (daemon_input_t *)cls;
  const char *page = PAGE;
  struct MHD_Response *response;
  int ret;

  if (&dummy != *ptr)
  {
    // The first time only the headers are valid, do not respond in the first round...
    *ptr = &dummy;
    return MHD_YES;
  }
  if ((0 != *upload_data_size) && (0 != strcmp(method, "GET")))
    return MHD_NO; /* upload data in a GET!? */
  *ptr = NULL;     /* clear context pointer */

  printf("Method |%s|\n", method);

  if (0 == strcmp(method, "GET"))
    // return not_implemented(connection);
    return get_capabilities(connection, input->capabilities);
  else if (0 == strcmp(method, "POST"))
    return post_notification(connection);
  else
    return not_implemented(connection);
}

struct MHD_Daemon *start_https_server_daemon(uint port, unyte_https_queue_t *output_queue)
{
  unyte_https_capabilities_t *capabilities = init_capabilities_buff();

  daemon_input_t *daemon_in = (daemon_input_t *)malloc(sizeof(daemon_input_t));
  daemon_in->output_queue = output_queue;
  daemon_in->capabilities = capabilities;

  struct MHD_Daemon *d = MHD_start_daemon(MHD_USE_THREAD_PER_CONNECTION,
                                          port,
                                          NULL,
                                          NULL,
                                          &dispatcher,
                                          daemon_in,
                                          MHD_OPTION_END);

  if (d == NULL)
    return NULL;

  return d;
}
