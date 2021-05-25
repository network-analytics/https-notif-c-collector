#ifndef H_UNYTE_HTTPS_CAPABILITIES
#define H_UNYTE_HTTPS_CAPABILITIES

#include <stdbool.h>

typedef struct active_capabilities
{
  bool json_encoding;
  bool xml_encoding;
} active_capabilities_t;

typedef struct capabilities
{
  char *json;
  int json_length;
  char *xml;
  int xml_length;
  active_capabilities_t enabled;
} unyte_https_capabilities_t;

#define URN_ENCODING_JSON "urn:ietf:capability:https-notif-receiver:encoding:json"
#define URN_ENCODING_XML "urn:ietf:capability:https-notif-receiver:encoding:xml"

#define JSON_CAPABILITIES_START "{\"receiver-capabilities\": {\"receiver-capability\": ["
#define JSON_CAPABILITIES_END "]}}"

#define XML_CAPABILITIES_START "<receiver-capabilities>"
#define XML_CAPABILITIES_END "</receiver-capabilities>"
#define XML_CAP_WRAPPER_START "<receiver-capability>"
#define XML_CAP_WRAPPER_END "</receiver-capability>"

unyte_https_capabilities_t *init_capabilities_buff(bool disable_json_encoding, bool disable_xml_encoding);
void free_capabilities_buff(unyte_https_capabilities_t *capabilities);

#endif