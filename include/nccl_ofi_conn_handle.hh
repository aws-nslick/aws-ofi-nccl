#pragma once

#include <cstdint>

#include <nccl/net.h>

#include "nccl_ofi.hh"
#include "aon/detail/transport/base/save_comm_state.hh"

struct nccl_net_ofi_conn_handle_t {
  char ep_name[MAX_EP_ADDR];
  std::uint32_t comm_id;
  /* Save temporary communicator state when creating send communicator */
  save_comm_state_t state;
};

static_assert(sizeof(nccl_net_ofi_conn_handle_t) <= NCCL_NET_HANDLE_MAXSIZE, "Size of OFI Handle is too large");
static_assert(offsetof(nccl_net_ofi_conn_handle_t, state) <= NCCL_NET_HANDLE_MAXSIZE_V4, "Size of OFI Handle (without state) is too large");
static_assert(NCCL_NET_MAX_REQUESTS <= NCCL_OFI_MAX_REQUESTS, "Maximum outstanding requests for plugin is less than what NCCL requires");
