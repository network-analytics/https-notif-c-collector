#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/unyte_https_collector.h"
#include "../src/unyte_https_utils.h"

#define MAX_TO_RECEIVE 200
#define SERVERKEYFILE "private.key"
#define SERVERCERTFILE "certificate.pem"

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

void time_diff(struct timespec *diff, struct timespec *stop, struct timespec *start, int messages, pthread_t thread_id)
{
  if (stop->tv_nsec < start->tv_nsec)
  {
    /* here we assume (stop->tv_sec - start->tv_sec) is not zero */
    diff->tv_sec = stop->tv_sec - start->tv_sec - 1;
    diff->tv_nsec = stop->tv_nsec - start->tv_nsec + 1000000000;
  }
  else
  {
    diff->tv_sec = stop->tv_sec - start->tv_sec;
    diff->tv_nsec = stop->tv_nsec - start->tv_nsec;
  }
  printf("Data;%ld;%d;%ld.%06ld\n", thread_id, messages, diff->tv_sec * 1000 + diff->tv_nsec / 1000000, diff->tv_nsec % 1000000);
}

struct https_th_input
{
  unyte_https_queue_t *queue;
  uint count;
  uint msg_between_logs;
};

void *t_https_read(void *in)
{
  struct https_th_input *input = (struct https_th_input *)in;

  pthread_t thread_id = pthread_self();
  uint count = 0;
  int first = 1;

  struct timespec start;
  struct timespec stop;
  struct timespec diff;

  while (count < input->count)
  {
    void *res = unyte_https_queue_read(input->queue);
    unyte_https_msg_met_t *msg = (unyte_https_msg_met_t *)res;

    // printf("%ld;%d;%lu\n", thread_id, unyte_https_get_src_port(msg), unyte_https_get_payload_length(msg));

    if ((count % input->msg_between_logs) == 0)
    {
      if (first)
      {
        first = 0;
        clock_gettime(CLOCK_MONOTONIC, &start);
      }
      else
      {
        clock_gettime(CLOCK_MONOTONIC, &stop);
        time_diff(&diff, &stop, &start, count, thread_id);
        clock_gettime(CLOCK_MONOTONIC, &start);
      }
    }
    fflush(stdout);

    // Freeing struct
    unyte_https_free_msg(msg);
    count++;
  }

  // log last time
  // clock_gettime(CLOCK_MONOTONIC, &stop);
  // time_diff(&diff, &stop, &start, count, thread_id);
  // clock_gettime(CLOCK_MONOTONIC, &start);
  return 0;
}

int main(int argc, char *argv[])
{
  if (argc != 6)
  {
    printf("Error: arguments not valid\n");
    printf("Usage: ./client_continuous <ip> <port> <nb_msgs> <client_threads> <msgs_between_log>\n");
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

  uint client_threads = atoi(argv[4]);
  pthread_t *threads = (pthread_t *)malloc(sizeof(pthread_t) * client_threads);

  unyte_https_collector_t *collector = unyte_start_collector(&options);
  printf("Starting collector on %s:%d\n", options.address, options.port);

  struct https_th_input input = {0};
  input.queue = collector->queue;
  input.count = atoi(argv[3]);
  input.msg_between_logs = atoi(argv[5]);

  for (uint i = 0; i < client_threads; i++)
    pthread_create((threads + i), NULL, t_https_read, &input);

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
