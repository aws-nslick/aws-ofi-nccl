#include <nanobind/nanobind.h>

NB_MODULE(config, m) {
    m.def("get_platform", []() { return "Hello world!"; });
    m.def("set_platform", []() { return "Hello world!"; });
}

