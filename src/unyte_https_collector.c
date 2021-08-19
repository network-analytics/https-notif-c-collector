#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <netdb.h>
#include "unyte_https_collector.h"
#include "unyte_https_version.h"
#include "unyte_https_defaults.h"
#include "unyte_https_queue.h"

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

void set_sk_default_options(unyte_https_sk_options_t *options)
{
  if (options == NULL)
  {
    printf("Invalid options.\n");
    exit(EXIT_FAILURE);
  }
  if (options->socket_fd < 0)
  {
    printf("socket_fd is mandatory\n");
    exit(EXIT_FAILURE);
  }
  if ((options->cert_pem == NULL) || (options->key_pem == NULL))
  {
    printf("TLS certs are not valid\n");
    exit(EXIT_FAILURE);
  }
  if (options->output_queue_size <= 0)
    options->output_queue_size = DF_OUTPUT_QUEUE_SIZE;
}

unyte_https_sock_t *unyte_https_init_socket(char *address, char *port, uint64_t sock_buff_size, int backlog)
{
  unyte_https_sock_t *conn = (unyte_https_sock_t *)malloc(sizeof(unyte_https_sock_t));

  if (conn == NULL)
  {
    printf("Malloc failed\n");
    exit(EXIT_FAILURE);
  }

  struct addrinfo *addr_info;
  struct addrinfo hints;

  memset(&hints, 0, sizeof(hints));

  hints.ai_socktype = SOCK_STREAM;
  hints.ai_family = AF_UNSPEC;
  hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV;

  // Using getaddrinfo to support both IPv4 and IPv6
  int rc = getaddrinfo(address, port, &hints, &addr_info);

  if (rc != 0) {
    printf("getaddrinfo error: %s\n", gai_strerror(rc));
    exit(EXIT_FAILURE);
  }

  printf("Address type: %s | %d\n", (addr_info->ai_family == AF_INET) ? "IPv4" : "IPv6", ntohs(((struct sockaddr_in *)addr_info->ai_addr)->sin_port));
  int sockfd = socket(addr_info->ai_family, addr_info->ai_socktype, addr_info->ai_protocol);

  if (sockfd < 0)
  {
    perror("Cannot create socket");
    exit(EXIT_FAILURE);
  }

  int optval = 1;
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(int)) < 0)
  {
    perror("Cannot set SO_REUSEPORT option on socket");
    exit(EXIT_FAILURE);
  }

  if (setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &sock_buff_size, sizeof(sock_buff_size)) < 0)
  {
    perror("Cannot set buffer size");
    exit(EXIT_FAILURE);
  }

  if (bind(sockfd, addr_info->ai_addr, (int)addr_info->ai_addrlen) != 0)
  {
    perror("Bind failed");
    close(sockfd);
    exit(EXIT_FAILURE);
  }

  if (listen(sockfd, backlog) != 0)
  {
    perror("Listen failed");
    close(sockfd);
    exit(EXIT_FAILURE);
  }

  conn->addr = addr_info;
  conn->sockfd = sockfd;

  return conn;
}

unyte_https_collector_t *unyte_https_start_collector_sk(unyte_https_sk_options_t *options)
{
  unyte_https_collector_t *collector = (unyte_https_collector_t *)malloc(sizeof(unyte_https_collector_t));

  set_sk_default_options(options);

  unyte_https_queue_t *queue = unyte_https_queue_init(options->output_queue_size);
  unyte_https_sock_t *conn = (unyte_https_sock_t *)malloc(sizeof(unyte_https_sock_t));

  if (queue == NULL || collector == NULL || conn == NULL)
  {
    printf("Malloc error\n");
    exit(EXIT_FAILURE);
  }

  conn->addr = NULL;
  conn->sockfd = options->socket_fd;

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
  freeaddrinfo(collector->sock_conn->addr);
  free(collector->sock_conn);
  free(collector);
  return 0;
}

int unyte_https_free_msg(unyte_https_msg_met_t *msg)
{
  free(msg->payload);
  free(msg->src);
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
