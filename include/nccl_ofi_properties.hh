#pragma once

#include <cstddef>
#include <cstdint>

struct nccl_net_ofi_plugin_t;

/**
 * Properties structure
 */
struct nccl_ofi_properties_t {
  char *name;
  /** Path to the device in /sys */
  char *pci_path;
  /** globally unique identifier for NIC */
  std::uint64_t guid;
  /** support device memory */
  bool hmem_support;
  /** support dmabuf interface */
  bool dmabuf_support;
  /** Port number */
  int port_number;
  /** Port speed in Mbps */
  int port_speed;
  /** Port latency */
  float latency;
  /** Maximum number of comms supported */
  unsigned int max_communicators;
  /** Maximum number of grouped receives */
  unsigned int max_group_receives;
  /** regMr is global if is not tied to a particular comm **/
  int regIsGlobal;
  /** Maximum size of buffer supported to be transferred via
   * RMA write inline operation **/
  std::size_t max_write_inline_size;
  /** Maximum size of the memory region remote access key in bytes **/
  std::size_t max_mr_key_size;
  /** Indicator whether RMA operations of NCCL Net API are supported **/
  int rma_supported;
};

/*
 * @brief	Set properties obtained from libfabric NIC Info.
 *
 * @return	Populated props structure
 */
int nccl_net_ofi_info_properties(nccl_net_ofi_plugin_t *plugin, struct fi_info *nic_prov, int dev_id, int num_devices, nccl_ofi_properties_t *props);
