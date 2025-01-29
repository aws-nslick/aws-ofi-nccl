/*
 * Copyright (c) 2024 Amazon.com, Inc. or its affiliates. All rights reserved.
 */

#pragma once

#include "config.hh"

#include <nccl/tuner.h>
#include <linux/limits.h>

/*
 * NCCL 2.19.1 supports ncclTunerPlugin_v1
 * NCCL 2.21.5 supports ncclTunerPlugin_v2 only
 * NCCL 2.22.3 supports ncclTunerPlugin_v3 with fallback to ncclTunerPlugin_v2
 */
NCCL_OFI_EXPORT_SYMBOL extern const ncclTuner_v3_t ncclTunerPlugin_v3;
NCCL_OFI_EXPORT_SYMBOL extern const ncclTuner_v2_t ncclTunerPlugin_v2;
NCCL_OFI_EXPORT_SYMBOL extern const ncclTuner_v1_t ncclTunerPlugin_v1;
