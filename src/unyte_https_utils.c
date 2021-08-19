#include "unyte_https_utils.h"
#include <arpa/inet.h>
#include <string.h>

void print_https_notif_msg(unyte_https_msg_met_t *msg, FILE *std)
{
  fprintf(std, "\n###### Unyte HTTPS-notif msg ######\n");
  char ip_canonical[100];
  if (unyte_https_get_src(msg)->ss_family == AF_INET) {
    printf("src IPv4: %s\n", inet_ntop(unyte_https_get_src(msg)->ss_family, &((struct sockaddr_in*)unyte_https_get_src(msg))->sin_addr.s_addr, ip_canonical, sizeof ip_canonical));
    printf("src port: %u\n", ntohs(((struct sockaddr_in*)unyte_https_get_src(msg))->sin_port));
  } else {
    printf("src IPv6: %s\n", inet_ntop(unyte_https_get_src(msg)->ss_family, &((struct sockaddr_in6*)unyte_https_get_src(msg))->sin6_addr.s6_addr, ip_canonical, sizeof ip_canonical));
    printf("src port: %u\n", ntohs(((struct sockaddr_in6*)unyte_https_get_src(msg))->sin6_port));
  }
  fprintf(std, "Payload size: %lu\n", msg->payload_length);
  if (NULL != msg->content_type)
  {
    if (0 == strcmp(msg->content_type, UHTTPS_MIME_JSON))
      fprintf(std, "Content type: application/json\n");
    else if (0 == strcmp(msg->content_type, UHTTPS_MIME_XML))
      fprintf(std, "Content type: application/xml\n");
  }
  else
    fprintf(std, "Content type: null\n");
}

struct sockaddr_storage *unyte_https_get_src(unyte_https_msg_met_t *msg) { return msg->src; }
char *unyte_https_get_payload(unyte_https_msg_met_t *msg) { return msg->payload; }
size_t unyte_https_get_payload_length(unyte_https_msg_met_t *msg) { return msg->payload_length; }
char *unyte_https_get_content_type(unyte_https_msg_met_t *msg) { return msg->content_type; }
