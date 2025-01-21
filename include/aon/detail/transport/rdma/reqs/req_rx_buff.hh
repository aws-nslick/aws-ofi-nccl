#pragma once

#include <cstddef>

struct nccl_ofi_freelist_elem_t;
struct nccl_net_ofi_ep_rail_t;
struct nccl_net_ofi_rdma_ep_t;

struct rdma_req_rx_buff_data_t {
  /* Rx buffer freelist item */
  nccl_ofi_freelist_elem_t *rx_buff_fl_elem;
  /* Length of rx buffer */
  size_t buff_len;
  /* Length of received data */
  size_t recv_len;

  /*
   * Keeps tracks of Rail ID which is used to post the rx buffer.
   * This is useful for re-posting the buffer on the same rail
   * when it gets completed.
   */
  nccl_net_ofi_ep_rail_t *rail;
  /*
   * Back-pointer to associated endpoint
   */
  nccl_net_ofi_rdma_ep_t *ep;
};
