#pragma once

#include "aon/detail/transport/base/communicator.hh"
#include "aon/detail/transport/common/mr/cache.hh"

struct nccl_net_ofi_mr_handle_t;
struct nccl_net_ofi_req_t;

struct nccl_net_ofi_send_comm_t {
  nccl_net_ofi_comm_t base;

  /*
   * @brief	Register memory region on send communicator (both Host and CUDA)
   *
   * @return	Memory handle for data send operations
   * @return	0 on success
   *		non-zero on error
   */
  int (*regMr)(nccl_net_ofi_send_comm_t *send_comm, nccl_ofi_mr_ckey_ref ckey, int type, void **mhandle);

  /*
   * @brief	Deregister memory region on send communicator (both Host and CUDA)
   *
   * @return	Memory handle for data send operations
   * @return	0 on success
   *		non-zero on error
   */
  int (*deregMr)(nccl_net_ofi_send_comm_t *send_comm, nccl_net_ofi_mr_handle_t *mhandle);

  int (*send)(nccl_net_ofi_send_comm_t *send_comm, void *data, int size, int tag, nccl_net_ofi_mr_handle_t *mhandle, nccl_net_ofi_req_t **req);

  int (*close)(nccl_net_ofi_send_comm_t *send_comm);

  int (*write)(nccl_net_ofi_send_comm_t *send_comm, void *src, size_t size, void *src_mhandle, uint64_t dest, uint64_t mr_key, nccl_net_ofi_req_t **req);
  int (*write_inline)(nccl_net_ofi_send_comm_t *, void *src, size_t size, uint64_t dest, uint64_t mr_key, nccl_net_ofi_req_t **request);
};
