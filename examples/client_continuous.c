#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/unyte_https_collector.h"
#include "../src/unyte_https_utils.h"

#define MAX_TO_RECEIVE 200
#define SERVERKEYFILE "server.key"
#define SERVERCERTFILE "server.pem"

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

struct https_th_input
{
  unyte_https_queue_t *queue;
  uint count;
};

void *t_https_read(void *in)
{
  struct https_th_input *input = (struct https_th_input *)in;

  pthread_t thread_id = pthread_self();
  uint count = 0;
  while (count < input->count)
  {
    void *res = unyte_https_queue_read(input->queue);
    unyte_https_msg_met_t *msg = (unyte_https_msg_met_t *)res;

    printf("%ld;%lu\n", thread_id, unyte_https_get_payload_length(msg));

    fflush(stdout);

    // Freeing struct
    unyte_https_free_msg(msg);
    count++;
  }
  return 0;
}

int main(int argc, char *argv[])
{
  if (argc != 3)
  {
    printf("Error: arguments not valid\n");
    printf("Usage: ./client_continuous <ip> <port>\n");
    exit(1);
  }

  char *key_pem = load_file(SERVERKEYFILE);
  char *cert_pem = load_file(SERVERCERTFILE);

  if ((key_pem == NULL) || (cert_pem == NULL))
  {
    printf("The key/certificate files could not be read.\n");
    return 1;
  }

  unyte_https_options_t options = {0};
  options.address = argv[1];
  options.port = atoi(argv[2]);
  options.cert_pem = cert_pem;
  options.key_pem = key_pem;

  uint client_threads = 5;
  pthread_t *threads = (pthread_t *)malloc(sizeof(pthread_t) * client_threads);

  unyte_https_collector_t *collector = unyte_start_collector(&options);
  printf("Starting collector on %s:%d\n", options.address, options.port);

  struct https_th_input input = {0};
  input.queue = collector->queue;
  input.count = 100;

  for (uint i = 0; i < client_threads; i++)
  {
    pthread_create((threads + i), NULL, t_https_read, &input);
  }

  for (uint o = 0; o < client_threads; o++)
    pthread_join(*(threads + o), NULL);

  // Stopping the collector and the https server
  unyte_stop_collector(collector);
  // Freeing all dynamic mallocs
  unyte_free_collector(collector);

  free(key_pem);
  free(cert_pem);
  free(threads);
  return 0;
}
