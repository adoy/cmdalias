AC_PREREQ([2.69])
AC_INIT([cmdalias], [1.0.0-dev], [pierrick@adoy.net])
AC_CONFIG_SRCDIR([src/cmdalias.c])
AC_CONFIG_HEADERS([include/config.h])

cat >>confdefs.h <<_ACEOF
#define CMDALIAS_VERSION "$PACKAGE_VERSION"
_ACEOF

AC_PROG_CC
AC_PROG_LEX
AC_PROG_YACC

# Checks for libraries.

# Checks for header files.
AC_FUNC_ALLOCA
AC_CHECK_HEADERS([inttypes.h libintl.h malloc.h stddef.h stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT8_T
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_LSTAT_FOLLOWS_SLASHED_SYMLINK
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([dup2 memset strdup strerror strndup])

AC_ARG_ENABLE([debug], AS_HELP_STRING([--enable-debug], [Enable the debug mode]))
AC_MSG_CHECKING([if the debug mode is enabled])
AS_IF([test "x$enable_debug" = "xyes"], [
	CFLAGS="-g -O2"
	AC_MSG_RESULT([yes])
	AC_DEFINE(CMDALIAS_DEBUG,1,[Define to 1 if you want to enable the debug support])
],[
    CFLAGS="-O3"
	AC_MSG_RESULT([no])
])

AC_CONFIG_FILES([Makefile])

AC_OUTPUT
