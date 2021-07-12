AC_INIT([unyte_https_notif], m4_esyscmd_s(cat VERSION), [alex.huang-feng@insa-lyon.fr])
AC_CONFIG_AUX_DIR([build-aux])

AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])

AC_CONFIG_MACRO_DIR([m4])
AC_PREFIX_DEFAULT([/usr/local])

AM_PROG_AR
LT_INIT

AC_LANG([C])
AC_CONFIG_HEADERS([config.h])
AC_PROG_CC([gcc])
AC_CONFIG_SRCDIR(src/unyte_https_collector.c)

AC_PROG_LN_S

AC_ARG_ENABLE([tcmalloc],
    [AS_HELP_STRING([--enable-tcmalloc], [Use tcmalloc instead of native malloc. tcmalloc should be installed first.])],
    [ tcmalloc_enabled=yes ],
    [ tcmalloc_enabled=no ]
)

AC_ARG_WITH([examples],
    [AS_HELP_STRING([--with-examples], [Build examples samples])],
    [ with_examples=yes ],
    [ with_examples=no ]
)

AC_ARG_WITH([pkgconfigdir],
    [AS_HELP_STRING([--with-pkgconfigdir], [pkg-config directory to install the .pc file.])],
    [ with_pkgconfigdir=$with_pkgconfigdir ],
    [ with_pkgconfigdir=$libdir/pkgconfig ]
)

PKGCONFIG_USER=$with_pkgconfigdir
AC_SUBST([PKGCONFIG_USER])

AM_CONDITIONAL(EXAMPLES, test "x$with_examples" = "xyes")

AC_ARG_ENABLE([ebpf-loadbalancing],
    [AS_HELP_STRING([--enable-ebpf-loadbalancing], [Use eBPF reuseport socket load balancing.])],
    [ ebpf_reuse_enabled=yes ],
    [ ebpf_reuse_enabled=no ]
)

AM_CONDITIONAL(EBPF, test "x$ebpf_reuse_enabled" = "xyes")

dnl Check for microhttpd
AC_CHECK_LIB(microhttpd, MHD_start_daemon, [], [
    echo "*** Error! You need to install microhttpd first: https://github.com/Karlson2k/libmicrohttpd ***"
    exit -1
])
AC_CHECK_HEADERS([microhttpd.h])

PKG_PROG_PKG_CONFIG
PKG_CHECK_MODULES([LIBMICROHTTPD], [libmicrohttpd >= 0.9.73])

if test "x$tcmalloc_enabled" = "xyes"
then
    AC_CHECK_LIB(tcmalloc, malloc, [], [
        echo "*** Error! You need to install tcmalloc first. ***"
        exit -1
    ])
    EXTRA_LDFLAGS="-ltcmalloc -fno-builtin-malloc -fno-builtin-calloc -fno-builtin-realloc -fno-builtin-free"
    AC_SUBST([EXTRA_LDFLAGS])
fi

if test "x$ebpf_reuse_enabled" = "xyes"
then
    AC_CHECK_LIB(bpf, bpf_object__find_map_by_name, [], [
        echo "*** Error! You need to install libbpf first. ***"
        exit -1
    ])

    AC_CHECK_PROG([CCLANG], [clang], [yes])
    AS_IF([ test "x$CCLANG" != xyes ], [
        echo "*** Error! eBPF needs CLANG as dependency. Please install it first. ***"
        exit -1
    ])
    AC_CHECK_PROG([BPFTOOL], [bpftool], [yes])
    AS_IF([ test "x$BPFTOOL" != xyes ], [
        echo "*** Error! eBPF needs bpftool as dependency. Please install it first. ***"
        exit -1
    ])

    AC_CHECK_FILE([/sys/kernel/btf/vmlinux], [], [
        echo "*** Error! eBPF needs vmlinux to compile the eBPF program. ***"
        exit -1
    ])

    AC_ARG_WITH([linux],
        [AS_HELP_STRING([--with-linux], [Linux source headers [[default: /usr/src/linux]]])],
        [LINUX_HEADERS="$withval"],
        [LINUX_HEADERS=/usr/src/linux]
    )
    AC_SUBST([LINUX_HEADERS])
    dnl linux source headers
    AC_CHECK_FILES([
        $LINUX_HEADERS/include/uapi/linux
        $LINUX_HEADERS/arch/x86/include
        $LINUX_HEADERS/arch/x86/include/generated
        $LINUX_HEADERS/include
        $LINUX_HEADERS/arch/x86/include/uapi
        $LINUX_HEADERS/arch/x86/include/generated/uapi
        $LINUX_HEADERS/include/uapi
        $LINUX_HEADERS/include/generated/uapi
        ], [], [
            echo "*** Error! eBPFa needs linux source to compile the eBPF program. ***"
            echo "*** Linux sources were not found in $LINUX_HEADERS ***"
            exit -1
        ]
    )
    dnl TODO: check for bpf dependencies
fi

AC_CONFIG_FILES([
    Makefile
    src/Makefile
    examples/Makefile
    unyte-https-notif.pc
])
AC_OUTPUT
