#pragma once

#include <cstdint>

/*
 * @brief	Number of bits used for message sequence number
 *
 * The immediate data associated with an RDMA write operation is 32
 * bits and is divided into three parts, the segment count, the
 * communicator ID, and the message sequence number (msg_seq_num).
 * The data is encoded as follows:
 *
 * | 4-bit segment count | 18-bit comm ID | 10-bit msg_seq_num |
 *
 * - Segment count: number of RDMA writes that will be delivered as part of this message
 * - Comm ID: the ID for this communicator
 * - Message sequence number: message identifier
 */
enum nccl_ofi_rdma_msg_type {
  NCCL_OFI_RDMA_MSG_CONN = 0,
  NCCL_OFI_RDMA_MSG_CONN_RESP,
  NCCL_OFI_RDMA_MSG_CTRL,
  NCCL_OFI_RDMA_MSG_EAGER,
  NCCL_OFI_RDMA_MSG_CLOSE,
  NCCL_OFI_RDMA_MSG_INVALID = 15,
  NCCL_OFI_RDMA_MSG_MAX = NCCL_OFI_RDMA_MSG_INVALID,
};

static_assert(NCCL_OFI_RDMA_MSG_MAX <= (0x10), "Out of space in nccl_ofi_rdma_msg_type; must fit in a nibble");

/* This goes on the wire, so we want the datatype
 * size to be fixed.
 */
// FIXME: it doesn't work like that^, this doesn't do anything to make this more "wire-friendly".
typedef uint16_t nccl_ofi_rdma_msg_type_t;
