#pragma once

/* Maximum number of rails supported. This defines the size of
 * messages exchanged during connection establishment (linear
 * scaling). The default is set to 4 to support 4 different rails per
 * NCCL comm structure. */
#define MAX_NUM_RAILS (4)

#define NCCL_OFI_RDMA_CTRL_TYPE_BITS (4)

/*
 * @brief      Number of bits used for the communicator ID
 */
#define NCCL_OFI_RDMA_COMM_ID_BITS (18)

#define NCCL_OFI_RDMA_SEQ_BITS (10)

/* For LL/LL128 protocols, eager rx buffers (source of RDMA read operations)
   need to be 128B aligned */
#define EAGER_RX_BUFFER_ALIGNMENT 128
