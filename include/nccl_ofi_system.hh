/*
 * Copyright (c) 2018-2024 Amazon.com, Inc. or its affiliates. All rights reserved.
 */

#pragma once

/*
 * @brief   Reads the product name from the DMI information.
 *          The caller must free the returned string.
 *          Provides the manufacturer-assigned product name
 *          /sys/devices/virtual/dmi/id/product_name.
 *          Users of this API *should* free the buffer when a
 *          Non-NULL string is returned.
 *
 * @return  NULL, on allocation and file system error
 *          product name, on success
 */
char const *nccl_net_ofi_get_product_name(void);
