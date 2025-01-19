#pragma once

struct nccl_net_ofi_rdma_req_t;

struct rdma_req_eager_copy_data_t {
  /* Pointer to rx buffer containing eager data */
  nccl_net_ofi_rdma_req_t *eager_rx_buff_req;
  /* Pointer to recv parent request */
  nccl_net_ofi_rdma_req_t *recv_req;
};
