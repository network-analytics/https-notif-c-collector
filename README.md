# C-Collector for HTTPS-notif protocol
Library for collecting HTTPS-notif protocol messages.

## Dependencies
- [libmicrohttpd](https://www.gnu.org/software/libmicrohttpd/): https library
    - `libgnutls28-dev libgcrypt20`: dependencies for libmicrohttpd TLS module (tested on Ubuntu)

## Build & install 
TODO:

### Installing
TODO:

## Usage
TODO:

### TLS layer
- `openssl genrsa -out server.key 2048`
- `openssl req -days 365 -out server.pem -new -x509 -key server.key`
