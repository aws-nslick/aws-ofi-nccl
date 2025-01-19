#include "config.hh"

#ifdef HAVE_GETTID
#include <sys/types.h>
#else
#include <cstdlib>
#include <sys/syscall.h>
#endif

#include "nccl_ofi.hh"

long nccl_net_ofi_gettid(void) {
#ifdef HAVE_GETTID
  return (long)gettid();
#else
  return syscall(SYS_gettid);
#endif
}
