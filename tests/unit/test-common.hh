/*
 * Copyright (c) 2018-2023 Amazon.com, Inc. or its affiliates. All rights reserved.
 */

#pragma once

#include <cstdarg>
#include <cstdio>

#include "nccl_ofi.hh"
#include "nccl_ofi_log.hh"

static inline void logger(ncclDebugLogLevel level, unsigned long flags, const char *filefunc, int line, const char *fmt, ...) {
  va_list vargs;

  switch (level) {
  case NCCL_LOG_WARN:
    printf("WARN: Function: %s Line: %d: ", filefunc, line);
    break;
  case NCCL_LOG_INFO:
    printf("INFO: Function: %s Line: %d: ", filefunc, line);
    break;
  case NCCL_LOG_TRACE:
#if OFI_NCCL_TRACE
    printf("TRACE: Function: %s Line: %d: ", filefunc, line);
    break;
#else
    return;
#endif
  case NCCL_LOG_NONE:
  case NCCL_LOG_VERSION:
  case NCCL_LOG_ABORT:
  default:
    break;
  };

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat=2"
  va_start(vargs, fmt);
  vprintf(fmt, vargs);
  printf("\n");
  va_end(vargs);
#pragma GCC diagnostic pop
}
