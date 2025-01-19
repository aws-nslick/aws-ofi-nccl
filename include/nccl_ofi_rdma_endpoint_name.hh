#pragma once

#include "nccl_ofi.hh"
#include <cstddef>

/*
 * Rdma endpoint name
 *
 * Length of the name is limited to `MAX_EP_ADDR`.
 */
struct nccl_ofi_rdma_ep_name_t {
  char ep_name[MAX_EP_ADDR];
  size_t ep_name_len;
};
