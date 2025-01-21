#pragma once

#include <cstdint>
#include <pthread.h>

#include "nccl_ofi_plugin.hh"

struct nccl_net_ofi_ep_t;
struct nccl_net_ofi_domain_t;
struct nccl_ofi_properties_t;
struct nccl_net_ofi_plugin_t;

/**
 * Device Data
 *
 * A device is roughly a NIC (or a port on a NIC) or a multi-rail
 * group.  The device is the unit of bandwidth sharing and general NIC
 * propoeries, and accessing domains (ie, groups of NIC resources).
 */
struct nccl_net_ofi_device_t {
  nccl_net_ofi_plugin_t *plugin;

  /* this device's index in the plugin's devices array */
  int dev_id;

  /*
   * name of the device - should include the provider name, but may be
   * augmented (in the case of mrail).  Set during the transport's
   * initialization, and should be read-only from that point.
   */
  char *name;

  /* do we need to use an mr rkey pool?  This is a
   * provider-specific behavior determined when providers are
   * selected.
   */
  bool need_mr_rkey_pool;

  int (*get_properties)(nccl_net_ofi_device_t *base_dev, nccl_ofi_properties_t *props);

  /* Retrieve a domain associated with this device.  There may
   * be more than one domain per device, depending on a number
   * of performance tradeoffs (be sure to read the domain
   * description below).
   */
  nccl_net_ofi_domain_t *(*get_domain)(nccl_net_ofi_device_t *dev);

  int (*get_ep)(nccl_net_ofi_device_t *base_dev, nccl_net_ofi_ep_t **ep);

  int (*get_mr_key)(nccl_net_ofi_device_t *base_dev, void *mhandle, std::uint64_t *mr_key);

  /**
   * destructor - releases resources associated with device
   */
  int (*release)(nccl_net_ofi_device_t *device);

  /* Lock for concurrency since domains can be shared by
   * multiple entities. */
  pthread_mutex_t device_lock;

  /* private */
  /*
   * create a new domain.  This function is a private pure
   * virtual function, which is called from the base
   * implementation of get_domain() and should not be called
   * from the more general case.
   */
  nccl_net_ofi_domain_t *(*create_domain)(nccl_net_ofi_device_t *dev);

  /*
   * hash table indexed by thread id of active domains.
   */
  nccl_net_ofi_domain_t *domain_table;
};

/**
 * Constructor for a device object
 */
int nccl_net_ofi_device_init(nccl_net_ofi_device_t *device, nccl_net_ofi_plugin_t *plugin, int device_index, struct fi_info *ofi_info);

/**
 * Destructor for a device object
 */
int nccl_net_ofi_device_fini(nccl_net_ofi_device_t *device);
