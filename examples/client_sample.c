#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>

#include "../src/unyte_https_collector.h"

#define USED_VLEN 10
#define MAX_TO_RECEIVE 200

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


  unyte_free_collector(collector);
  return 0;
}
