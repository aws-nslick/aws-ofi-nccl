/*
 * Copyright (c) 2023-2024 Amazon.com, Inc. or its affiliates. All rights reserved.
 */

#pragma once

#include <rdma/fabric.h>

#include "nccl_ofi.hh"
#include "nccl_ofi_connection_info.hh"
#include "nccl_ofi_device.hh"
#include "nccl_ofi_domain.hh"
#include "nccl_ofi_endpoint.hh"
#include "nccl_ofi_freelist.hh"
#include "nccl_ofi_listen_comm.hh"
#include "nccl_ofi_plugin.hh"
#include "nccl_ofi_recv_comm.hh"
#include "nccl_ofi_req.hh"
#include "nccl_ofi_save_comm_state.hh"
#include "nccl_ofi_send_comm.hh"

enum nccl_net_ofi_sendrecv_req_state_t {
  NCCL_OFI_SENDRECV_REQ_CREATED = 0,
  NCCL_OFI_SENDRECV_REQ_PENDING,
  NCCL_OFI_SENDRECV_REQ_COMPLETED,
  NCCL_OFI_SENDRECV_REQ_ERROR,
};

enum nccl_net_ofi_sendrecv_req_direction_t {
  NCCL_OFI_SENDRECV_INVALID_DIRECTION = 0,
  NCCL_OFI_SENDRECV_SEND = 1,
  NCCL_OFI_SENDRECV_RECV,
};

struct nccl_net_ofi_sendrecv_listen_comm_t {
  /* This base listen communicator must be the first member of
   * this struct. This allows casting between pointers of this
   * struct and its base struct. */
  nccl_net_ofi_listen_comm_t base;

  uint64_t tag;
  struct fid_ep *local_ep;
  fi_addr_t local_ep_addr;
  bool accepted;
  /* Saves temporary state when creating receive communicator object */
  save_comm_state_t state;
  /* Saves peer address information */
  nccl_ofi_connection_info_t *conn_info;
};

struct nccl_net_ofi_sendrecv_send_comm_t {
  /* This base send communicator must be the first member of this
   * struct. This allows casting between pointers of this struct
   * and its base struct. */
  nccl_net_ofi_send_comm_t base;

  uint64_t num_inflight_reqs;
  nccl_ofi_freelist_t *nccl_ofi_reqs_fl;

  uint64_t tag;
  fi_addr_t remote_ep;
  fi_addr_t local_ep_addr;
  struct fid_ep *local_ep;

  nccl_ofi_connection_info_t *conn_info;
};

/* Metadata about dummy flush buffer */
struct nccl_net_ofi_sendrecv_flush_buffer_t {
  void *host_buffer;
  size_t size;
  /* Memory registration handle of the local buffer */
  struct fid_mr *mr_handle;
};

struct nccl_net_ofi_sendrecv_recv_comm_t {
  /* This base receive communicator must be the first member of
   * this struct. This allows casting between pointers of this
   * struct and its base struct. */
  nccl_net_ofi_recv_comm_t base;

  uint64_t num_inflight_reqs;
  nccl_ofi_freelist_t *nccl_ofi_reqs_fl;

  uint64_t tag;
  fi_addr_t remote_ep;
  fi_addr_t local_ep_addr;
  struct fid_ep *local_ep;

  nccl_net_ofi_sendrecv_flush_buffer_t flush_buff;
};

/**
 * @brief	Sendrecv Endpoint
 *
 * Sendrecv endpoint implements the nccl_net_ofi_ep_t interface
 * for the sendrecv protocol that uses libfabric's fi_tsend and
 * fi_trecv for communication.
 */
struct nccl_net_ofi_sendrecv_ep_t {
  /* This base endpoint interface struct provides access to the
   * sendrecv endpoint's functions such as sendrecv_listen() and
   * sendrecv_connect(). At construction time of this endpoint,
   * the constructor assigns these functions to the member
   * functions of abstract nccl_net_ofi_ep_t endpoint 'base'.
   *
   * This base endpoint must be the first member of this
   * struct. This allows casting between pointers of this struct
   * and its base struct. */
  nccl_net_ofi_ep_t base;

  /* Current available tag ID */
  uint64_t tag;

  /* Endpoint handle to communicate to */
  struct fid_ep *ofi_ep;

  /* Address vector handle */
  struct fid_av *av;

  /* Completion Queue handle */
  struct fid_cq *cq;
};

/*
 * Domain - container for the libfabric domain, which is the threading
 * boundary for most Libfabric providers, given how the util cq
 * implementation works.
 */
struct nccl_net_ofi_sendrecv_domain_t {
  nccl_net_ofi_domain_t base;

  /* Access Domain handle */
  struct fid_domain *domain;
};

/**
 * @brief	Sendrecv Device
 *
 * Device implementation of the Sendrecv protocol
 *
 * Sendrecv device implements the nccl_net_ofi_device_t interface for
 * the sendrecv protocol that uses libfabric's fi_tsend and fi_trecv
 * for communication. Internally, the sendrecv device maintains
 * sendrecv endpoints that are per thread to avoid contention over the
 * endpoint's libfabric resources. Access to endpoints is protected via
 * locks and the lifetime of resources is maintained with a reference
 * counter.
 */
struct nccl_net_ofi_sendrecv_device_t {
  /* This base device interface struct provides access to the
   * sendrecv endpoint's functions such as
   * sendrecv_get_properties(), sendrecv_get_ep(), and
   * sendrecv_release_ep(). At construction time of this device,
   * the constructor assigns these functions to the member
   * functions of abstract nccl_net_ofi_device_t device
   * 'device'.
   *
   * This base device must be the first member of this
   * struct. This allows casting between pointers of this struct
   * and its base struct. */
  nccl_net_ofi_device_t base;

  /* Device provider */
  struct fi_info *info;

  /* Maximum supported tag ID */
  uint64_t max_tag;

  /* Provider name. Device did not obtain ownership. */
  char *prov_name;

  // TODO: So far, devices resources are not released and device
  // memory is not freed. These actions should include closing
  // fabirc, domain, and cq as well as freeing prov_name.

  /* Fabric handle */
  struct fid_fabric *fabric;
};

struct nccl_net_ofi_sendrecv_req_t {
  nccl_net_ofi_req_t base;

  /* Associated Comm object */
  nccl_net_ofi_comm_t *comm;

  /* Associated OFI Context */
  struct fi_context ctx[2];

  /* Associated Device ID */
  int dev_id;

  /* Number of receives associated with request */
  int num_recvs;

  /* Size of completed request */
  size_t size;

  /* State of request */
  nccl_net_ofi_sendrecv_req_state_t state;

  /* Direction of request */
  nccl_net_ofi_sendrecv_req_direction_t direction;

  /* Backpointer to freelist elem (for cleanup) */
  nccl_ofi_freelist_elem_t *elem;
};

struct nccl_net_ofi_sendrecv_plugin {
  nccl_net_ofi_plugin_t base;

  struct fi_info *provider_list;
};
typedef struct nccl_net_ofi_sendrecv_plugin nccl_net_ofi_sendrecv_plugin_t;

/*
 * @brief	Initialize plugin with sendrecv protocol structures
 */
int nccl_net_ofi_sendrecv_init(char const *provider_filter, nccl_net_ofi_plugin_t **plugin_p);
