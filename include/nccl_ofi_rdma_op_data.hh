#pragma once

#include <cstddef>
#include <cstdint>

struct rdma_req_rma_op_data_t {
  /* Remote destination buffer address */
  uint64_t remote_buff;
  /* Remote MR key */
  uint64_t remote_mr_key;
  /* Number of rails where we have successfully posted the network xfer.
   * Used mostly when the network xfer is sliced across multiple rails */
  uint64_t xferred_rail_id;
  /* Application-provided local src/dst buffer */
  void *buff;
  /* Length of application-provided buffer */
  size_t buff_len;
  /* First rail descriptor from memory registration of `buff' */
  void *desc;
  /* Additional flags */
  uint64_t flags;
  /* Total number of completions. Expect one completion for receiving the
   * control message and one completion for each send segment. */
  int total_num_compls;
};
