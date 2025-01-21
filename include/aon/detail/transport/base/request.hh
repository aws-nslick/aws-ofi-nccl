#pragma once

/**
 * Request - handle for an outstanding non-blocking communication
 *
 * A request will be allocated and returned for every call to send,
 * recv, or flush.  Memory is allocated by the callee to send, recv,
 * or flush, and will be freed by the callee of test when the request
 * is complete.
 */
struct nccl_net_ofi_req_t {
  int (*test)(nccl_net_ofi_req_t *req, int *done, int *size);
};
