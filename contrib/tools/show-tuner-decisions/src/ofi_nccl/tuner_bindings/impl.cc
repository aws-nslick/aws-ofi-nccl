#include <nanobind/nanobind.h>

NB_MODULE(_ofi_nccl_tuner_bindings, m) {
    m.def("hello", []() { return "Hello world!"; });
}

