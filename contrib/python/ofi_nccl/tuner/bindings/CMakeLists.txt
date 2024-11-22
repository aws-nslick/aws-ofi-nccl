find_package(Python
  REQUIRED COMPONENTS Interpreter Development.Module
  OPTIONAL_COMPONENTS Development.SABIModule)
find_package(nanobind CONFIG REQUIRED)
nanobind_add_module(
  impl
  STABLE_ABI
  impl.cc
)
install(TARGETS impl LIBRARY DESTINATION ${SKBUILD_PROJECT_NAME}/tuner/bindings)
