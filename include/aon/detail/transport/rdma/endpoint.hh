#pragma once

#include "nccl_ofi_deque.hh"
#include "nccl_ofi_endpoint.hh"
#include "nccl_ofi_endpoint_rail.hh"
#include "nccl_ofi_freelist.hh"

/*
 * @brief	RDMA Endpoint
 *
 * RDMA endpoint implements the nccl_net_ofi_ep_t interface
 * for the rdma protocol that uses libfabric's fi_tsend and
 * fi_trecv for communication.
 */
struct nccl_net_ofi_rdma_ep_t {
  /* This base endpoint interface struct provides access to the
   * rdma endpoint's functions such as rdma_listen() and
   * rdma_connect(). At construction time of this endpoint,
   * the constructor assigns these functions to the member
   * functions of abstract nccl_net_ofi_ep_t endpoint 'base'.
   *
   * This base endpoint must be the first member of this
   * struct. This allows casting between pointers of this struct
   * and its base struct. */
  nccl_net_ofi_ep_t base;

  /* Number of rails */
  int num_rails;

  /* Number of control rails */
  int num_control_rails;

  /* Array of `num_rails` endpoint rails */
  nccl_net_ofi_ep_rail_t *rails;

  /* Array of `num_control_rails` endpoint rails */
  nccl_net_ofi_ep_rail_t *control_rails;

  bool use_long_rkeys;

  /* Pending requests queue */
  nccl_ofi_deque_t *pending_reqs_queue;

  /* Free list of rx buffers */
  nccl_ofi_freelist_t *rx_buff_fl;
  /* Free list of rx buffer requests */
  nccl_ofi_freelist_t *rx_buff_reqs_fl;
  /* Size of rx buffers */
  size_t rx_buff_size;

  /* true if the current endpoint is a endpoint_per_communicator
     receive communicator */
  bool is_endpoint_per_communicator_ep;

  /* thread id of the thread that called get_ep().  Used as the
     hash key for the endpoint hash */
  long creating_thread_id;
};
