# Building project 

## Dependencies
This project uses autotools to build the library and `libmicrohttpd` for the HTTPS server. **The libmicrohttpd library should be compiled and installed including TLS support.**

### Ubuntu
```shell
$ sudo apt-get install autoconf libtool make automake gcc pkg-config        # autotools and gcc
$ sudo apt-get install libgnutls28-dev libgcrypt20
$ mkdir microhttpd && cd microhttpd                                         # use a tmp directory
$ wget https://ftp.gnu.org/gnu/libmicrohttpd/libmicrohttpd-0.9.73.tar.gz
$ tar -xf libmicrohttpd-0.9.73.tar.gz
$ cd libmicrohttpd-0.9.73
$ ./configure               # check HTTPS support is activated
$ make
$ sudo make install
```

### Centos 7
```shell
$ sudo yum install gnutls-devel
$ wget https://ftp.gnu.org/gnu/libmicrohttpd/libmicrohttpd-0.9.73.tar.gz
$ tar -xf libmicrohttpd-0.9.73.tar.gz
$ cd libmicrohttpd-0.9.73
$ ./configure           # check HTTPS support is activated
$ make
$ sudo make install
```

## Compiling project 
This project uses autotools to compile and install the library.

### Installing
To install the library on a linux machine.
```shell
$ ./bootstrap
$ ./configure         # See "./configure --help" for options
$ make
$ make install        # Usually needs sudo permissions
$ ./export.sh         # Optional: export LD_LIBRARY_PATH with /usr/local/lib in global variable to allow linking process
```

#### Configure options
There are some custom `./configure` options : 
- `--with-examples`: compile examples directory. Not compiled by default.
- `--enable-tcmalloc`: enable compilation with tcmalloc instead of native malloc. tcmalloc should be installed first.

### Uninstalling
```shell
$ sudo make uninstall
```
You should remove the export of the lib in your `.bashrc` manually yourself to fully remove the lib.
