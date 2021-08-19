/**
 * Example of client using a eBPF loadbalancer.
 * This allows multiple client instances listening on the same IP/port and receiving consistent data
 * (all packets from one src IP will always go on the same collector)
 *
 * Usage: ./client_ebpf_user <ip> <port> <index> <balancer_max>
 *
 * Example: launching 3 instances on the same ip port. The index is the index on the map to put the socket
 * and the balancer_max is how many max instances are in use.
 *
 *    ./client_ebpf_user 192.168.1.17 10001 0 3
 *    ./client_ebpf_user 192.168.1.17 10001 1 3
 *    ./client_ebpf_user 192.168.1.17 10001 2 3
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <assert.h>
#include <bpf/bpf.h>
#include <bpf/libbpf.h>
#include <linux/bpf.h>
#include <linux/unistd.h>

#include "../../src/unyte_https_collector.h"
#include "../../src/unyte_https_utils.h"
#include "../../src/unyte_https_defaults.h"

#define MAX_TO_RECEIVE 10
#define SERVERKEYFILE "private.key"      // Should be generated before run this sample
#define SERVERCERTFILE "certificate.pem" // Should be generated before run this sample

// eBPF program
#define BPF_KERNEL_PRG "reuseport_https_kern.o"

#ifndef MAX_BALANCER_COUNT
// Keep in sync with _kern.c
#define MAX_BALANCER_COUNT 128
#endif

static int libbpf_print_fn(enum libbpf_print_level level, const char *format, va_list args) {
  return level <= LIBBPF_DEBUG ? vfprintf(stderr, format, args) : 0;
}

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
  hints.ai_protocol = IPPROTO_TCP;

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

int open_socket_attach_ebpf(char *address, char *port, uint64_t buff_size, int backlog, uint32_t key, uint32_t balancer_count)
{
  int umap_fd, size_map_fd, prog_fd;
  char filename[] = BPF_KERNEL_PRG;
  int64_t usock;
  long err = 0;

  assert(!balancer_count || key < balancer_count);
  assert(balancer_count <= MAX_BALANCER_COUNT);
  printf("from args: Using hash bucket index %u", key);
  if (balancer_count > 0) printf(" (%u buckets in total)", balancer_count);
  puts("");

  // set log
  libbpf_set_print(libbpf_print_fn);

  // Open reuseport_https_kern.o
  struct bpf_object_open_opts opts = {.sz = sizeof(struct bpf_object_open_opts),
                                      .pin_root_path = "/sys/fs/bpf/reuseport"};
  struct bpf_object *obj = bpf_object__open_file(filename, &opts);

  err = libbpf_get_error(obj);
  if (err) {
    perror("Failed to open BPF elf file");
    return -1;
  }

  struct bpf_map *tcpmap = bpf_object__find_map_by_name(obj, "tcp_balancing_targets");
  assert(tcpmap);

  // Load reuseport_https_kern.o to the kernel
  if (bpf_object__load(obj) != 0) {
    perror("Error loading BPF object into kernel");
    return -1;
  }

  struct bpf_program *prog = bpf_object__find_program_by_name(obj, "_selector");
  if (!prog) {
    perror("Could not find BPF program in BPF object");
    return -1;
  }

  prog_fd = bpf_program__fd(prog);
  assert(prog_fd);

  umap_fd = bpf_map__fd(tcpmap);
  assert(umap_fd);

  usock = create_socket(address, port, buff_size, backlog);

  assert(usock >= 0);

  if (setsockopt(usock, SOL_SOCKET, SO_ATTACH_REUSEPORT_EBPF, &prog_fd, sizeof(prog_fd)) != 0) {
    perror("Could not attach BPF prog");
    return -1;
  }

  printf("TCP sockfd: %ld\n", usock);
  if (bpf_map_update_elem(umap_fd, &key, &usock, BPF_ANY) != 0) {
    perror("Could not update reuseport array");
    return -1;
  }

  // Determine intended number of hash buckets
  // Assumption: static during lifetime of this process
  struct bpf_map *size_map = bpf_object__find_map_by_name(obj, "size");
  assert(size_map);
  size_map_fd = bpf_map__fd(size_map);
  assert(size_map_fd);

  uint32_t index = 0;
  if (balancer_count == 0) {  // no user-supplied limit
    bpf_map_lookup_elem(size_map_fd, &index, &balancer_count);
    if (balancer_count == 0) {  // BPF program hasn't run yet to initalize this
      balancer_count = MAX_BALANCER_COUNT;
      if (bpf_map_update_elem(size_map_fd, &index, &balancer_count, BPF_ANY) != 0) {
        perror("Could not update balancer count");
        return -1;
      }
    }
  } else {  // Overwrite global count with user supplied one
    if (bpf_map_update_elem(size_map_fd, &index, &balancer_count, BPF_ANY) != 0) {
      perror("Could not update balancer count");
      return -1;
    }
  }

  return usock;
}

int main(int argc, char *argv[])
{
  if (argc != 5)
  {
    printf("Error: arguments not valid\n");
    printf("Usage: ./client_ebpf_user <ip> <port> <index> <loadbalance_max>\n");
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
  int socketfd = open_socket_attach_ebpf(argv[1], argv[2], DF_SOCK_BUFF_SIZE, DF_SOCK_LISTEN_BACKLOG, atoi(argv[3]), atoi(argv[4]));

  if (socketfd < 0) exit(1);

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

    // printf("unyte_https_get_src: %s\n", unyte_https_get_src(msg)->ss_family == AF_INET ? "IPv4" : "IPv6");
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
