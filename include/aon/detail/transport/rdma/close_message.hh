#pragma once

#include "nccl_ofi_rdma_constants.hh"
#include <cstdint>

/* Message from receiver to sender indicating sender can close resources */
struct nccl_net_ofi_rdma_close_msg_t {
  /* Message type, must be NCCL_OFI_RDMA_MSG_CLOSE */
  uint16_t type : NCCL_OFI_RDMA_CTRL_TYPE_BITS;

  /* Count of number of ctrl messages sent by the r_comm */
  uint64_t ctrl_counter;

  /* Comm ID provided by the sender */
  uint32_t send_comm_id;
};
