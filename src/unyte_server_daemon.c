#include "unyte_server_daemon.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <microhttpd.h>
#include "unyte_https_queue.h"
#include "unyte_https_utils.h"

int push_body_output_queue(struct MHD_Connection *connection, unyte_https_queue_t *output_queue, char *body, size_t body_length)
{
  unyte_https_msg_met_t *msg = (unyte_https_msg_met_t *)malloc(sizeof(unyte_https_msg_met_t));
  if (msg == NULL)
  {
    printf("Malloc failed\n");
    return -1;
  }

  msg->payload = body;
  msg->payload_length = body_length;
  // TODO: src + port
  msg->src_addr = 0;
  msg->src_port = 0;
  return unyte_https_queue_write(output_queue, msg);
}

enum MHD_Result not_implemented(struct MHD_Connection *connection)
{
  const char *page = NOT_IMPLEMENTED;
  struct MHD_Response *response = MHD_create_response_from_buffer(strlen(page), (void *)page, MHD_RESPMEM_PERSISTENT);
  MHD_add_response_header(response, CONTENT_TYPE, MIME_JSON);
  enum MHD_Result ret = MHD_queue_response(connection, MHD_HTTP_NOT_IMPLEMENTED, response);
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
    response = MHD_create_response_from_buffer(capabilities->xml_length, (void *)capabilities->xml, MHD_RESPMEM_PERSISTENT);
    MHD_add_response_header(response, CONTENT_TYPE, MIME_XML);
  }
  else
  {
    response = MHD_create_response_from_buffer(capabilities->json_length, (void *)capabilities->json, MHD_RESPMEM_PERSISTENT);
    MHD_add_response_header(response, CONTENT_TYPE, MIME_JSON);
  }
  enum MHD_Result ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
  MHD_destroy_response(response);
  return ret;
}

enum MHD_Result post_notification(struct MHD_Connection *connection, unyte_https_queue_t *output_queue, struct unyte_https_body *body_buff)
{
  struct MHD_Response *response = MHD_create_response_from_buffer(0, (void *)NULL, MHD_RESPMEM_PERSISTENT);
  const char *req_content_type = MHD_lookup_connection_value(connection, MHD_HEADER_KIND, CONTENT_TYPE);
  // if application/xml send xml format else json
  if (0 == strcmp(req_content_type, MIME_XML))
    MHD_add_response_header(response, CONTENT_TYPE, MIME_XML);
  else
    MHD_add_response_header(response, CONTENT_TYPE, MIME_JSON);

  enum MHD_Result http_ret;
  // OK
  if (0 == push_body_output_queue(connection, output_queue, body_buff->buffer, body_buff->buffer_size))
  {
    http_ret = MHD_queue_response(connection, MHD_HTTP_NO_CONTENT, response);
    MHD_destroy_response(response);
    free(body_buff);
  }
  // any ret value from queue_t different from 0 --> error
  else
  {
    //TODO: What error should the collector send on error ?
    http_ret = MHD_queue_response(connection, MHD_HTTP_INTERNAL_SERVER_ERROR, response);
    MHD_destroy_response(response);
  }
  return http_ret;
}

static enum MHD_Result dispatcher(void *cls,
                                  struct MHD_Connection *connection,
                                  const char *url,
                                  const char *method,
                                  const char *version,
                                  const char *upload_data,
                                  size_t *upload_data_size,
                                  void **con_cls)
{
  daemon_input_t *input = (daemon_input_t *)cls;
  // if POST malloc buffer to save body
  if ((NULL == *con_cls) && (0 == strcmp(method, "POST")))
  {
    struct unyte_https_body *body = malloc(sizeof(struct unyte_https_body));

    if (NULL == body)
    {
      printf("Malloc failed\n");
      return MHD_NO;
    }

    body->buffer = NULL;
    body->buffer_size = 0;

    *con_cls = (void *)body;
    return MHD_YES;
  }

  if ((0 == strcmp(method, "GET")) && (0 == strcmp(url, "/capabilities")))
    return get_capabilities(connection, input->capabilities);
  else if ((0 == strcmp(method, "POST")) && (0 == strcmp(url, "/relay-notification")))
  {
    struct unyte_https_body *body_buff = *con_cls;
    // if body exists, save body to use on next iteration
    if (*upload_data_size != 0)
    {
      body_buff->buffer = malloc(*upload_data_size + 1); // buff_size + \0
      
      if (body_buff->buffer == NULL)
      {
        printf("Malloc failed\n");
        return MHD_NO;
      }

      memcpy(body_buff->buffer, upload_data, *upload_data_size + 1);

      body_buff->buffer_size = *upload_data_size;
      *upload_data_size = 0;
      return MHD_YES;
    }
    // having body buffer
    else if (NULL != body_buff->buffer)
      return post_notification(connection, input->output_queue, body_buff);
    else
    {
      // TODO: if no body : what do we do ?
      return not_implemented(connection);
    }
  }
  else
    return not_implemented(connection);
}

void daemon_panic(void *cls, const char *file, unsigned int line, const char *reason)
{
  //TODO:
  printf("HTTPS server panic: %s\n", reason);
}

struct unyte_daemon *start_https_server_daemon(uint port, unyte_https_queue_t *output_queue)
{
  struct unyte_daemon *daemon = (struct unyte_daemon *)malloc(sizeof(struct unyte_daemon));
  daemon_input_t *daemon_in = (daemon_input_t *)malloc(sizeof(daemon_input_t));

  if (daemon_in == NULL || daemon == NULL)
  {
    printf("Malloc failed\n");
    return NULL;
  }

  unyte_https_capabilities_t *capabilities = init_capabilities_buff();

  daemon_in->output_queue = output_queue;
  daemon_in->capabilities = capabilities;

  struct MHD_Daemon *d = MHD_start_daemon(MHD_USE_THREAD_PER_CONNECTION,
                                          port,
                                          NULL,
                                          NULL,
                                          &dispatcher,
                                          daemon_in,
                                          MHD_OPTION_END);

  MHD_set_panic_func(daemon_panic, NULL);

  daemon->daemon = d;
  daemon->daemon_in = daemon_in;
  
  return daemon;
}

int stop_https_server_daemon(struct unyte_daemon *daemon)
{
  MHD_stop_daemon(daemon->daemon);
  // int ret = MHD_quiesce_daemon(daemon->daemon);
  // if (ret < 0)
  // {
  //   printf("Error stopping listenning for connections %d\n", ret);
  // }
  return 0;
}