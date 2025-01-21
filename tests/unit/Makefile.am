######################################################################################
#  Copyright (c) 2018-2023 Amazon.com, Inc. or its affiliates. All rights reserved.  #
#  Copyright (c) 2015-2018, NVIDIA CORPORATION. All rights reserved.                 #
#                                                                                    #
######################################################################################

#
# Copyright (c) 2018-2023, Amazon.com, Inc. or its affiliates. All rights reserved.
#
# See LICENSE.txt for license information
#

if ENABLE_UNIT_TESTS
AM_CPPFLAGS = -I$(top_srcdir)/include
AM_CPPFLAGS += -isystem $(abs_top_srcdir)/3rd-party/nccl/$(DEVICE_INTERFACE)/include
AM_CPPFLAGS += -isystem $(abs_top_srcdir)/3rd-party/uthash/include/uthash
LDADD = $(top_builddir)/src/libinternal_net_plugin.la
noinst_HEADERS = test-common.hh

noinst_PROGRAMS = \
	deque \
	freelist \
	msgbuff \
	scheduler \
	idpool \
	ep_addr_list \
	mr

if WANT_PLATFORM_AWS
noinst_PROGRAMS += aws_platform_mapper
endif

if !ENABLE_NEURON
if WANT_PLATFORM_AWS
  AM_LDFLAGS = $(CUDA_LDFLAGS)
  AM_CPPFLAGS += $(CUDA_CPPFLAGS)
  LDADD += $(CUDA_LIBS)
  noinst_PROGRAMS += region_based_tuner
  region_based_tuner_SOURCES = region_based_tuner.cc
  region_based_tuner_LDADD = $(top_builddir)/src/libinternal_tuner_plugin.la
endif
endif

idpool_SOURCES = idpool.cc
deque_SOURCES = deque.cc
freelist_SOURCES = freelist.cc
msgbuff_SOURCES = msgbuff.cc
scheduler_SOURCES = scheduler.cc
ep_addr_list_SOURCES = ep_addr_list.cc
mr_SOURCES = mr.cc
aws_platform_mapper_SOURCES = aws_platform_mapper.cc

TESTS = $(noinst_PROGRAMS)
endif
