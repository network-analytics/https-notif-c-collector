#ifndef H_UNYTE_HTTPS_CAPABILITIES
#define H_UNYTE_HTTPS_CAPABILITIES

typedef struct capabilities
{
  char *json;
  int json_length;
  char *xml;
  int xml_length;
} unyte_https_capabilities_t;

#define JSON_CAPABILITIES "{\
  \"receiver-capabilities\": {\
    \"receiver-capability\": [\
      \"urn:ietf:capability:https-notif-receiver:encoding:json\",\
      \"urn:ietf:capability:https-notif-receiver:encoding:xml\",\
      \"urn:ietf:capability:https-notif-receiver:encoding:rfc8639-enabled\"\
    ]\
  }\
}"

#define XML_CAPABILITIES "<receiver-capabilities>\
<receiver-capability>\
urn:ietf:capability:https-notif-receiver:encoding:json\
</receiver-capability>\
<receiver-capability>\
urn:ietf:capability:https-notif-receiver:encoding:xml\
</receiver-capability>\
<receiver-capability>\
urn:ietf:capability:https-notif-receiver:encoding:rfc8639-enabled\
</receiver-capability>\
</receiver-capabilities>"

unyte_https_capabilities_t *init_capabilities_buff();

#endif