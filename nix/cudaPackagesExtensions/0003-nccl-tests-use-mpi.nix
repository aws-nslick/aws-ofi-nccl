{ config, ... }:
_ffinal: pprev: {
  nccl-tests = pprev.nccl-tests.override {
    mpiSupport = true;
    mpi = config.packages.openmpi;
  };
}
