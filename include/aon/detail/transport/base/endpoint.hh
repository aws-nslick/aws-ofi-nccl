#pragma once

#include <thread>
#include <uthash.h>

struct nccl_net_ofi_domain_t;
struct nccl_net_ofi_conn_handle_t;
struct nccl_net_ofi_send_comm_t;
struct nccl_net_ofi_listen_comm_t;

/**
 * Endpoint - A per-Proxy Thread device abstraction
 *
 * The device structure is shared across potentially multiple proxy
 * threads (depending on NCCL configuration).  The Endpoint abstracts
 * a unique address (assuming an RDM provider), allowing for the
 * possibility that the underlying transport uses an endpoint per
 * thread (or per thread calling listen/connect) to drive traffic
 * across multiple Libfabric endpoints and completion queues.
 *
 * Endpoints are implicitly created as part of the get_ep() call
 * in the device interface.  Whether they are created during the first
 * call to get_ep() or during initialization is left to the
 * implementation.
 */
struct nccl_net_ofi_ep_t {
  /* Backpointer to the domain associated with this ep. */
  nccl_net_ofi_domain_t *domain;

  /* Create a receiving object and provide a handle to it.
   *
   * The callee can expect that the handle provides
   * NCCL_NET_HANDLE_MAXSIZE bytes and will be exchanged across
   * the wire through an out of band mechanism. The callee must
   * allocate memory for listen_comm.
   *
   * The callee has to guarantee that the state stage of the
   * handle is set to nccl_ofi_comm_stage_t::COMM_CREATE_START.
   */
  int (*listen)(nccl_net_ofi_ep_t *ep, nccl_net_ofi_conn_handle_t *handle, nccl_net_ofi_listen_comm_t **listen_comm);

  /* Create a connection to a process that has called
   * listen().
   *
   * The callee has to guarantee the following invariants when
   * this function returns 0 and no send
   * communicator has been returned
   * 1) The state stage of the handle is set to a value
   * different from nccl_ofi_comm_stage_t::COMM_CREATE_START.
   * 2) The communicator state of the handle stores a pointer to
   * a communicator. Also, the endpoint pointer member variable
   * of that communicator points to the endpoint passed to
   * this connect() function.
   *
   * The callee must allocate memory for send_comm.
   */
  int (*connect)(nccl_net_ofi_ep_t *ep, nccl_net_ofi_conn_handle_t *handle, nccl_net_ofi_send_comm_t **send_comm);

  /*
   * @brief	Release nccl_ofi_ep.
   *
   * Decrease reference counter. Release resources and free
   * endpoint if reference counter becomes zero. Must be
   * protected by lock stored in base_dev.
   */
  int (*release_ep)(nccl_net_ofi_ep_t *ep);

  /* private */
  /* pure virtual function called when resources associated with
   * the ep should be destroyed.  Device lock will be held when
   * this function is called.
   */
  int (*free_ep)(nccl_net_ofi_ep_t *ep);

  /* thread id of the thread that called get_ep().  Used as the
     hash key for the endpoint hash */
  std::thread::id creating_thread_id;

  /* hash table handle */
  UT_hash_handle hh;

  /* Endpoint reference counter for resource management.
   * sendrecv_get_ep()/sendrecv_release_ep() must be called in
   * pair when an object is acquired to use and
   * released. sendrecv_get_ep() allocates a new object when it
   * is called for the first time. sendrecv_get_ep() creates the
   * endpoint libfabric resources if the reference counter was
   * zero. sendrecv_release_ep() releases the resources if the
   * reference counter is decreased down to zero. */
  int ref_cnt;
};

/* base implementation of endpoint release.  endpoint_init() will set
 * the release pointer to this function, although transports can
 * override that function pointer and later call this function
 * directly.
 */
int nccl_net_ofi_endpoint_release(nccl_net_ofi_ep_t *ep);

/* initialize resources associated with the endpoint base class.
 * Expectation is that this will be called by a transport's endpoint
 * creation function */
int nccl_net_ofi_endpoint_init(nccl_net_ofi_domain_t *domain, nccl_net_ofi_ep_t *ep);

/* free resources associated with the endpoint base class.
 * Expectation is that this will be called by a transport's endpoint
 * free function. */
int nccl_net_ofi_endpoint_fini(nccl_net_ofi_ep_t *ep);
