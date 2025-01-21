#pragma once

#include "nccl_ofi_rdma_constants.hh"
#include "nccl_ofi_rdma_endpoint_name.hh"
#include <cstdint>

/*
 * @brief	Message storing rail endpoint addresses for connection establishment
 *
 * Connect message is send from sender to receiver side to provide
 * connection information.
 */
struct nccl_ofi_rdma_connection_info_t {
  /* Message type
   * either NCCL_OFI_RDMA_MSG_CONN or NCCL_OFI_RDMA_MSG_CONN_RESP
   */
  uint16_t type : NCCL_OFI_RDMA_CTRL_TYPE_BITS;
  uint16_t pad : (16 - NCCL_OFI_RDMA_CTRL_TYPE_BITS);

  /* Number of rails */
  uint16_t num_rails;
  uint16_t num_control_rails;

  /* A comm identitifer that uniquely identifies the comm on the sender
     side. The receiver must use this ID when sending messages to sender */
  uint32_t local_comm_id;

  /* A comm identitifer that uniquely identifies the comm
   * on the receiver side */
  uint32_t remote_comm_id;

  /* Arrays of `MAX_NUM_RAILS` `nccl_ofi_rdma_ep_name_t`
   * structs. The member `num_rails` and `num_control_rails` indicate
   * the number of entries that are in use. */
  nccl_ofi_rdma_ep_name_t control_ep_names[MAX_NUM_RAILS];
  nccl_ofi_rdma_ep_name_t ep_names[MAX_NUM_RAILS];
};
/* Since this is a message on the wire, check that it has the expected size */
static_assert(sizeof(nccl_ofi_rdma_connection_info_t) == 528, "Wrong size for RDMA connect message");
