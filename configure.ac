# Require Autoconf 2.69 or later
AC_PREREQ([2.69])
AC_INIT([tdse], [0.0], [laftur.jeremy@gmail.com])

# Automake warnings are treated as errors
AM_INIT_AUTOMAKE([-Wall -Werror])
AC_CONFIG_MACRO_DIRS([m4])

# Find archiver, symbol tabler, compiler (C++11 support required)
AM_PROG_AR
AC_PROG_RANLIB
AC_PROG_CXX
AX_CXX_COMPILE_STDCXX_11([noext], [mandatory])

# Enable/disable building the library
AC_ARG_ENABLE([lib],
              [AS_HELP_STRING([--enable-lib], [build TDSE library (requires
                 Boost and Bullet, default is yes)])],
              [AS_CASE(["$enableval"], [yes], [], [no], [],
                 [AC_MSG_ERROR([bad value $(enableval) for --enable-lib])])],
              [enable_lib=yes])

# Enable/disable building the demo
AC_ARG_ENABLE([demo],
              [AS_HELP_STRING([--enable-demo], [build demonstration program
                 (requires SDL2 and GLEW, default is no)])],
              [AS_CASE(["$enableval"], [yes], [], [no], [],
                 [AC_MSG_ERROR([bad value $(enableval) for --enable-demo])])],
              [enable_demo=no])

# If building the library, find Boost (networking, I/O) and Bullet (physics)
AS_IF([test "$enable_lib" = yes],
      [AX_BOOST_BASE([1.59], , [AC_MSG_ERROR([boost was not found])])
       AX_BOOST_SYSTEM
       AX_BOOST_ASIO
       # If building for Windows, link with special socket library
       AC_CANONICAL_HOST
       AS_IF([test "$host_os" = mingw64 || test "$host_os" = mingw32],
             [AC_SUBST(WINSOCKETS_LIB, [-lws2_32])],
             [])
       PKG_CHECK_MODULES(Bullet, bullet >= 2.82)
       AX_PTHREAD(, [AC_MSG_ERROR([pthread was not found])])
       AC_SUBST(LIB_SUBDIR, [src])],
      [])

# If building the demo, find SDL2 and GLEW (graphics and user input)
AS_IF([test "$enable_demo" = yes],
      [PKG_CHECK_MODULES(SDL2, sdl2 >= 1.11)
       PKG_CHECK_MODULES(GLEW, glew >= 1.12)
       AC_SUBST(DEMO_SUBDIR, [src/demo])],
      [])

# The application can access compile-time configuration via config.h
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile src/Makefile src/demo/Makefile])
AC_OUTPUT
