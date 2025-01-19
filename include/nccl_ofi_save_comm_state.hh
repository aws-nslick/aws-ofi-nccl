#pragma once

struct nccl_net_ofi_comm_t;
struct nccl_net_ofi_req_t;

enum struct nccl_ofi_comm_stage_t {
  COMM_CREATE_START = 0,
  COMM_SEND_CONN,
  COMM_RECV_CONN,
  COMM_CONN_REQ_PENDING,
  COMM_CONN_RESP_REQ_PENDING,
  COMM_CONNECTED,
};

struct save_comm_state_t {
  nccl_net_ofi_comm_t *comm;
  nccl_net_ofi_req_t *req;
  nccl_ofi_comm_stage_t stage;
};
