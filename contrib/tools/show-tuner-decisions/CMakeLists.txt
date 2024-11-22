cmake_minimum_required(VERSION 3.15)
project(${SKBUILD_PROJECT_NAME} LANGUAGES CXX)
find_package(Python
  REQUIRED COMPONENTS Interpreter Development.Module
  OPTIONAL_COMPONENTS Development.SABIModule)
find_package(nanobind CONFIG REQUIRED)
nanobind_add_module(
  tuner_bindings
  STABLE_ABI
  src/ofi_nccl/tuner_bindings/impl.cc
)
install(TARGETS tuner_bindings LIBRARY DESTINATION ${SKBUILD_PROJECT_NAME})
