#include "unyte_https_utils.h"
#include <arpa/inet.h>

void print_https_notif_msg(unyte_https_msg_met_t *msg, FILE *std)
{
  struct in_addr ip_addr;
  ip_addr.s_addr = htonl(msg->src_addr);
  fprintf(std, "\n###### Unyte HTTPS-notif msg ######\n");
  fprintf(std, "IP src: %s\n", inet_ntoa(ip_addr));
  fprintf(std, "Port src: %u\n", msg->src_port);
  fprintf(std, "Payload: %s\n", msg->payload);
}
