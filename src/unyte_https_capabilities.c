#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "unyte_https_capabilities.h"

unyte_https_capabilities_t *init_capabilities_buff()
{
  unyte_https_capabilities_t *capabilities = (unyte_https_capabilities_t *)malloc(sizeof(unyte_https_capabilities_t));

  capabilities->json = JSON_CAPABILITIES;
  capabilities->json_length = strlen(JSON_CAPABILITIES);
  capabilities->xml = XML_CAPABILITIES;
  capabilities->xml_length = strlen(XML_CAPABILITIES);

  return capabilities;
}
