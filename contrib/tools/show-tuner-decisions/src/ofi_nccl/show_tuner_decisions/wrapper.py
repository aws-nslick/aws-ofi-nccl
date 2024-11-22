#!/usr/bin/env python3

import ctypes
import logging
import pandas as pd
import numpy as np
import os
import pathlib
from enum import Enum
from contextlib import contextmanager
import functools

class NCCLDebugLogLevel(int, Enum):
    NONE = 0
    ERROR = 1
    WARN = 2
    INFO = 3
    DEBUG = 4
    TRACE = 5

class NCCLFunc(int, Enum):
    Broadcast = 0
    Reduce = 1
    AllGather = 2
    ReduceScatter = 3
    AllReduce = 4
    SendRecv = 5
    Send = 6
    Recv = 7

class NCCLAlgo(int, Enum):
    TREE = 0
    RING = 1
    COLLNET_DIRECT = 2
    COLLNET_CHAIN = 3
    NVLS = 4
    NVLS_TREE = 5
    PAT = 6

class NCCLProto(int, Enum):
    LL = 0
    LL128 = 1
    SIMPLE = 2

class Tuner:
    def _debug_logger_callback(self, level, flags, file, line, fmt, *args):
        level_map = {
            NCCLDebugLogLevel.NONE: logging.NOTSET,
            NCCLDebugLogLevel.ERROR: logging.ERROR,
            NCCLDebugLogLevel.WARN: logging.WARNING,
            NCCLDebugLogLevel.INFO: logging.INFO,
            NCCLDebugLogLevel.DEBUG: logging.DEBUG,
            NCCLDebugLogLevel.TRACE: logging.DEBUG  # Python doesn't have a TRACE level
        }
        py_level = level_map.get(level, logging.DEBUG)
        message = f"NCCL [{file.decode('utf-8')}:{line}]: {fmt.decode('utf-8')} {args}"
        self.logger.log(py_level, message)

    def __init__(self, tuner_dso: pathlib.Path, nranks: int, nnodes: int, log_level=logging.DEBUG):
        self.nranks = nranks
        self.nnodes = nnodes

        self.logger = logging.getLogger("NCCLTuner")
        self.logger.setLevel(log_level)
        self.ncclDebugLogger_t = ctypes.CFUNCTYPE(
            None,
            ctypes.c_int,  # ncclDebugLogLevel
            ctypes.c_ulong,  # flags
            ctypes.c_char_p,  # file
            ctypes.c_int,  # line
            ctypes.c_char_p,  # fmt
            ctypes.c_void_p  # varargs
        )
        num_entries = len(NCCLAlgo) * len(NCCLProto)
        self.ncclTuner_v3_t = type('ncclTuner_v3_t', (ctypes.Structure,), {
            '_fields_': [
                ("name", ctypes.c_char_p),
                ("init", ctypes.CFUNCTYPE(ctypes.c_int, ctypes.c_size_t, ctypes.c_size_t, ctypes.c_void_p, ctypes.POINTER(ctypes.c_void_p))),
                ("getCollInfo", ctypes.CFUNCTYPE(ctypes.c_int, ctypes.c_void_p, ctypes.c_int, ctypes.c_size_t, ctypes.c_int, ctypes.POINTER(ctypes.c_float * num_entries), ctypes.c_int, ctypes.c_int, ctypes.POINTER(ctypes.c_int))),
                ("destroy", ctypes.CFUNCTYPE(ctypes.c_int, ctypes.c_void_p))
            ]
        })

        callback_func = self.ncclDebugLogger_t(self._debug_logger_callback)
        self._callback_func = callback_func
        if not self.logger.handlers:
            handler = logging.StreamHandler()
            formatter = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s')
            handler.setFormatter(formatter)
            self.logger.addHandler(handler)

        self.context = ctypes.c_void_p()
        self.lib = ctypes.CDLL(tuner_dso)
        self.tuner = self.ncclTuner_v3_t.in_dll(self.lib, "ncclTunerPlugin_v3")
        result = self.tuner.init(nranks, nnodes, callback_func, ctypes.byref(self.context))
        if result != 0:
            raise RuntimeError(f"Failed to initialize NCCL tuner. Error code: {result}")

    @functools.lru_cache(maxsize=None)
    def get_coll_info(self, coll_type: NCCLFunc, msg_size: int, num_pipe_ops: int = 1):
        if self.context is None:
            raise RuntimeError("NCCL tuner not initialized. Call initialize() first.")

        nBytes = ctypes.c_size_t(msg_size)
        numPipeOps = ctypes.c_int(num_pipe_ops)
        num_entries = len(NCCLAlgo) * len(NCCLProto)
        cost_table_array = (ctypes.c_float * num_entries)()
        for i in range(len(cost_table_array)):
            cost_table_array[i] = float(1337)
        #collCostTable = (ctypes.POINTER(ctypes.c_float) * (len(NCCLAlgo) * len(NCCLProto)))()
        #for i in range(len(NCCLAlgo) * len(NCCLProto)):
        #    collCostTable[i] = ctypes.pointer(ctypes.c_float(0.0))
        nChannels = ctypes.c_int(0)
        result = self.tuner.getCollInfo(
            self.context, coll_type.value, nBytes, numPipeOps,
            ctypes.byref(cost_table_array), len(NCCLAlgo), len(NCCLProto), ctypes.byref(nChannels)
        )

        if result != 0:
            raise RuntimeError(f"Failed to get collective info. Error code: {result}")

        decision = "fallback"
        for algo in NCCLAlgo:
            for proto in NCCLProto:
                cost = cost_table_array[algo.value * len(NCCLProto) + proto.value]
                if cost != float(1337):
                    decision = ((algo, proto))

        return {
            'decision': decision,
            'channels': nChannels.value,
        }

    def analyze_message_range(self, coll_type: NCCLFunc, min_size: int, max_size: int):
        def bisect(min_size, max_size):
            min_decision = self.get_coll_info(coll_type, min_size)
            max_decision = self.get_coll_info(coll_type, max_size)
            print(min_size, min_decision)
            print(max_size, max_decision)

            if min_decision == max_decision or max_size - min_size <= 1:
                return [(min_size, min_decision), (max_size, max_decision)]

            mid_size = (min_size + max_size) // 2
            left_results = bisect(min_size, mid_size)
            right_results = bisect(mid_size, max_size)

            combined = left_results + right_results[1:]
            return [combined[0]] + [combined[i] for i in range(1, len(combined)) if combined[i][1] != combined[i-1][1]]

        edges = bisect(min_size, max_size)
        breakpoint()
        return pd.DataFrame([
            {
                'collective': coll_type.name,
                'message_size': size,
                'decision': decision['decision'],
                #'protocol': decision['protocol'].name,
                'channels': decision['channels'],
                #'cost': decision['cost'],
                'ranks': self.nranks,
                'nodes': self.nnodes
            }
            for size, decision in edges
        ])

    def analyze_all(self, min_size: int = 32, max_size: int = 32*1024*1024*1024):
        return pd.concat([self.analyze_message_range(c, min_size, max_size) for c in NCCLFunc])
