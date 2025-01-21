#pragma once

#include <cstddef>
#include <pthread.h>

#include "nccl_ofi.hh" // for MAX_EP_ADDR

struct fid_ep;
struct fid_av;
struct fid_cq;
struct fid_domain;

/*
 * @brief	Endpoint rail
 *
 * Endpoint rail encapsulates data of an endpoint for a
 * specific rail.
 */
struct nccl_net_ofi_ep_rail_t {
  int rail_id;

  /* Local libfabric endpoint handle */
  struct fid_ep *ofi_ep;

  /* Name of local libfabric endpoint */
  char local_ep_name[MAX_EP_ADDR];

  /* Length of local_ep_name */
  std::size_t local_ep_name_len;

  /* Address vector handle */
  struct fid_av *av;

  /* Completion Queue handle */
  struct fid_cq *cq;

  /* Access domain handles */
  struct fid_domain *domain;

  /*
   * Rx buffer management
   */
  /* Number of rx buffers posted */
  size_t num_rx_buff_posted;
  /* Minimum posted rx buffers (see RDMA_MIN_POSTED_BOUNCE_BUFFERS) */
  size_t min_rx_buff_posted;
  /* Maximum posted rx buffers (see RDMA_MAX_POSTED_BOUNCE_BUFFERS) */
  size_t max_rx_buff_posted;
  /* Mutex for rx buffer operations */
  pthread_mutex_t rx_buff_mutex;
};
