AC_PREREQ([2.69])
AC_INIT([tdse], [0.0], [laftur.jeremy@gmail.com])
AM_INIT_AUTOMAKE([-Wall -Werror])
AC_CONFIG_MACRO_DIRS([m4])
AC_PROG_CXX
AX_CXX_COMPILE_STDCXX_11([noext], [mandatory])
AX_BOOST_BASE([1.59], , [AC_MSG_ERROR([boost was not found])])
AX_BOOST_SYSTEM
AX_BOOST_ASIO
AC_CANONICAL_HOST
if test "x$host_os" = xmingw64 || test "x$host_os" = xmingw32; then
    AC_SUBST(WINSOCKETS_LIB, [-lws2_32])
fi
AX_PTHREAD(, [AC_MSG_ERROR([pthread was not found])])
PKG_CHECK_MODULES(SDL2, sdl2 >= 1.11)
PKG_CHECK_MODULES(GLEW, glew >= 1.12)
PKG_CHECK_MODULES(Bullet, bullet >= 2.82)
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT
