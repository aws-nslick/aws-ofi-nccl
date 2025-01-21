/*
 * Copyright (c) 2018 Amazon.com, Inc. or its affiliates. All rights reserved.
 */

#include "config.hh"
#include "aon/detail/transport/base/listen_communicator.hh"
#include "aon/detail/transport/base/send_communicator.hh"

#include "test-common.hh"

#define PROC_NAME_IDX(i) ((i) * MPI_MAX_PROCESSOR_NAME)

int main(int argc, char *argv[]) {
  ncclResult_t res = ncclSuccess;
  int rank = 0, size = 0, next = 0, prev = 0, proc_name_len = 0, local_rank = 0;
  int buffer_type = NCCL_PTR_HOST;

  /* Plugin defines */
  int ndev = 0;

  nccl_net_ofi_send_comm_t *sComm_next = NULL;
  nccl_net_ofi_listen_comm_t *lComm = NULL;
  nccl_net_ofi_send_comm_t *rComm = NULL;
  char handle[NCCL_NET_HANDLE_MAXSIZE] = {};
  char src_handle_prev[NCCL_NET_HANDLE_MAXSIZE] = {};
  char src_handle_next[NCCL_NET_HANDLE_MAXSIZE] = {};
  ncclNetDeviceHandle_v8_t *s_ignore, *r_ignore;
  test_nccl_net_t *extNet = nullptr;

  ofi_log_function = logger;

  /* Initialisation for data transfer */
  nccl_net_ofi_req_t *send_req[NUM_REQUESTS] = {NULL};
  nccl_net_ofi_req_t *recv_req[NUM_REQUESTS] = {NULL};
  void *send_mhandle[NUM_REQUESTS];
  void *recv_mhandle[NUM_REQUESTS];
  int req_completed_send[NUM_REQUESTS] = {};
  int req_completed_recv[NUM_REQUESTS] = {};
  int inflight_reqs = NUM_REQUESTS * 2;
  char *send_buf[NUM_REQUESTS] = {NULL};
  char *recv_buf[NUM_REQUESTS] = {NULL};
  char *expected_buf = nullptr;
  int done = 0, received_size = 0;

  /* Indicates if NICs support GPUDirect */
  int *test_support_gdr = nullptr;

  /* All processors IDs, used to find out the local rank */
  char *all_proc_name = nullptr;

  /* For grouped receives */
  int tag = 1;
  int nrecv = NCCL_OFI_MAX_RECVS;
  int *sizes = (int *)malloc(sizeof(int) * nrecv);
  int *tags = (int *)malloc(sizeof(int) * nrecv);
  if (sizes == nullptr || tags == nullptr) {
    NCCL_OFI_WARN("Failed to allocate memory");
    return ncclInternalError;
  }

  for (int recv_n = 0; recv_n < nrecv; recv_n++) {
    sizes[recv_n] = RECV_SIZE;
    tags[recv_n] = tag;
  }

  /* Start up MPI */
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  if (size < 2) {
    NCCL_OFI_WARN("Expected at least two ranks but got %d. "
                  "The ring functional test should be run with at least two ranks.",
                  size);
    return ncclInvalidArgument;
  }

  all_proc_name = (char *)malloc(sizeof(char) * size * MPI_MAX_PROCESSOR_NAME);
  if (all_proc_name == nullptr) {
    NCCL_OFI_WARN("Failed to allocate memory");
    return ncclInternalError;
  }

  MPI_Get_processor_name(&all_proc_name[PROC_NAME_IDX(rank)], &proc_name_len);
  MPI_Allgather(MPI_IN_PLACE, 0, MPI_DATATYPE_NULL, all_proc_name, MPI_MAX_PROCESSOR_NAME, MPI_BYTE, MPI_COMM_WORLD);

  /* Determine local rank */
  for (int i = 0; i < size; i++) {
    if (!strcmp(&all_proc_name[PROC_NAME_IDX(rank)], &all_proc_name[PROC_NAME_IDX(i)])) {
      if (i < rank) {
        ++local_rank;
      }
    }
  }

  /* Set CUDA device for subsequent device memory allocation, in case GDR is used */
  NCCL_OFI_TRACE(NCCL_NET, "Using CUDA device %d for memory allocation", local_rank);

  /* Allocate and populate expected buffer */
  OFINCCLCHECKGOTO(allocate_buff((void **)&expected_buf, SEND_SIZE, NCCL_PTR_HOST), res);
  OFINCCLCHECKGOTO(initialize_buff((void *)expected_buf, SEND_SIZE, NCCL_PTR_HOST), res);

  /*
   * Calculate the rank of the next process in the ring.  Use the
   * modulus operator so that the last process "wraps around" to
   * rank zero.
   */
  next = (rank + 1) % size;
  prev = (rank + size - 1) % size;

  /* Get external Network from NCCL-OFI library */
  extNet = get_extNet();
  if (extNet == nullptr) {
    res = ncclInternalError;
    goto exit;
  }

  /* Init API */
  OFINCCLCHECKGOTO(extNet->init(&logger), res);
  NCCL_OFI_INFO(NCCL_NET, "Process rank %d started. NCCLNet device used on %s is %s.", rank, &all_proc_name[PROC_NAME_IDX(rank)], extNet->name);

  /* Devices API */
  OFINCCLCHECKGOTO(extNet->devices(&ndev), res);
  NCCL_OFI_INFO(NCCL_NET, "Received %d network devices", ndev);

  /* Indicates if NICs support GPUDirect */
  test_support_gdr = (int *)malloc(sizeof(int) * ndev);
  if (test_support_gdr == nullptr) {
    NCCL_OFI_WARN("Failed to allocate memory");
    res = ncclInternalError;
    goto exit;
  }

  /* Get Properties for the device */
  for (int dev = 0; dev < ndev; dev++) {
    test_nccl_properties_t props = {};
    OFINCCLCHECKGOTO(extNet->getProperties(dev, &props), res);
    print_dev_props(dev, &props);

    /* Set CUDA support */
    test_support_gdr[dev] = is_gdr_supported_nic(props.ptrSupport);
  }

  /* Test all devices */
  for (int dev_idx = 0; dev_idx < ndev; dev_idx++) {

    int dev = (local_rank + dev_idx) % ndev;

    NCCL_OFI_TRACE(NCCL_INIT, "Rank %d uses %d device for communication", rank, dev);

    if (test_support_gdr[dev] == 1) {
      NCCL_OFI_INFO(NCCL_INIT | NCCL_NET, "Network supports communication using CUDA buffers. Dev: %d", dev);
      buffer_type = NCCL_PTR_CUDA;
    }

    /* Listen API */
    NCCL_OFI_INFO(NCCL_NET, "Server: Listening on device %d", dev);
    OFINCCLCHECKGOTO(extNet->listen(dev, (void *)&handle, (void **)&lComm), res);

    /* MPI send: Distribute handle to prev and next ranks */
    MPI_Send(&handle, NCCL_NET_HANDLE_MAXSIZE, MPI_CHAR, prev, 0, MPI_COMM_WORLD);
    MPI_Send(&handle, NCCL_NET_HANDLE_MAXSIZE, MPI_CHAR, next, 0, MPI_COMM_WORLD);

    /* MPI recv: Receive handle from prev and next ranks */
    MPI_Recv((void *)src_handle_prev, NCCL_NET_HANDLE_MAXSIZE, MPI_CHAR, prev, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Recv((void *)src_handle_next, NCCL_NET_HANDLE_MAXSIZE, MPI_CHAR, next, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    NCCL_OFI_INFO(NCCL_NET, "Send connection request to rank %d", next);
    NCCL_OFI_INFO(NCCL_NET, "Server: Start accepting requests");

    while (sComm_next == NULL || rComm == NULL) {
      /* Connect to next rank */
      if (sComm_next == NULL) {
        OFINCCLCHECKGOTO(extNet->connect(dev, (void *)src_handle_next, (void **)&sComm_next, &s_ignore), res);
      }

      /*
       * Accept API: accept connection from prev rank as the data flow is
       * clockwise
       */
      if (rComm == NULL) {
        OFINCCLCHECKGOTO(extNet->accept((void *)lComm, (void **)&rComm, &r_ignore), res);
      }
    }

    NCCL_OFI_INFO(NCCL_NET, "Successfully accepted connection from rank %d", prev);

    /* Send NUM_REQUESTS to next rank */
    NCCL_OFI_INFO(NCCL_NET, "Sending %d requests to rank %d", NUM_REQUESTS, next);
    for (int idx = 0; idx < NUM_REQUESTS; idx++) {
      OFINCCLCHECKGOTO(allocate_buff((void **)&send_buf[idx], SEND_SIZE, buffer_type), res);
      OFINCCLCHECKGOTO(initialize_buff((void *)send_buf[idx], SEND_SIZE, buffer_type), res);

      OFINCCLCHECKGOTO(extNet->regMr((void *)sComm_next, (void *)send_buf[idx], SEND_SIZE, buffer_type, &send_mhandle[idx]), res);
      NCCL_OFI_TRACE(NCCL_NET, "Successfully registered send memory for request %d of rank %d", idx, rank);

      while (send_req[idx] == NULL) {
        OFINCCLCHECKGOTO(extNet->isend((void *)sComm_next, (void *)send_buf[idx], SEND_SIZE, tag, send_mhandle[idx], (void **)&send_req[idx]), res);
      }
    }

    /* Receive NUM_REQUESTS from prev rank */
    NCCL_OFI_INFO(NCCL_NET, "Rank %d posting %d receive buffers", rank, NUM_REQUESTS);
    for (int idx = 0; idx < NUM_REQUESTS; idx++) {
      OFINCCLCHECKGOTO(allocate_buff((void **)&recv_buf[idx], RECV_SIZE, buffer_type), res);
      OFINCCLCHECKGOTO(extNet->regMr((void *)rComm, (void *)recv_buf[idx], RECV_SIZE, buffer_type, &recv_mhandle[idx]), res);
      NCCL_OFI_TRACE(NCCL_NET, "Successfully registered receive memory for request %d of rank %d", idx, rank);

      while (recv_req[idx] == NULL) {
        OFINCCLCHECKGOTO(extNet->irecv((void *)rComm, nrecv, (void **)&recv_buf[idx], sizes, tags, &recv_mhandle[idx], (void **)&recv_req[idx]), res);
      }
    }

    /* Test all completions */
    while (inflight_reqs > 0) {
      /* Test for send completions */
      for (int idx = 0; idx < NUM_REQUESTS; idx++) {
        if (req_completed_send[idx])
          continue;

        OFINCCLCHECKGOTO(extNet->test((void *)send_req[idx], &done, &received_size), res);
        if (done) {
          inflight_reqs--;
          req_completed_send[idx] = 1;
          /* Deregister memory handle */
          OFINCCLCHECKGOTO(extNet->deregMr((void *)sComm_next, send_mhandle[idx]), res);
        }
      }

      /* Test for recv completions */
      for (int idx = 0; idx < NUM_REQUESTS; idx++) {
        if (req_completed_recv[idx])
          continue;

        OFINCCLCHECKGOTO(extNet->test((void *)recv_req[idx], &done, &received_size), res);
        if (done) {
          inflight_reqs--;
          req_completed_recv[idx] = 1;

          /* Invoke flush operations unless user has explicitly disabled it */
          if (buffer_type == NCCL_PTR_CUDA) {
            NCCL_OFI_TRACE(NCCL_NET, "Issue flush for data consistency. Request idx: %d", idx);
            nccl_net_ofi_req_t *iflush_req = NULL;
            OFINCCLCHECKGOTO(extNet->iflush((void *)rComm, nrecv, (void **)&recv_buf[idx], sizes, &recv_mhandle[idx], (void **)&iflush_req), res);
            done = 0;
            if (iflush_req) {
              while (!done) {
                OFINCCLCHECKGOTO(extNet->test((void *)iflush_req, &done, NULL), res);
              }
            }
          }

          if ((buffer_type == NCCL_PTR_CUDA) && !ofi_nccl_gdr_flush_disable()) {
            /* Data validation may fail if flush operations are disabled */
          } else
            OFINCCLCHECKGOTO(validate_data(recv_buf[idx], expected_buf, SEND_SIZE, buffer_type), res);

          /* Deregister memory handle */
          OFINCCLCHECKGOTO(extNet->deregMr((void *)rComm, recv_mhandle[idx]), res);
        }
      }
    }

    for (int idx = 0; idx < NUM_REQUESTS; idx++) {
      if (send_buf[idx]) {
        OFINCCLCHECKGOTO(deallocate_buffer(send_buf[idx], buffer_type), res);
        send_buf[idx] = NULL;
      }
      if (recv_buf[idx]) {
        OFINCCLCHECKGOTO(deallocate_buffer(recv_buf[idx], buffer_type), res);
        recv_buf[idx] = NULL;
      }
    }

    /* Close all Comm objects */
    OFINCCLCHECKGOTO(extNet->closeSend((void *)sComm_next), res);
    sComm_next = NULL;
    OFINCCLCHECKGOTO(extNet->closeRecv((void *)rComm), res);
    rComm = NULL;
    OFINCCLCHECKGOTO(extNet->closeListen((void *)lComm), res);
    lComm = NULL;

    MPI_Barrier(MPI_COMM_WORLD);
  }

  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Finalize();

  NCCL_OFI_INFO(NCCL_NET, "Test completed successfully for rank %d", rank);

exit:
  ncclResult_t close_res = ncclSuccess;

  /* Deallocate buffers */
  for (int idx = 0; idx < NUM_REQUESTS; idx++) {
    if (send_buf[idx]) {
      close_res = deallocate_buffer(send_buf[idx], buffer_type);
      if (close_res != ncclSuccess) {
        NCCL_OFI_WARN("Send buffer deallocation failure: %d", close_res);
        res = res ? res : close_res;
      }
      send_buf[idx] = NULL;
    }
    if (recv_buf[idx]) {
      close_res = deallocate_buffer(recv_buf[idx], buffer_type);
      if (close_res != ncclSuccess) {
        NCCL_OFI_WARN("Recv buffer deallocation failure: %d", close_res);
        res = res ? res : close_res;
      }
      recv_buf[idx] = NULL;
    }
  }

  if (expected_buf) {
    close_res = deallocate_buffer(expected_buf, NCCL_PTR_HOST);
    if (close_res != ncclSuccess) {
      NCCL_OFI_WARN("Expected buffer deallocation failure: %d", close_res);
      res = res ? res : close_res;
    }
    expected_buf = nullptr;
  }

  if (test_support_gdr) {
    free(test_support_gdr);
    test_support_gdr = nullptr;
  }

  if (all_proc_name) {
    free(all_proc_name);
    all_proc_name = nullptr;
  }

  if (sizes) {
    free(sizes);
    sizes = nullptr;
  }

  if (tags) {
    free(tags);
    tags = nullptr;
  }
  return res;
}
