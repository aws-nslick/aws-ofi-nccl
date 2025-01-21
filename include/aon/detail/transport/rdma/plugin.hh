#pragma once

#include "nccl_ofi_plugin.hh"
#include "nccl_ofi_topo.hh"

struct nccl_net_ofi_rdma_plugin_t {
  nccl_net_ofi_plugin_t base;

  nccl_ofi_topo_t *topo;
};

/*
 * @brief	Initialize plugin with rdma protocol structures
 */
int nccl_net_ofi_rdma_init(const char *provider_filter, nccl_net_ofi_plugin_t **plugin_p, bool *found_multi_rail);
