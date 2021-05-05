#include "unyte_https_utils.h"
#include <arpa/inet.h>
#include <string.h>

void print_https_notif_msg(unyte_https_msg_met_t *msg, FILE *std)
{
  struct in_addr ip_addr;
  ip_addr.s_addr = htonl(msg->src_addr);
  fprintf(std, "\n###### Unyte HTTPS-notif msg ######\n");
  fprintf(std, "IP src: %s\n", inet_ntoa(ip_addr));
  fprintf(std, "Port src: %u\n", msg->src_port);
  fprintf(std, "Payload size: %lu\n", msg->payload_length);
  if (0 == strcmp(msg->content_type, UHTTPS_MIME_JSON))
    fprintf(std, "Content type: application/json\n");
  else if (0 == strcmp(msg->content_type, UHTTPS_MIME_XML))
    fprintf(std, "Content type: application/xml\n");
  else
    fprintf(std, "Content type: null\n");
}

uint16_t unyte_https_get_src_port(unyte_https_msg_met_t *msg) { return msg->src_port; }
uint32_t unyte_https_get_src_addr(unyte_https_msg_met_t *msg) { return msg->src_addr; }
char *unyte_https_get_payload(unyte_https_msg_met_t *msg) { return msg->payload; }
size_t unyte_https_get_payload_length(unyte_https_msg_met_t *msg) { return msg->payload_length; }
char *unyte_https_get_content_type(unyte_https_msg_met_t *msg) { return msg->content_type; }
