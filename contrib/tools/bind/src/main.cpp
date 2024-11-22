#include <nanobind/nanobind.h>

NB_MODULE(_my_ext_impl, m) {
    m.def("hello", []() { return "Hello world!"; });
}
