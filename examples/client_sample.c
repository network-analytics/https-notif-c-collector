#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>

#include "../src/unyte_https_collector.h"

#define MAX_TO_RECEIVE 5

int main(int argc, char *argv[])
{
  if (argc != 3)
  {
    printf("Error: arguments not valid\n");
    printf("Usage: ./client_sample <ip> <port>\n");
    exit(1);
  }

  unyte_https_options_t options = {0};
  options.address = argv[1];
  options.port = atoi(argv[2]);

  unyte_https_collector_t *collector = unyte_start_collector(&options);
  printf("Starting collector on %s:%d\n", options.address, options.port);

  uint count = 0;
  while (count < MAX_TO_RECEIVE)
  {
    void *res = unyte_https_queue_read(collector->queue);
    printf("%s\n", (char *)res);
    free(res);
    count++;
  }

  //TODO: getc to not exit program
  (void)getc(stdin);
  unyte_free_collector(collector);
  return 0;
}
