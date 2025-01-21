#pragma once

#include <cstddef>

struct nccl_net_ofi_device_t;

/**
 * Top-level plugin data
 *
 * Data associated with an instance of the plugin (which may involve
 * multiple proxy threads and multiple devices).  There will be a
 * single instance of this structure, exposed as a global variable
 * named nccl_net_ofi_plugin, which is valid after NCCL calls init()
 * on the plugin.
 */
struct nccl_net_ofi_plugin_t {
  /* public */

  /**
   * Complete initialization of plugin
   *
   * When a plugin is first created, it should not create any
   * network resources -- create is called to understand the
   * configuration of the network and see which transports can
   * run.  The base code will pick one and call complete_init,
   * at which point devices and network resources can be
   * created.
   */
  int (*complete_init)(nccl_net_ofi_plugin_t *plugin);

  int (*assign_device)(nccl_net_ofi_plugin_t *plugin, std::size_t device_index, nccl_net_ofi_device_t *device);

  nccl_net_ofi_device_t *(*get_device)(nccl_net_ofi_plugin_t *plugin, std::size_t device_index);

  std::size_t (*get_num_devices)(nccl_net_ofi_plugin_t *plugin);

  int (*release_plugin)(nccl_net_ofi_plugin_t *plugin);

  /*
   * Determine whether to allocate the domain per process or per
   * thread.
   * false: allocate domain per process
   * true: allocate domain per thread
   */
  bool domain_per_thread;

  /* private */
  /* Array of devices */
  nccl_net_ofi_device_t **p_devs;

  /* Number of devices in devs array */
  std::size_t p_num_devs;
};

/*
 * Create a plugin object
 *
 * Create a plugin object and initialize all the resources,
 * including devices, required for operation.  This function will pick
 * the correct transport and call its create function to actually
 * create the plugin (which is a little hacky, but it works).
 */
int nccl_net_ofi_create_plugin(nccl_net_ofi_plugin_t **plugin_p);

/*
 * Constructor for the nccl_net_ofi_plugin class
 *
 * Construct a nccl_net_ofi_plugin object.  This is expected to be
 * called from the transport-specific plugin creation function, which
 * is called from nccl_net_ofi_create_plugin().
 */
int nccl_net_ofi_plugin_init(nccl_net_ofi_plugin_t *plugin, size_t num_devices);

/*
 * Destructor for the nccl_net_ofi_plugin class
 *
 * Destruct a nccl_net_ofi_plugin object.  This is expected to be
 * called from the transport-specific plugin destructor.
 */
int nccl_net_ofi_plugin_fini(nccl_net_ofi_plugin_t *plugin);
