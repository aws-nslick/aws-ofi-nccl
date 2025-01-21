#pragma once

#include "nccl_ofi_comm.hh"

struct nccl_net_ofi_recv_comm_t;

/**
 * Listen Communicator - Communicator for a listen/accept pairing
 */
struct nccl_net_ofi_listen_comm_t {
  nccl_net_ofi_comm_t base;

  int (*accept)(nccl_net_ofi_listen_comm_t *listen_comm, nccl_net_ofi_recv_comm_t **recv_comm);
  int (*close)(nccl_net_ofi_listen_comm_t *listen_comm);
};
