#pragma once

#include "config.hh"
#if HAVE_NVTX_TRACING
#include <nvtx3/nvToolsExt.h>
#endif
#include "aon/detail/transport/base/recv_communicator.hh"
#include "aon/detail/transport/rdma/mr_handle.hh"
#include "aon/detail/transport/rdma/request.hh"
#include "aon/detail/types/freelist.hh"
#include "aon/detail/types/msgbuff.hh"

/*
 * @brief	RDMA receive communicator
 *
 * Use function `calloc_rdma_recv_comm(int num_rails, int num_control_rails)' to
 * allocate a RDMA receive communicator with `num_rails'+`num_control_rails' rails.
 */
struct nccl_net_ofi_rdma_recv_comm_t {
  /* This base receive communicator must be the first member of
   * this struct. This allows casting between pointers of this
   * struct and its base struct. */
  nccl_net_ofi_recv_comm_t base;

  uint64_t num_inflight_reqs;
  nccl_ofi_freelist_t *nccl_ofi_reqs_fl;

  /* Comm ID provided by the local endpoint */
  uint32_t local_comm_id;

  /* Comm ID provided by remote endpoint */
  uint32_t remote_comm_id;

  /* Metadata about dummy flush buffer */
  struct nccl_net_ofi_rdma_flush_buffer_t {
    void *host_buffer;
    size_t size;
    /* Memory registration handle of the local buffer */
    nccl_net_ofi_rdma_mr_handle_t *mr_handle;
  };

  /* The flush buffer */
  nccl_net_ofi_rdma_flush_buffer_t flush_buff;

  uint16_t next_msg_seq_num;

  nccl_ofi_msgbuff_t *msgbuff;

  /* Free list to track control buffers, for sending RDMA control messages */
  nccl_ofi_freelist_t *ctrl_buff_fl;

#if HAVE_NVTX_TRACING
  nvtxDomainHandle_t nvtx_domain[NCCL_OFI_N_NVTX_DOMAIN_PER_COMM];
#endif
  nccl_net_ofi_rdma_req_t *send_close_req;

  nccl_ofi_deque_elem_t cleanup_list_elem;

  /* Counters for total sent and received control messages */
  pthread_mutex_t ctrl_counter_lock;
  uint64_t n_ctrl_sent;
  uint64_t n_ctrl_delivered;

  /* Number of rails */
  int num_rails;
  /* Number of control rails */
  int num_control_rails;

  bool comm_active;

  /*
   * @brief	Receive communicator rail
   *
   * Communicator rail encapsulates data of a communicator for a
   * specific rail.
   */
  struct nccl_net_ofi_rdma_recv_comm_rail_t {
    /* Fabric address of remote endpoint */
    fi_addr_t remote_addr;

    /* Pointer to libfabric endpoint of corresponding rdma
     * endpoint rail */
    struct fid_ep *local_ep;

    /* Libfabric address of local endpoint used for flushing */
    fi_addr_t local_addr;
  };

  /* Array of `num_rails` communicator rails */
  nccl_net_ofi_rdma_recv_comm_rail_t *rails;
  /* Array of `num_control_rails` communicator rails */
  nccl_net_ofi_rdma_recv_comm_rail_t *control_rails;
};
