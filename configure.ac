#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([tcloud-player], [0.1], [jay.han])
AC_CONFIG_SRCDIR([])
AM_INIT_AUTOMAKE(1.0)
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_CC(clang llvm-gcc gcc)
AC_PROG_CXX(clang++ llvm-g++ g++)
AC_PROG_CPP
AC_PROG_AWK
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
LT_INIT
AC_PROG_LIBTOOL

AM_CONDITIONAL([ARM_BUILD],[test "$with_arm" != "no"])

AC_ARG_WITH([x11], [AS_HELP_STRING([--with-x11],
                    [With X11 video sink])],[],[])

if test "x$with_x11" = xyes ; then
        CFLAGS=$(echo "$CFLAGS -Wall -I include `pkg-config --cflags --libs gtk+-3.0 gstreamer-video-1.0`")
    AC_MSG_NOTICE([Enable x11])
fi  

# Checks for libraries.

AC_CHECK_LIB([rt], [clock_gettime], [], [AC_MSG_FAILURE([rt liberary not found])])
AC_CHECK_LIB([pthread], [pthread_create], [], [AC_MSG_FAILURE([pthread liberary not found])])

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT
