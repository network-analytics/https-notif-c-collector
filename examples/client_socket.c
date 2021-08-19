/**
 * Sample to show how to consume HTTPS-notif protocol using the library and a custom socket
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#include "../src/unyte_https_collector.h"
#include "../src/unyte_https_utils.h"
#include "../src/unyte_https_defaults.h"

#define MAX_TO_RECEIVE 10
#define SERVERKEYFILE "private.key"      // Should be generated before run this sample
#define SERVERCERTFILE "certificate.pem" // Should be generated before run this sample

/**
 * Read file and return bytes
 */
char *load_file(const char *file_path)
{
  FILE *infile;
  char *buffer;
  long numbytes;

  infile = fopen(file_path, "r");

  if (infile == NULL)
    return NULL;

  fseek(infile, 0L, SEEK_END);
  numbytes = ftell(infile);
  rewind(infile);

  buffer = (char *)malloc(sizeof(char) * (numbytes + 1));

  if (buffer == NULL)
    return NULL;

  fread(buffer, sizeof(char), numbytes, infile);
  fclose(infile);
  buffer[numbytes] = '\0';
  return buffer;
}

int create_socket(char *addr, char *port, uint64_t buff_size, int backlog)
{
  struct addrinfo *addr_info;
  struct addrinfo hints;

  memset(&hints, 0, sizeof(hints));

  hints.ai_socktype = SOCK_STREAM;
  hints.ai_family = AF_UNSPEC;
  hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV;

  // Using getaddrinfo to support both IPv4 and IPv6
  int rc = getaddrinfo(addr, port, &hints, &addr_info);

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
  // enabling SO_REUSEPORT to loadbalance between multiple collectors
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(int)) < 0)
  {
    perror("Cannot set SO_REUSEPORT option on socket");
    exit(EXIT_FAILURE);
  }

  // Setting custom buffer size
  if (setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &buff_size, sizeof(buff_size)) < 0)
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

  // listen to socket file descriptor using custom backlog
  if (listen(sockfd, backlog) != 0)
  {
    perror("Listen failed");
    close(sockfd);
    exit(EXIT_FAILURE);
  }

  // free addr_info after usage
  freeaddrinfo(addr_info);

  return sockfd;
}

int main(int argc, char *argv[])
{
  if (argc != 3)
  {
    printf("Error: arguments not valid\n");
    printf("Usage: ./client_sample <ip> <port>\n");
    exit(1);
  }

  char *key_pem = load_file(SERVERKEYFILE);
  char *cert_pem = load_file(SERVERCERTFILE);

  if ((key_pem == NULL) || (cert_pem == NULL))
  {
    printf("The key/certificate files could not be read.\n");
    return 1;
  }

  // creating and listening to socket using 20MB of socket buffer and a backlog of 10
  int socketfd = create_socket(argv[1], argv[2], DF_SOCK_BUFF_SIZE, DF_SOCK_LISTEN_BACKLOG);

  unyte_https_sk_options_t options = {0};
  options.socket_fd = socketfd;
  options.cert_pem = cert_pem;
  options.key_pem = key_pem;
  options.disable_xml_encoding = true;
  options.disable_json_encoding = false;

  unyte_https_collector_t *collector = unyte_https_start_collector_sk(&options);
  printf("Starting collector on %s:%s\n", argv[1], argv[2]);

  uint count = 0;
  while (count < MAX_TO_RECEIVE)
  {
    void *res = unyte_https_queue_read(collector->queue);
    unyte_https_msg_met_t *msg = (unyte_https_msg_met_t *)res;

    print_https_notif_msg(msg, stdout);

    // printf("unyte_https_get_src_port: %u\n", unyte_https_get_src_port(msg));
    // printf("unyte_https_get_src_addr: %u\n", unyte_https_get_src_addr(msg));
    // printf("unyte_https_get_payload: %s\n", unyte_https_get_payload(msg));
    // printf("unyte_https_get_payload_length: %lu\n", unyte_https_get_payload_length(msg));
    // printf("unyte_https_get_content_type: %s\n", unyte_https_get_content_type(msg));

    fflush(stdout);

    // Freeing struct
    unyte_https_free_msg(msg);
    count++;
  }

  // Stopping the collector and the https server
  unyte_https_stop_collector(collector);
  // Freeing all dynamic mallocs
  unyte_https_free_collector(collector);

  free(key_pem);
  free(cert_pem);
  return 0;
}
