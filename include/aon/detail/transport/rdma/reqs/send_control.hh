#pragma once

#include "config.hh"
#include "aon/detail/types/freelist.hh"

struct nccl_net_ofi_schedule_t;
struct nccl_net_ofi_rdma_req_t;

/*
 * @brief	Data of request responsible for sending the control message
 */
struct rdma_req_send_ctrl_data_t {
  /* Pointer to the allocated control buffer from freelist */
  nccl_ofi_freelist_elem_t *ctrl_fl_elem;
  /* Schedule used to transfer the control buffer. We save the
   * pointer to reference it when transferring the buffer over
   * network. */
  nccl_net_ofi_schedule_t *ctrl_schedule;
  /* Pointer to recv parent request */
  nccl_net_ofi_rdma_req_t *recv_req;
#if HAVE_NVTX_TRACING
  nvtxRangeId_t trace_id;
#endif
};
