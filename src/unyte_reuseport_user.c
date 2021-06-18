
#include <arpa/inet.h>
#include <assert.h>
#include <bpf/bpf.h>
#include <bpf/libbpf.h>
#include <linux/bpf.h>
#include <stdio.h>
#include <unistd.h>

#include <errno.h>
#include <linux/unistd.h>
#include <stdlib.h>

#define BPF_KERNEL_PRG "unyte_reuseport_kern.o"

const char NONCE_PATH[] = "/sys/fs/bpf/nonce";
const char TCP_MAP_PATH[] = "/sys/fs/bpf/tcpmap";

static int libbpf_print_fn(enum libbpf_print_level level, const char *format, va_list args)
{
	return level <= LIBBPF_DEBUG ? vfprintf(stderr, format, args) : 0;
}

int attach_loadbalancing_bpf_pgr(int sockfd)
{
  struct bpf_object *obj;
  int map_fd, prog_fd;
  char filename[] = BPF_KERNEL_PRG;
  int64_t sock = sockfd;
  long err = 0;
  uint32_t key = 0;

	libbpf_set_print(libbpf_print_fn);

  obj = bpf_object__open_file(filename, NULL);
  err = libbpf_get_error(obj);

  if (err)
  {
    perror("Failed to open BPF elf file");
    return 1;
  }

  struct bpf_map *nonce = bpf_object__find_map_by_name(obj, "nonce");
  assert(nonce);
  assert(bpf_map__set_pin_path(nonce, NONCE_PATH) == 0);

  struct bpf_map *tcpmap = bpf_object__find_map_by_name(obj, "tcp_balancing_targets");
  assert(tcpmap);
  assert(bpf_map__set_pin_path(tcpmap, TCP_MAP_PATH) == 0);

  if (bpf_object__load(obj) != 0)
  {
    perror("Error loading BPF object into kernel");
    return 1;
  }

  map_fd = bpf_map__fd(tcpmap);
  assert(map_fd);

  struct bpf_program *prog = bpf_object__find_program_by_name(obj, "_selector");
  if (!prog)
  {
    perror("Could not find BPF program in BPF object");
    return 1;
  }

  prog_fd = bpf_program__fd(prog);
  assert(prog_fd);

  assert(sock >= 0);
  // assert(listen(sock, 3) == 0);

  if (setsockopt(sock, SOL_SOCKET, SO_ATTACH_REUSEPORT_EBPF, &prog_fd,
                 sizeof(prog_fd)) != 0)
  {
    perror("Could not attach BPF prog");
    return 1;
  }

  printf("sockfd: %ld\n", sock);
  if (bpf_map_update_elem(map_fd, &key, &sock, BPF_ANY) != 0)
  {
    perror("Could not update reuseport array");
    return 1;
  }

  uint64_t res;
  if (bpf_map_lookup_elem(map_fd, &key, &res) != 0)
  {
    perror("Could not find own entry in REUSEPORT Array");
  }

  return 0;
}