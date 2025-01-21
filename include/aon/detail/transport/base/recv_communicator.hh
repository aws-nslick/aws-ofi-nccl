#pragma once

#include "aon/detail/transport/base/communicator.hh"
#include "aon/detail/transport/base/request.hh"
#include "aon/detail/transport/common/mr/cache.hh"

struct nccl_net_ofi_mr_handle_t;

struct nccl_net_ofi_recv_comm_t {
  nccl_net_ofi_comm_t base;

  /*
   * @brief	Register memory region on recv communicator (both Host and CUDA)
   *
   * @return	Memory handle for data recv operations
   * @return	0 on success
   *		non-zero on error
   */
  int (*regMr)(nccl_net_ofi_recv_comm_t *recv_comm, nccl_ofi_mr_ckey_ref ckey, int type, void **mhandle);

  /*
   * @brief	Deregister memory region on recv communicator (both Host and CUDA)
   *
   * @return	Memory handle for data recv operations
   * @return	0 on success
   *		non-zero on error
   */
  int (*deregMr)(nccl_net_ofi_recv_comm_t *recv_comm, nccl_net_ofi_mr_handle_t *mhandle);

  int (*recv)(nccl_net_ofi_recv_comm_t *recv_comm, int n, void **data, int *sizes, int *tags, nccl_net_ofi_mr_handle_t **mhandles, nccl_net_ofi_req_t **req);

  int (*flush)(nccl_net_ofi_recv_comm_t *recv_comm, int n, void **data, int *sizes, nccl_net_ofi_mr_handle_t **mhandles, nccl_net_ofi_req_t **req);

  int (*close)(nccl_net_ofi_recv_comm_t *recv_comm);

  int (*read)(nccl_net_ofi_recv_comm_t *recv_comm, void *dest, size_t size, void *dest_mhandle, uint64_t src, uint64_t mr_key, nccl_net_ofi_req_t **req);
};
