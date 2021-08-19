# C-Collector for HTTPS-notif protocol
Library for collecting HTTPS-notif protocol messages defined on the IETF draft [draft-ietf-netconf-https-notif-08](https://datatracker.ietf.org/doc/html/draft-ietf-netconf-https-notif-08).

## Compiling and building the project
See [INSTALL](INSTALL.md)

## Usage
The collector allows to read HTTPS-notif protocol messages from a ip/port specified on the parameters. It allows to get directly the buffer and the metadata of the message in a struct.

The api is in `unyte_https_collector.h` :
- `unyte_https_collector_t *unyte_https_start_collector(unyte_https_options_t *options)` from `unyte_https_collector.h`: Initialize the HTTPS-notif messages collector. It accepts a struct with different options: address (the IP address to listen to), port (port to listen to).
- `void *unyte_https_queue_read(unyte_https_queue_t *queue)` from `unyte_https_queue.h` : read from a queue a struct with all the message buffer and metadata.
- `int unyte_https_free_msg(unyte_https_msg_met_t * msg)` from `unyte_https_collector.h`: free all struct used on a message received.

Simple example of usage of a client [client_sample.c](examples/client_sample.c):

/!\ To run the samples, a TLS private key and certificate should be generated first. See [TLS layer](#TLS-layer).
```c
#include <stdio.h>
#include <stdlib.h>

// include installed library headers
#include <unyte-https-notif/unyte_https_collector.h>
#include <unyte-https-notif/unyte_https_utils.h>

#define SERVERKEYFILE "private.key"
#define SERVERCERTFILE "certificate.pem"

int main()
{
  // Load cert file and key file for TLS encription
  char *key_pem = load_file(SERVERKEYFILE);
  char *cert_pem = load_file(SERVERCERTFILE);

  if ((key_pem == NULL) || (cert_pem == NULL))
  {
    printf("The key/certificate files could not be read.\n");
    return 1;
  }

  // Initialize collector options
  unyte_https_options_t options = {0};
  options.address = argv[1];
  options.port = atoi(argv[2]);
  options.cert_pem = cert_pem;
  options.key_pem = key_pem;
  options.disable_xml_encoding = false;   // enable xml encoding
  options.disable_json_encoding = false;  // enable json encoding

  // Initialize collector
  unyte_https_collector_t *collector = unyte_https_start_collector(&options);

  // Example with infinite loop, change the break condition to be able to free all struct gracefully
  while (1)
  {
    void *res = unyte_https_queue_read(collector->queue);
    unyte_https_msg_met_t *msg = (unyte_https_msg_met_t *)res;

    // TODO: Process the HTTPS-notif message here
    printf("unyte_https_get_src_port: %u\n", unyte_https_get_src_port(msg));
    printf("unyte_https_get_src_addr: %u\n", unyte_https_get_src_addr(msg));
    printf("unyte_https_get_payload: %s\n", unyte_https_get_payload(msg));
    printf("unyte_https_get_payload_length: %lu\n", unyte_https_get_payload_length(msg));
    printf("unyte_https_get_content_type: %s\n", unyte_https_get_content_type(msg));

    // Printing HTTPS-notif message on stdout
    print_https_notif_msg(msg, stdout);

    // Freeing struct
    unyte_https_free_msg(msg);
  }

  // Stopping the collector and the https server
  unyte_https_stop_collector(collector);

  // Freeing all dynamic mallocs
  unyte_https_free_collector(collector);

  // Freeing key_pem and cer_pem pointers
  free(key_pem);
  free(cert_pem);
  return 0;
}
```

### Message data
To process the message data, all the headers, meta-data and payload are found on the struct unyte_https_msg_met_t defined on unyte_https_utils.h:
```c
typedef struct unyte_msg_with_metadata
{
  uint16_t src_port;     // source port
  uint32_t src_addr;     // source address IPv4
  char *payload;         // payload buffer
  size_t payload_length; // payload buffer size
  char *content_type;    // payload buffer content type (application/json | application/xml)
} unyte_https_msg_met_t;
```
##### Getters for segments data
- `uint16_t unyte_https_get_src_port(unyte_https_msg_met_t *msg);` : source port of the message
- `uint32_t unyte_https_get_src_addr(unyte_https_msg_met_t *msg);` : source address of the message
- `char *unyte_https_get_payload(unyte_https_msg_met_t *msg);` : payload buffer
- `size_t unyte_https_get_payload_length(unyte_https_msg_met_t *msg);` : payload buffer size
- `char *unyte_https_get_content_type(unyte_https_msg_met_t *msg);` : content type of the message (application/json | application/xml)

### TLS layer
To use/test TLS layer, you should generate the certificate first :
```shell
$ openssl genrsa -out private.key 2048
$ openssl req -days 365 -out certificate.pem -new -x509 -key private.key
```
