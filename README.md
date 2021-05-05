# C-Collector for HTTPS-notif protocol
Library for collecting HTTPS-notif protocol messages defined on the IETF draft [draft-ietf-netconf-https-notif-08](https://datatracker.ietf.org/doc/html/draft-ietf-netconf-https-notif-08).

## Dependencies
The library uses `libmicrohttpd` as a HTTPS server. The library should be compiled and installed including TLS support

- [libmicrohttpd](https://www.gnu.org/software/libmicrohttpd/): https library
    - `libgnutls28-dev libgcrypt20`: dependencies for libmicrohttpd TLS module (tested on `Ubuntu`)

## Build & install 
To build the project and test example clients, just `make` on root folder. Il will compile with gcc all dependences and the clients.

### Installing
To install the library on a machine, run `make install` with sudo and `export.sh` without sudo. Export script will export the LD_LIBRARY_PATH on user space.
```
$ make
$ sudo make install
$ ./export.sh
```

## Usage
TODO:

### TLS layer
- `openssl genrsa -out server.key 2048`
- `openssl req -days 365 -out server.pem -new -x509 -key server.key`
