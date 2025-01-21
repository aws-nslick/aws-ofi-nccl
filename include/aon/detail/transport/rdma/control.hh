#pragma once

#include "aon/detail/transport/rdma/constants.hh"
#include <cstddef>
#include <cstdint>

/* Contents of ctrl message sent from receiver to sender to advertise
   destination buffer */
struct nccl_net_ofi_rdma_ctrl_msg_t {
  /* Message type, must be NCCL_OFI_RDMA_MSG_CTRL */
  uint32_t type : NCCL_OFI_RDMA_CTRL_TYPE_BITS;

  /* Message sequence number */
  uint32_t msg_seq_num : NCCL_OFI_RDMA_SEQ_BITS;

  /* A comm identitifer that uniquely identifies the comm
   * on the receiver side */
  uint32_t remote_comm_id : NCCL_OFI_RDMA_COMM_ID_BITS;

  uint32_t buff_len;

  uint64_t buff_addr;

  union {
    uint32_t short_buff_mr_key[MAX_NUM_RAILS];
    uint64_t long_buff_mr_key[MAX_NUM_RAILS];
  };
};

static_assert(sizeof(struct nccl_net_ofi_rdma_ctrl_msg_t) == 48, "Wrong size for RDMA Control message");
static_assert(sizeof(struct nccl_net_ofi_rdma_ctrl_msg_t) == 48, "Wrong size for RDMA Control message");

#define NCCL_NET_OFI_CTRL_MSG_SHORT_KEY_SIZE (sizeof(((nccl_net_ofi_rdma_ctrl_msg_t *)0)->short_buff_mr_key[0]))
#define NCCL_NET_OFI_CTRL_MSG_LONG_KEY_SIZE (sizeof(((nccl_net_ofi_rdma_ctrl_msg_t *)0)->long_buff_mr_key[0]))

static inline size_t nccl_net_ofi_rdma_ctrl_msg_size(size_t num_rails, bool use_long_rkeys) {
  size_t rkey_len = (use_long_rkeys) ? NCCL_NET_OFI_CTRL_MSG_LONG_KEY_SIZE : NCCL_NET_OFI_CTRL_MSG_SHORT_KEY_SIZE;
  return offsetof(nccl_net_ofi_rdma_ctrl_msg_t, short_buff_mr_key) + num_rails * rkey_len;
}
