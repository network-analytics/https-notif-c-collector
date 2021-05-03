#include "unyte_https_utils.h"

void print_https_notif_msg(unyte_https_msg_met_t *msg, FILE *std)
{
  fprintf(std, "\n###### Unyte HTTPS-notif msg ######\n");
  fprintf(std, "IP src: %u\n", msg->src_addr);
  fprintf(std, "Port src: %u\n", msg->src_port);
  fprintf(std, "Payload: %s\n", msg->payload);
}
