#pragma once

struct nccl_net_ofi_rdma_mr_handle_t;

/*
 * @brief	Data of request responsible for flush operation
 */
struct rdma_req_flush_data_t {
  // FIXME:
  /* Buffer to read flush data from */
  void *data;
  /* MR handles for the data buffer */
  nccl_net_ofi_rdma_mr_handle_t *mr_handle;
  /* Total number of completions. Expect completions from all NIC rail */
  int total_num_compls;
};
