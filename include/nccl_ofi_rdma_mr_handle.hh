#pragma once

struct fid_mr;

/*
 * @brief	Rdma memory registration handle
 *
 * Use function `calloc_rdma_mr_handle(int num_rails, int num_control_rails)' to
 * allocate a RDMA memory registration handle with `num_rails`+`num_control_rails` rails.
 */
struct nccl_net_ofi_rdma_mr_handle_t {

  int num_rails;

  int num_control_rails;

  /* Array of size `num_rails' */
  struct fid_mr **mr;

  /* Array of size `num_control_rails' */
  struct fid_mr **control_mr;
};
