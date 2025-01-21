#pragma once

#include "config.hh"
#if HAVE_NVTX_TRACING
#include <nvtx3/nvToolsExt.h>
#endif
#include "nccl_ofi_freelist.hh"
#include "nccl_ofi_msgbuff.hh"
#include "nccl_ofi_rdma_connection_info.hh"
#include "nccl_ofi_rdma_req.hh"
#include "nccl_ofi_send_comm.hh"

/*
 * @brief	RDMA send communicator
 *
 * Use function `calloc_rdma_send_comm(int num_rails, int num_control_rails)' to
 * allocate a RDMA send communicator with `num_rails'+`num_control_rails' rails.
 */
struct nccl_net_ofi_rdma_send_comm_t {
  /* This base send communicator must be the first member of this
   * struct. This allows casting between pointers of this struct
   * and its base struct. */
  nccl_net_ofi_send_comm_t base;

  uint64_t num_inflight_reqs;
  uint64_t num_inflight_writes;

  nccl_ofi_freelist_t *nccl_ofi_reqs_fl;

  /* Comm ID provided by the local endpoint */
  uint32_t local_comm_id;

  /* Comm ID provided by remote endpoint */
  uint32_t remote_comm_id;

  /* Request to receive connect response message to finalize
   * connection establishment */
  nccl_net_ofi_rdma_req_t *conn_resp_req;

  /* Message struct send connect message and receive connect
   * response message */
  nccl_ofi_rdma_connection_info_t conn_msg;

  uint16_t next_msg_seq_num;

  nccl_ofi_msgbuff_t *msgbuff;

  /* Number of rails */
  int num_rails;
  /* Number of rails */
  int num_control_rails;

  /* Number of initialized rails. The function
   * `create_send_comm()' creates a send communicator with one
   * initialized control rail and sets `num_init_control_rails=1' after the
   * out-of-bounds message is received. After the connect
   * response message has been received, the remaining rails
   * will be initialized via function `init_send_comm_rails()'
   * and `num_init_control_rails' is adjusted. */
  int num_init_control_rails;

#if HAVE_NVTX_TRACING
  nvtxDomainHandle_t nvtx_domain[NCCL_OFI_N_NVTX_DOMAIN_PER_COMM];
#endif

  nccl_ofi_deque_elem_t cleanup_list_elem;

  pthread_mutex_t ctrl_recv_lock;
  bool received_close_message;
  /* Counters for total sent and received control messages */
  uint64_t n_ctrl_received;
  uint64_t n_ctrl_expected;

  bool comm_active;

  /*
   * @brief	Send communicator rail
   *
   * Communicator rail encapsulates data of a communicator for a
   * specific rail.
   */
  struct nccl_net_ofi_rdma_send_comm_rail_t {
    /* Fabric address of remote endpoint */
    fi_addr_t remote_addr;

    /* Pointer to libfabric endpoint of corresponding rdma
     * endpoint rail */
    struct fid_ep *local_ep;
  };

  /* Array of `num_rails` communicator rails */
  nccl_net_ofi_rdma_send_comm_rail_t *rails;
  /* Array of `num_control_rails` communicator rails */
  nccl_net_ofi_rdma_send_comm_rail_t *control_rails;
};
