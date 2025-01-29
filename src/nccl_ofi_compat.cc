#include "config.h"

#ifdef HAVE_GETTID
#include <sys/types.h>
#else
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#endif

#include "nccl_ofi.h"

long nccl_net_ofi_gettid(void) {
#ifdef HAVE_GETTID
  return (long)gettid();
#else
  return syscall(SYS_gettid);
#endif
}
