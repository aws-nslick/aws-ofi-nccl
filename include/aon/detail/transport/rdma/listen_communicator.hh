#pragma once

#include "aon/detail/transport/base/listen_communicator.hh"
#include "aon/detail/transport/base/save_comm_state.hh"
#include "aon/detail/transport/rdma/connection_info.hh"
#include "aon/detail/transport/rdma/recv_communicator.hh"

struct nccl_net_ofi_rdma_listen_comm_t {
  /* This base listen communicator must be the first member of
   * this struct. This allows casting between pointers of this
   * struct and its base struct. */
  nccl_net_ofi_listen_comm_t base;

  /* Comm ID provided by local endpoint */
  uint32_t comm_id;

  /* Communicator created while accept routine is executed */
  nccl_net_ofi_rdma_recv_comm_t *r_comm;

  /* Reusable request for connect and connect response message */
  nccl_net_ofi_rdma_req_t req;

  /* Stage of connection establishment on listen side */
  nccl_ofi_comm_stage_t stage;

  /* Message struct send connect message and receive connect
   * response message */
  nccl_ofi_rdma_connection_info_t conn_msg;
};
