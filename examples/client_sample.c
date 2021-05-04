#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>

#include "../src/unyte_https_collector.h"
#include "../src/unyte_https_utils.h"

#define MAX_TO_RECEIVE 5
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

  fseek(infile, 0L, SEEK_SET);

  buffer = (char *)calloc(numbytes, sizeof(char));

  if (buffer == NULL)
    return NULL;

  fread(buffer, sizeof(char), numbytes, infile);
  fclose(infile);

  return buffer;
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

  unyte_https_options_t options = {0};
  options.address = argv[1];
  options.port = atoi(argv[2]);
  options.cert_pem = cert_pem;
  options.key_pem = key_pem;

  unyte_https_collector_t *collector = unyte_start_collector(&options);
  printf("Starting collector on %s:%d\n", options.address, options.port);

  uint count = 0;
  while (count < MAX_TO_RECEIVE)
  {
    void *res = unyte_https_queue_read(collector->queue);
    unyte_https_msg_met_t *msg = (unyte_https_msg_met_t *)res;

    print_https_notif_msg(msg, stdout);

    // freeing struct
    unyte_https_free_msg(msg);
    count++;
  }

  // Stopping the collector and the https server
  unyte_stop_collector(collector);
  // Freeing all dynamic mallocs
  unyte_free_collector(collector);

  free(key_pem);
  free(cert_pem);
  return 0;
}
