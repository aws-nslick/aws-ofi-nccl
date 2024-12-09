# Copyright (c) 2024, Amazon.com, Inc. or its affiliates. All rights reserved.
#
# See LICENSE for licensing information

{
  description = "aws-ofi-nccl development/build flake.";

  outputs =
    { self, flake-parts, ... }@inputs:
    let
      inherit (inputs.lib-aggregate) lib;
      systems = [
        "x86_64-linux"
        "aarch64-linux"
      ];
    in
    flake-parts.lib.mkFlake { inherit inputs; } (
      { withSystem, flake-parts-lib, ... }:
      {
        inherit systems;
        imports = [
          inputs.git-hooks-nix.flakeModule
          #inputs.git-hooks-nix.inputs.treefmt-nix.flakeModule
          flake-parts.flakeModules.easyOverlay
        ];
        flake = {

        };
        debug = true;
        perSystem =
          {
            system,
            config,
            final,
            pkgs,
            ...
          }:
          {
            _module.args.pkgs = import inputs.nixpkgs {
              inherit system;
              overlays = [
                (import ./nix/overlays/libfabric)
                inputs.cuda-packages.overlays.default
                inputs.self.overlays.default
              ];
              config = {
                cudaSupport = true;
                cudaForwardCompat = true;
                cudaCapabilities = [
                  "7.0"
                  "7.5"
                  "8.0"
                  "8.6"
                  "8.9"
                  "9.0"
                  "9.0a"
                  "10.0"
                ];
                allowBroken = true;
                allowUnfree = true;
              };
            };
            pre-commit.settings = import ./nix/checks.nix { inherit lib; };
            devShells.default = import ./nix/shell.nix {
              inherit
                pkgs
                config
                system
                inputs
                self
                ;
            };
            overlayAttrs = {
              cudaPackagesExtensions = (import ./nix/cudaPackagesExtensions { inherit lib config pkgs; });

              inherit (config.packages)
                libfabric
                openmpi
                ;
            };
            packages = rec {
              aws-ofi-nccl = (
                pkgs.callPackage ./nix/pkgs/aws-ofi-nccl {
                  inherit inputs self;
                }
              );
              ubuntu-test-runners = pkgs.callPackage ./nix/ubuntuTestRunners.nix {
                nccl-tests = pkgs.pkgsCuda.sm_90.cudaPackages.nccl-tests-aws;
              };
              default = aws-ofi-nccl;
              inherit (pkgs)
                libfabric
                openmpi
                ;
            };
          };
      }
    );

  inputs = {
    nixpkgs.url = "https://flakehub.com/f/NixOS/nixpkgs/0.1.*.tar.gz";
    nixpkgs-stable.url = "https://flakehub.com/f/NixOS/nixpkgs/0.2411.*.tar.gz";

    lib-aggregate.url = "github:nix-community/lib-aggregate";
    flake-parts.url = "https://flakehub.com/f/hercules-ci/flake-parts/0.1.350.tar.gz";

    cuda-packages.url = "github:ConnorBaker/cuda-packages";
    treefmt-nix.url = "github:numtide/treefmt-nix";
    git-hooks-nix.url = "https://flakehub.com/f/cachix/git-hooks.nix/0.1.958.tar.gz";

    flake-parts.inputs.nixpkgs-lib.follows = "lib-aggregate";
    treefmt-nix.inputs.nixpkgs.follows = "nixpkgs";
    git-hooks-nix.inputs.nixpkgs.follows = "nixpkgs";
    cuda-packages.inputs.nixpkgs.follows = "nixpkgs";
    cuda-packages.inputs.nixpkgs-24_11.follows = "nixpkgs-stable";
    cuda-packages.inputs.flake-parts.follows = "flake-parts";
    cuda-packages.inputs.git-hooks-nix.follows = "git-hooks-nix";
    cuda-packages.inputs.treefmt-nix.follows = "treefmt-nix";
  };

  nixConfig = {
    allowUnfree = true;
    cudaSupport = true;
    extra-substituters = [
      "https://numtide.cachix.org"
      "https://nix-community.cachix.org"
      "https://cuda-maintainers.cachix.org"
    ];
    extra-trusted-public-keys = [
      "numtide.cachix.org-1:2ps1kLBUWjxIneOy1Ik6cQjb41X0iXVXeHigGmycPPE="
      "nix-community.cachix.org-1:mB9FSh9qf2dCimDSUo8Zy7bkq5CX+/rkCWyvRCYg3Fs="
      "cuda-maintainers.cachix.org-1:0dq3bujKpuEPMCX6U4WylrUDZ9JyUG0VpVZa7CNfq5E="
    ];
  };
}
