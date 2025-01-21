#pragma once

struct nccl_net_ofi_rdma_req_t;

/*
 * @brief	Data of request responsible for receiving segments
 */
struct rdma_req_recv_segms_data_t {
  /* Pointer to recv parent request */
  nccl_net_ofi_rdma_req_t *recv_req;
};
