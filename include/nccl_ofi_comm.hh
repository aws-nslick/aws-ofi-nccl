#pragma once

#include "nccl_ofi_endpoint.hh"

enum nccl_net_ofi_comm_type_t {
  NCCL_NET_OFI_BASE_COMM,
  NCCL_NET_OFI_LISTEN_COMM,
  NCCL_NET_OFI_SEND_COMM,
  NCCL_NET_OFI_RECV_COMM,
};

/**
 * Communicator - base class for communicator structures
 *
 * This is the base class for the listen, send, and recv
 * communicators.  It should not be directly extended by transports,
 * but instead underlying transports should extend the listen, send,
 * and recv communicators.
 */
struct nccl_net_ofi_comm_t {
  nccl_net_ofi_comm_type_t type;
  nccl_net_ofi_ep_t *ep;
  int dev_id;
};
