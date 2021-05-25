#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "unyte_https_capabilities.h"

unyte_https_capabilities_t *malloc_capabilities_buff(bool disable_json_encoding, bool disable_xml_encoding)
{
  unyte_https_capabilities_t *capabilities = (unyte_https_capabilities_t *)malloc(sizeof(unyte_https_capabilities_t));

  if (capabilities == NULL)
    return NULL;

  int json_capabilities_length = strlen(JSON_CAPABILITIES_START) + strlen(JSON_CAPABILITIES_END);
  int xml_capabilities_length = strlen(XML_CAPABILITIES_START) + strlen(XML_CAPABILITIES_END);

  if (!disable_xml_encoding)
  {
    json_capabilities_length += strlen(URN_ENCODING_XML) + 2; // 2 for the two ""
    xml_capabilities_length += strlen(XML_CAP_WRAPPER_START) + strlen(URN_ENCODING_XML) + strlen(XML_CAP_WRAPPER_END);
  }

  if (!disable_json_encoding)
  {
    if (!disable_xml_encoding)
      json_capabilities_length += 1;                           // 1 for the ","
    json_capabilities_length += strlen(URN_ENCODING_JSON) + 2; // 2 for the two ""
    xml_capabilities_length += strlen(XML_CAP_WRAPPER_START) + strlen(URN_ENCODING_JSON) + strlen(XML_CAP_WRAPPER_END);
  }

  capabilities->json = (char *)malloc(json_capabilities_length + 1); // 1 for \0
  capabilities->xml = (char *)malloc(xml_capabilities_length + 1);   // 1 for \0
  capabilities->json_length = json_capabilities_length;
  capabilities->xml_length = xml_capabilities_length;

  capabilities->enabled.json_encoding = false;
  capabilities->enabled.xml_encoding = false;
  return capabilities;
}

void add_xml_capability(unyte_https_capabilities_t *capabilities, int *json_it, int *xml_it)
{
  capabilities->json[*json_it] = '"';
  (*json_it)++;

  strcpy(capabilities->json + *json_it, URN_ENCODING_XML);
  (*json_it) += strlen(URN_ENCODING_XML);

  capabilities->json[*json_it] = '"';
  (*json_it)++;

  strcpy(capabilities->xml + *xml_it, XML_CAP_WRAPPER_START);
  (*xml_it) += strlen(XML_CAP_WRAPPER_START);
  strcpy(capabilities->xml + *xml_it, URN_ENCODING_XML);
  (*xml_it) += strlen(URN_ENCODING_XML);
  strcpy(capabilities->xml + *xml_it, XML_CAP_WRAPPER_END);
  (*xml_it) += strlen(XML_CAP_WRAPPER_END);
  
  capabilities->enabled.xml_encoding = true;
}

void add_json_capability(unyte_https_capabilities_t *capabilities, int *json_it, int *xml_it)
{

  capabilities->json[*json_it] = '"';
  (*json_it)++;

  strcpy(capabilities->json + *json_it, URN_ENCODING_JSON);
  (*json_it) += strlen(URN_ENCODING_JSON);

  capabilities->json[*json_it] = '"';
  (*json_it)++;

  strcpy(capabilities->xml + *xml_it, XML_CAP_WRAPPER_START);
  (*xml_it) += strlen(XML_CAP_WRAPPER_START);
  strcpy(capabilities->xml + *xml_it, URN_ENCODING_JSON);
  (*xml_it) += strlen(URN_ENCODING_JSON);
  strcpy(capabilities->xml + *xml_it, XML_CAP_WRAPPER_END);
  (*xml_it) += strlen(XML_CAP_WRAPPER_END);

  capabilities->enabled.json_encoding = true;
}

unyte_https_capabilities_t *init_capabilities_buff(bool disable_json_encoding, bool disable_xml_encoding)
{
  if (disable_json_encoding && disable_xml_encoding)
  {
    printf("Cannot initialize capabilities ignoring all supported encodings. Enable one or more encodings\n");
    return NULL;
  }
  unyte_https_capabilities_t *capabilities = malloc_capabilities_buff(disable_json_encoding, disable_xml_encoding);

  if (capabilities == NULL)
    return NULL;

  int json_it = strlen(JSON_CAPABILITIES_START);
  int xml_it = strlen(XML_CAPABILITIES_START);

  strcpy(capabilities->json, JSON_CAPABILITIES_START);
  strcpy(capabilities->xml, XML_CAPABILITIES_START);

  if (!disable_xml_encoding)
    add_xml_capability(capabilities, &json_it, &xml_it);

  if (!disable_json_encoding)
  {
    if (!disable_xml_encoding)
    {
      capabilities->json[json_it] = ',';
      json_it++;
    }
    add_json_capability(capabilities, &json_it, &xml_it);
  }

  strcpy(capabilities->json + json_it, JSON_CAPABILITIES_END);
  strcpy(capabilities->xml + xml_it, XML_CAPABILITIES_END);

  return capabilities;
}

void free_capabilities_buff(unyte_https_capabilities_t *capabilities)
{
  free(capabilities->json);
  free(capabilities->xml);
  free(capabilities);
}