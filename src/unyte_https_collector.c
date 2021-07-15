#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include "unyte_https_collector.h"
#include "unyte_https_version.h"
#include "unyte_https_defaults.h"
#include "unyte_https_queue.h"

#if _USE_EBPF_REUSEPORT
#include "unyte_https_reuseport_user.h"
#endif

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
  if (options->output_queue_size <= 0)
    options->output_queue_size = DF_OUTPUT_QUEUE_SIZE;
  if (options->sock_buff_size <= 0)
    options->sock_buff_size = DF_SOCK_BUFF_SIZE;
  if (options->sock_listen_backlog <= 0)
    options->sock_listen_backlog = DF_SOCK_LISTEN_BACKLOG;
}

unyte_https_sock_t *unyte_https_init_socket(char *address, uint16_t port, uint64_t sock_buff_size, int backlog)
{
  unyte_https_sock_t *conn = (unyte_https_sock_t *)malloc(sizeof(unyte_https_sock_t));
  struct sockaddr_in *servaddr = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in));
  int *sockfd = (int *)malloc(sizeof(int));

  if (servaddr == NULL || conn == NULL || sockfd == NULL)
  {
    printf("Malloc failed\n");
    exit(EXIT_FAILURE);
  }

  memset(servaddr, 0, sizeof(struct sockaddr_in));

  *sockfd = socket(AF_INET, SOCK_STREAM, 0);

  if (*sockfd < 0)
  {
    perror("Cannot create socket");
    exit(EXIT_FAILURE);
  }

  servaddr->sin_family = AF_INET;
  servaddr->sin_port = htons(port);
  inet_pton(AF_INET, address, &servaddr->sin_addr);

  int optval = 1;
  if (setsockopt(*sockfd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(int)) < 0)
  {
    perror("Cannot set SO_REUSEPORT option on socket");
    exit(EXIT_FAILURE);
  }

  if (setsockopt(*sockfd, SOL_SOCKET, SO_RCVBUF, &sock_buff_size, sizeof(sock_buff_size)) < 0)
  {
    perror("Cannot set buffer size");
    exit(EXIT_FAILURE);
  }

  if (bind(*sockfd, (struct sockaddr *)servaddr, sizeof(*servaddr)) != 0)
  {
    perror("Bind failed");
    close(*sockfd);
    exit(EXIT_FAILURE);
  }

  if (listen(*sockfd, backlog) != 0)
  {
    perror("Listen failed");
    close(*sockfd);
    exit(EXIT_FAILURE);
  }

#if _USE_EBPF_REUSEPORT
  if (attach_loadbalancing_bpf_pgr(*sockfd) != 0)
  {
    exit(EXIT_FAILURE);
  }
#endif

  conn->addr = servaddr;
  conn->sockfd = sockfd;

  return conn;
}

unyte_https_collector_t *unyte_https_start_collector(unyte_https_options_t *options)
{
  unyte_https_collector_t *collector = (unyte_https_collector_t *)malloc(sizeof(unyte_https_collector_t));

  set_defaults(options);

  unyte_https_queue_t *queue = unyte_https_queue_init(options->output_queue_size);

  if (queue == NULL || collector == NULL)
  {
    printf("Malloc error\n");
    exit(EXIT_FAILURE);
  }

  unyte_https_sock_t *conn = unyte_https_init_socket(options->address, options->port, options->sock_buff_size, options->sock_listen_backlog);
  struct unyte_daemon *daemon = start_https_server_daemon(conn,
                                                          queue,
                                                          options->key_pem,
                                                          options->cert_pem,
                                                          options->disable_json_encoding,
                                                          options->disable_xml_encoding);

  if (daemon == NULL)
  {
    printf("Error creating daemon\n");
    exit(EXIT_FAILURE);
  }

  collector->queue = queue;
  collector->https_daemon = daemon;
  collector->sock_conn = conn;

  return collector;
}

int unyte_https_stop_collector(unyte_https_collector_t *collector)
{
  return stop_https_server_daemon(collector->https_daemon);
}

int unyte_https_free_collector(unyte_https_collector_t *collector)
{
  while (unyte_https_is_queue_empty(collector->queue) != 0)
    free(unyte_https_queue_read(collector->queue));

  free(collector->queue->data);
  free(collector->queue);
  free_https_server_daemon(collector->https_daemon);
  free(collector->sock_conn->sockfd);
  free(collector->sock_conn->addr);
  free(collector->sock_conn);
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
