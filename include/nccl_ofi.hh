/*
 * Copyright (c) 2018-2024 Amazon.com, Inc. or its affiliates. All rights reserved.
 */

#pragma once

#include <nccl/net.h>
#include <rdma/fabric.h>
#include <rdma/fi_cm.h>
#include <rdma/fi_domain.h>
#include <rdma/fi_endpoint.h>
#include <rdma/fi_errno.h>
#include <rdma/fi_rma.h>
#include <rdma/fi_tagged.h>
#include <uthash.h>

#include "nccl_ofi_log.hh"
#include "nccl_ofi_topo.hh"
#include "aon/detail/transport/common/mr/cache.hh"
#include "aon/detail/types/idpool.hh"

#include <thread>

/*
 * NCCL_NET_HANDLE_MAXSIZE is a limited resource (and defined in NCCL).
 * An endpoint address buffer of 56 bytes *should* be large enough to hold
 * all libfabric providers. In case the requirement changes, NCCL v2.12
 * provides enough room to increase this size but we would need to maintain
 * backwards compatibility with all NCCL versions.
 *
 * We also store tags and communicator stage information in remaining
 * part of the handle.
 */
#define MAX_EP_ADDR (56)

/*
 * For each tag, we use MSB as control bit and remaining
 * for identifying different rings. We look at mem_tag_format for
 * an endpoint to determine if provider is reserving any MSBs.
 */
#define OFI_HIGHEST_TAG_BIT (0x1UL << 63)

/*
 * We are supporting minimum 2^32 rings per endpoint and reserving 1 bit
 * for marking control sends/recvs.
 */
#define MIN_TAG_BITS_FOR_RING_ID (32 + 1)

/* Maximum number of grouped receives */
#define NCCL_OFI_MAX_RECVS 1

/*
 * This defines a higher value than maximum inflight requests supported by NCCL
 * while not putting a lot of memory pressure. This higher number ensures that
 * we are able to support more number of outstanding requests with dynamic buffer
 * depth changes in NCCL and Neuron.
 */
#define NCCL_OFI_MAX_REQUESTS (128)

/*
 * Number of send requests that can be active at any given time.  In
 * the case of supporting NCCL_OFI_MAX_RECVS grouped receives for each
 * receive request, which means the number of send requests that must
 * be supported is actually larger than the number of receive
 * requests.
 */
#define NCCL_OFI_MAX_SEND_REQUESTS (NCCL_OFI_MAX_REQUESTS * NCCL_OFI_MAX_RECVS)

/* Flush read size (bytes) */
#define NCCL_OFI_FLUSH_SIZE (4ULL)

/* CPU cache line size (bytes) */
#define NCCL_OFI_DEFAULT_CPU_CACHE_LINE_SIZE (64ULL)

/* Initial number of entries in the MR cache of a device */
#define NCCL_OFI_MR_CACHE_INIT_SIZE 128

/* Indicates if GPUDirect is supported by libfabric provider */
enum gdr_support_level_t { GDR_UNKNOWN, GDR_SUPPORTED, GDR_UNSUPPORTED };
extern enum gdr_support_level_t support_gdr;

/* Indicates if the cudaDeviceFlushGPUDirectRDMAWrites function should be used
 * to flush data to the GPU. Note, CUDA flush support is not supported on all
 * platforms and should be disabled by default */
extern bool cuda_flush;

/* number of duplicate providers to create for each discovered
 * provider, including renaming to cause NCCL to create additional
 * rings to use the connections
 */
extern int nic_dup_conns;

/* number of cq entries to read in a single call to fi_cq_read.
   This variable will be updated during init (hence, can not be
   const), but will not change during execution.  Therefore, it may be
   read in the polling loop without protection of a lock. */
extern size_t cq_read_count;

/* Indicates if memory registration of local buffers is required */
extern bool local_mr;

/* Indicates if endpoint memory registration is required */
extern bool endpoint_mr;

/* Indicates if remote virtual addressing is used */
extern bool virt_addr_mr;

/* Selected communication protocol.
 *
 * Until the protocol environment variable is checked in init(), this
 * is the protocol that the plugin will try to initialize; it can be
 * overridden by platform_init().  After init(), this is the protocol
 * that was selected.
 *
 * Valid values are SENDRECV and RDMA; default is SENDRECV (set by the
 * param OFI_NCCL_PROTOCOL)
 */
extern char const *nccl_ofi_selected_protocol;

/* Internode network latency reported to NCCL. */
extern float net_latency;

/* Size of system memory pages */
extern size_t system_page_size;

/*
 * @brief	Allocate memory region for memory registration
 *
 * This function allocates memory that covers full page aligned.
 *
 * Internally allocated memory that is registered is required to cover
 * full memory pages. For more information, see functions
 * `register_internal_mr_buffers()` and `reg_internal_mr_ep()`.
 *
 * To free deallocate the memory region, function
 * nccl_net_ofi_dealloc_mr_buffer() must be used.
 *
 * @param	size
 *		Size of the memory region. Must be a multiple of system memory page size.
 * @return	Pointer to memory region. Memory region is aligned to system memory page size.
 * @return	0, on success
 *		error, on others
 */
int nccl_net_ofi_alloc_mr_buffer(size_t size, void **ptr);

/*
 * @brief	Deallocate memory region allocated by function nccl_net_ofi_alloc_mr_buffer()
 *
 * @return	Pointer to memory region
 * @param	size
 *		Size of the memory region
 * @return	0, on success
 *		error, on others
 */
int nccl_net_ofi_dealloc_mr_buffer(void *ptr, size_t size);

/*
 * @brief       Parse selected provider for required behavior flags
 * @return      0 (Success)
 *
 * Set required behavior flags (and print debugging information) for
 * local_mr, virt_addr_mr, and endpoint_mr.
 */
int nccl_net_ofi_query_provider_capabilities(const struct fi_info *selected_provider, unsigned int num_providers);

/*
 * @brief       Retrieve maximum size of inject RMA operations of ofi endpoint
 *
 * @return      0, on success
 *              -FI_ENOPROTOOPT, in case option to retrieve size is not available
 *              error, on others
 */
int get_inject_rma_size_opt(struct fid_ep *ofi_ep, size_t *max_write_inline_size);
