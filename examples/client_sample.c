#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>

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

  return 0;
}
