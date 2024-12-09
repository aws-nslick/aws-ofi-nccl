{ pkgs, ... }:
ffinal: pprev: {
  nccl-tests-aws = pkgs.replaceDependency {
    drv = ffinal.nccl-tests;
    oldDependency = ffinal.nccl;
    newDependency = ffinal.ncclAws;
  };
}
