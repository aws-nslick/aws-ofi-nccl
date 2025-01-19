#pragma once

struct nccl_ofi_freelist_elem_t;
struct nccl_net_ofi_schedule_t;

/*
 * @brief	Data of request responsible for sending the close message
 */
struct rdma_req_send_close_data_t {
  /* Pointer to the allocated control buffer from freelist */
  nccl_ofi_freelist_elem_t *ctrl_fl_elem;
  /* Schedule used to transfer the close buffer. We save the
   * pointer to reference it when transferring the buffer over
   * network. */
  nccl_net_ofi_schedule_t *ctrl_schedule;
};
