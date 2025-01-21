#pragma once

#include "aon/detail/types/idpool.hh"
#include <pthread.h>
#include <thread>
#include <uthash.h>

struct nccl_net_ofi_device_t;
struct nccl_net_ofi_ep_t;
struct nccl_ofi_mr_cache_t;

/**
 * Domain Object - Represents a protection and thread safety domain
 *
 * A domain is a weird combination of a Libfabric domain (and related
 * resources like an AV and CQ) as well as a general thread boundary.
 * Transports are free to implement fine grained threads, but
 * generally it is expected that calls into resources that share the
 * same domain will share the same lock.
 */
struct nccl_net_ofi_domain_t {
  /* Backpointer to the device associated with this domain. */
  nccl_net_ofi_device_t *device;

  /*
   * Retrieve an endpoint for this domain.  If a suitable
   * endpoint does not exist, call create_endpoint() to create
   * one and return that endpoint.  This function is a pure
   * virtual function that must be implemented by inheriting
   * classes.
   */
  int (*get_ep)(nccl_net_ofi_domain_t *domain, nccl_net_ofi_ep_t **endpoint);

  /*
   * Destructor - release resources associated with the domain
   */
  int (*release)(nccl_net_ofi_domain_t *domain);

  /*
   * Protocol-agnostic MR cache for this device.
   */
  nccl_ofi_mr_cache_t *mr_cache;

  /* Memory registration key pool */
  nccl_ofi_idpool_t mr_rkey_pool;

  pthread_mutex_t domain_lock;

  /* Private */
  /* pure virtual function called when resources associated with
   * the ep should be destroyed.  Device lock will be held when
   * this function is called.
   */
  int (*free)(nccl_net_ofi_domain_t *domain);

  /* Create a new endpoint
   *
   * Pure virtual function to allocate a new endpoint structure
   */
  int (*create_endpoint)(nccl_net_ofi_domain_t *domain, nccl_net_ofi_ep_t **ep);

  /* hash table of active endpoints.  We reuse endpoints based
   * on the thread that calls get_ep().
   */
  nccl_net_ofi_ep_t *endpoint_table;

  /* thread id of the thread that called get_domain().  Used as
     the hash key for the domain hash */
  std::thread::id creating_thread_id;

  /* hash table handle */
  UT_hash_handle hh;
};

/* initialize resources associated with the domain base class.
 * Expectation is that this will be called by a transport's domain
 * creation routine */
int nccl_net_ofi_domain_init(nccl_net_ofi_device_t *device, nccl_net_ofi_domain_t *domain);

/* free resources associated with the domain base class.  Expectation
 * is that this will be called by a transport's domain free
 * function. */
int nccl_net_ofi_domain_fini(nccl_net_ofi_domain_t *domain);
