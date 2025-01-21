/*
 * Copyright (c) 2024 Amazon.com, Inc. or its affiliates. All rights reserved.
 */

#pragma once

#include "nccl_ofi_endpoint.hh"

#include <uthash.h>
#include <utlist.h>

/**
 * A Libfabric address, stored in a form hashable by uthash
 */
struct hashed_addr_t {
  UT_hash_handle hh;
  char addr[];
};

/**
 * A linked list of pairs of (ep, HashSet<addr>).
 */
struct ep_pair_list_elem_t {
  nccl_net_ofi_ep_t *ep;
  hashed_addr_t *addr_set;
  ep_pair_list_elem_t *prev;
  ep_pair_list_elem_t *next;
};

/**
 * Outer structure storing the ep list and a mutex to protect access
 */
struct nccl_ofi_ep_addr_list_t {
  ep_pair_list_elem_t *ep_pair_list;
  size_t max_addr_size;
  pthread_mutex_t mutex;
};

/**
 * Initialize an endpoint-address-set pair list. This function allocates memory
 * for the new list
 *
 * nccl_ofi_ep_addr_list_fini should be called to free memory associated with
 * this list.
 *
 * @param max_addr_size: max size of addresses stored in ep_addr_list
 * @return: pointer to the new list
 *          NULL, if error occurred
 */
nccl_ofi_ep_addr_list_t *nccl_ofi_ep_addr_list_init(size_t max_addr_size);

/**
 * Find endpoint in the list ep_pair_list that is not already connected to addr.
 * If an endpoint is found, add this address to its connection list.
 * If all endpoints are already connected to addr, return NULL.
 *
 * @param ep_list Input list
 * @param addr_in Libfabric address
 * @param addr_size Size of address
 * @param ep Output ep
 *	     NULL if no ep found
 * @return 0, on success
 *	   -ENOMEM, memory allocation failure
 *	   -EINVAL, invalid argument
 */
int nccl_ofi_ep_addr_list_get(nccl_ofi_ep_addr_list_t *ep_list, void *addr_in, size_t addr_size, nccl_net_ofi_ep_t **ep);

/**
 * Add ep to the list ep_pair_list, with a single connection to addr.
 *
 * This function makes a copy of the data in addr, so the caller is free to
 * modify the memory at addr as desired.
 *
 * @param ep_list Input list
 * @param ep pointer to endpoint
 * @param addr_in Libfabric address of size MAX_EP_ADDR
 * @return 0, on success
 *	   -ENOMEM, memory allocation failure
 *	   -EINVAL, invalid argument
 */
int nccl_ofi_ep_addr_list_insert(nccl_ofi_ep_addr_list_t *ep_list, nccl_net_ofi_ep_t *ep, void *addr_in, size_t addr_size);

/**
 * Remove ep from the list ep_pair_list, if present
 *
 * @param ep_list Input list
 * @param ep pointer to endpoint
 *
 * @return 0, on success
 *         -ENOENT, ep not found
 */
int nccl_ofi_ep_addr_list_delete(nccl_ofi_ep_addr_list_t *ep_list, nccl_net_ofi_ep_t *ep);

/**
 * Finalize (destroy) an ep addr list
 * @param ep_list list to destroy
 */
void nccl_ofi_ep_addr_list_fini(nccl_ofi_ep_addr_list_t *ep_list);
