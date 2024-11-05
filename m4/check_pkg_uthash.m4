AC_DEFUN([CHECK_PKG_UTHASH], [
  check_pkg_found=yes

  check_pkg_CPPFLAGS_save="${CPPFLAGS}"
  AC_ARG_WITH([uthash],
     [AC_HELP_STRING([--with-uthash=PATH], [Path to non-standard uthash installation])])

  AS_IF([test -n "${with_uthash}"], [NCCL_NET_OFI_DISTCHCK_CONFIGURE_FLAGS="$NCCL_NET_OFI_DISTCHCK_CONFIGURE_FLAGS --with-uthash=${with_uthash}"])

  AS_IF([test -z "${with_uthash}" -o "${with_uthash}" = "yes"],
        [],
        [test "${with_uthash}" = "no"],
        [check_pkg_found=no],
        [CPPFLAGS="-isystem ${with_uthash}/include ${CPPFLAGS}"])

  AS_IF([test "${check_pkg_found}" = "yes"],
        [AC_CHECK_HEADERS([uthash.h], [], [check_pkg_found=no])])

  AS_IF([test "${check_pkg_found}" = "yes"],
        [$1],
        [CPPFLAGS="${check_pkg_CPPFLAGS_save}"
         $2])

  AS_UNSET([check_pkg_found])
  AS_UNSET([check_pkg_CPPFLAGS_save])
])
