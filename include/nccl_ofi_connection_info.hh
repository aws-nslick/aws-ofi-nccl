#pragma once

#include "nccl_ofi.hh"
#include <cstdint>

struct nccl_net_ofi_req_t;

struct nccl_ofi_connection_info_t {
  char ep_name[MAX_EP_ADDR];
  std::uint64_t ep_namelen;
  std::uint64_t connect_to_self;
  nccl_net_ofi_req_t *req;
};
/* Since this is a message on the wire, check that it has the expected size */
static_assert(sizeof(nccl_ofi_connection_info_t) == 80, "Wrong size for SENDRECV connect message");
