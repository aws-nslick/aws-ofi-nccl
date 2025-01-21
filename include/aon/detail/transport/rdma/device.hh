#pragma once

#include "config.hh"
#if HAVE_NVTX_TRACING
#include <nvtx3/nvToolsExt.h>
#endif
#include "nccl_ofi_scheduler.hh"
#include "aon/detail/transport/base/communicator.hh"
#include "aon/detail/transport/base/device.hh"
#include "aon/detail/types/idpool.hh"

/*
 * @brief	RDMA Device
 *
 * Device implementation of the RDMA protocol
 *
 * RDMA device implements the nccl_net_ofi_device_t interface for
 * the rdma protocol that uses libfabric's fi_tsend and fi_trecv
 * for communication. Internally, the rdma device maintains
 * rdma endpoints that are per thread to avoid contention over the
 * endpoint's libfabric resources. Access to endpoints is protected via
 * locks and the lifetime of resources is maintained with a reference
 * counter.
 */
struct nccl_net_ofi_rdma_device_t {
  /* This base device interface struct provides access to the
   * rdma endpoint's functions such as
   * rdma_get_properties(), rdma_get_ep(), and
   * rdma_release_ep(). At construction time of this device,
   * the constructor assigns these functions to the member
   * functions of abstract nccl_net_ofi_device_t device
   * 'device'.
   *
   * This base device must be the first member of this
   * struct. This allows casting between pointers of this struct
   * and its base struct. */
  nccl_net_ofi_device_t base;

  /* Message scheduler */
  nccl_net_ofi_scheduler_t *scheduler;

  /* Number of rails */
  int num_rails;

  /*
   * @brief	Device rail
   *
   * Deivice rail encapsulates data of an endpoint for a
   * specific rail.
   */
  struct nccl_net_ofi_rdma_device_rail_t {
    /* NIC info */
    struct fi_info *info;

    /* Fabric handle */
    struct fid_fabric *fabric;
  };

  /* Array of 'num_rails' device rails */
  nccl_net_ofi_rdma_device_rail_t *device_rails;

  /* Maximum number of supported communicator IDs */
  uint32_t num_comm_ids;

  /* ID pool */
  nccl_ofi_idpool_t *comm_idpool;

  /* Array of open comms associated with this endpoint. This is needed for fast
     lookup of comms in the RDMA protocol. */
  nccl_net_ofi_comm_t **comms;

  bool use_long_rkeys;

#if HAVE_NVTX_TRACING
  nvtxDomainHandle_t nvtx_domain[MAX_NUM_RAILS];
#endif
};
