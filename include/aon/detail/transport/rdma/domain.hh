#pragma once

#include "aon/detail/transport/base/domain.hh"
#include "aon/detail/transport/common/endpoint_address_list.hh"

struct nccl_net_ofi_rdma_domain_t {
  nccl_net_ofi_domain_t base;

  int num_rails;
  struct nccl_net_ofi_rdma_domain_rail_t {
    /* Access domain handles */
    struct fid_domain *domain;

    struct fid_cq *cq;
  };
  nccl_net_ofi_rdma_domain_rail_t *domain_rails;

  /* List of endpoints and set of addresses they have connections to */
  nccl_ofi_ep_addr_list_t *ep_addr_list;
};
