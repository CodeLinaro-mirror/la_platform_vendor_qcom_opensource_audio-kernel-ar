/* SPDX-License-Identifier: GPL-2.0-only WITH Linux-syscall-note */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 */

#ifndef _UAPI_AVTIMER_H
#define _UAPI_AVTIMER_H

#include <linux/ioctl.h>
#include <linux/types.h>

#define MAJOR_NUM 100

#define IOCTL_GET_AVTIMER_TICK _IOR(MAJOR_NUM, 0, __u64)

#endif
