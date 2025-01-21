{
  lib,
  inputs,
  glibc_multi,
  uthash,
  symlinkJoin,
  writeTextFile,
  stdenv,
  config,
  libfabric,
  hwloc,
  openmpi,
  pkgconf,
  pkg-config,
  gtest,
  autoreconfHook,
  lttng-ust,
  valgrind,
  mpi,
  cudaPackages ? { },
  autoAddDriverRunpath,
  neuronSupport ? (!config.cudaSupport),
  cudaSupport ? (config.cudaSupport && !neuronSupport),
  enableTests ? cudaSupport,
  enableTracePrints ? true,
  enableLTTNGTracing ? false,
  enablePickyCompiler ? false,
  enableWerror ? false,
  enableNVTXTracing ? false,
  enableValgrind ? false,
  enableAwsTuning ? true,
}:

assert neuronSupport != cudaSupport;
assert !enableNVTXTracing || (enableNVTXTracing && cudaSupport);
let

  effectiveStdenv = if cudaSupport then cudaPackages.backendStdenv else stdenv;

  cudaBuildDepsJoined = symlinkJoin {
    name = "cuda-build-deps-joined";
    paths = lib.optionals (cudaSupport) (
      [
        (lib.getDev cudaPackages.cuda_nvcc)
        cudaPackages.cuda_cudart.include
      ]
      ++ (
        if effectiveStdenv.hostPlatform.isStatic then
          [
            (lib.getOutput "static" cudaPackages.cuda_cudart)
          ]
        else
          [
            (lib.getLib cudaPackages.cuda_cudart)
          ]
      )
    );
  };
  cleanSrc = import ./cleanSource.nix {
    inherit lib;
    self = builtins.getFlake ./.;
  };
in
effectiveStdenv.mkDerivation {
  __contentAddressed = true;
  name = "aws-ofi-nccl";
  pname = lib.concatStringsSep "" [
    "lib"
    (if neuronSupport then "nccom" else "nccl")
    "-net-ofi"
    (lib.optionalString enableAwsTuning "-aws")
  ];
  version = inputs.self.shortRev or inputs.self.dirtyShortRev;
  src = cleanSrc;

  nativeBuildInputs =
    [ autoreconfHook ]
    ++ lib.optionals cudaSupport [
      autoAddDriverRunpath
      cudaPackages.cuda_nvcc
    ]
    ++ lib.optionals enableTests [
      pkgconf
      pkg-config
    ];

  buildInputs =
    [
      libfabric
      hwloc
    ]
    ++ lib.optionals cudaSupport [
      cudaBuildDepsJoined
    ]
    ++ lib.optionals enableValgrind [
      valgrind
    ]
    ++ lib.optionals enableTests [
      mpi
      gtest
    ]
    ++ lib.optionals enableLTTNGTracing [
      lttng-ust
    ];

  configureFlags = [
    "--disable-silent-rules"
    # core deps
    (lib.withFeatureAs true "libfabric" (lib.getDev libfabric))
    (lib.withFeatureAs true "hwloc" (lib.getDev hwloc))
    #(lib.withFeatureAs true "nccl-headers" (cudaPackages.nccl.dev))

    # libs
    (lib.withFeatureAs enableTests "mpi" (lib.getDev mpi))
    (lib.enableFeature enableTests "tests")
    (lib.withFeatureAs enableLTTNGTracing "lttng" (lib.getDev lttng-ust))
    (lib.withFeatureAs enableValgrind "valgrind" (lib.getDev valgrind))

    # accelerator support
    (lib.enableFeature neuronSupport "neuron")
    (lib.withFeatureAs cudaSupport "cuda" cudaBuildDepsJoined)
    #(lib.withFeatureAs true "gtest" gtest.dev)
    (lib.withFeatureAs (enableNVTXTracing && cudaSupport) "nvtx" (lib.getDev cudaPackages.cuda_nvtx))
    (lib.enableFeature (!effectiveStdenv.hostPlatform.isStatic) "cudart-dynamic")

    # build configuration
    (lib.enableFeature enableAwsTuning "platform-aws")
    (lib.enableFeature enablePickyCompiler "picky-compiler")
    (lib.enableFeature enableWerror "werror")
    (lib.enableFeature enableTracePrints "trace")
  ];

  meta = with lib; {
    homepage = "https://github.com/aws/aws-ofi-nccl";
    license = licenses.asl20;
    broken = (cudaSupport && !config.cudaSupport);
    maintainers = with maintainers; [ sielicki ];
    platforms = [
      "x86_64-linux"
      "aarch64-linux"
    ];
  };

  hardeningEnable = [
    "format"
    "fortify3"
    "shadowstack"
    "pacret"
    "pic"
    "pie"
    "stackprotector"
    "stackclashprotection"
    "strictoverflow"
    "trivialautovarinit"
  ];
  enableParallelBuilding = true;
  separateDebugInfo = true;
  strictDeps = true;

  passthru.editorconfigFile = writeTextFile {
    name = "editorconfig-config";
    text = lib.generators.toINIWithGlobalSection { } {
      globalSection = {
        root = true;
      };
      sections = {
        "*" = {
          trim_trailing_whitespace = true;
          charset = "utf-8";
          end_of_line = "lf";
          insert_final_newline = true;
        };
        "*.am" = {
          indent_size = 8;
          indent_style = "tab";
        };
        "*.md" = {
          indent_size = 2;
          indent_style = "space";
        };
        "*.nix" = {
          tab_width = 4;
          indent_size = 2;
          indent_style = "space";
        };
        "*.{c|h|cc|hh|cu}" = {
          tab_width = 8;
          indent_size = 8;
          indent_style = "tab";
        };
      };
    };
  };

  passthru.make-clangd-file =
    { workdir, aws-ofi-nccl }:
    writeTextFile {
      name = "clangd-config";
      text = lib.generators.toYAML { } {
        CompileFlags = {
          CompilationDatabase = "${workdir}";
          Add = [
            "-Wall"
            "-Wextra"
            "-Wformat"
            "-xc++"
            "-std=c++20"
            "-isystem${glibc_multi.dev}/include/"
            "-isystem${hwloc.dev}/include/"
            "-isystem${uthash}/include/"
            "-isystem${gtest.dev}/include/"
            "-isystem${cudaPackages.cuda_cudart.dev}/include/"
            "-isystem${cudaPackages.cuda_nvtx.dev}/include/"
            "-isystem${libfabric.dev}/include/"
            "-isystem${openmpi.dev}/include/"
            "-isystem${workdir}/3rd-party/nccl/cuda/include/"
            "-isystem${workdir}/3rd-party/expected/tl/include/"
            "-I${workdir}/include/"
            "-I${aws-ofi-nccl.dev}/nix-support/generated-headers/include"
          ];
        };
        Diagnostics = {
          ClangTidy = {
            CheckOptions = {
              "cppcoreguidelines-avoid-magic-numbers.IgnoreTypeAliases" = true;
              "readability-magic-numbers.IgnoreTypeAliases" = true;
            };
          };
          Includes = {
            IgnoreHeader = [
              "hwloc.h"
              "config.hh"
            ];
          };
        };
      };
    };

  passthru.make-ccls-file =
    { workdir, aws-ofi-nccl }:
    writeTextFile {
      name = "ccls-config";
      text = ''
        %compile_commands.json
        clang++
        %cpp -std=c++20
        -isystem${glibc_multi.dev}/include/
        -isystem${hwloc.dev}/include/
        -isystem${uthash}/include/
        -isystem${gtest.dev}/include/
        -isystem${cudaPackages.cuda_cudart.dev}/include/
        -isystem${cudaPackages.cuda_nvtx.dev}/include/
        -isystem${libfabric.dev}/include/
        -isystem${openmpi.dev}/include/
        -isystem${workdir}/3rd-party/nccl/cuda/include/
        -isystem${workdir}/3rd-party/expected/tl/include/
        -I${aws-ofi-nccl.dev}/nix-support/generated-headers/include
        -I${workdir}/include/
        -I${workdir}/3rd-party/nccl/cuda/include/
      '';
    };

  passthru.clangFormatFile = writeTextFile {
    name = "clang-format-config";
    text = lib.generators.toYAML { } {
      ColumnLimit = 160;
      IncludeCategories = [
        {
          Priority = -40;
          Regex = "^([\"]config[.]h[h]?[\"])$";
          SortPriority = -40;
        }
        {
          Priority = 5;
          Regex = "^[<](rdma/|uthash/|nccl/|mpi|hwloc/|lttng/|valgrind/|cuda).*[.]h[>]$";
          SortPriority = 5;
        }
        {
          Priority = 10;
          Regex = "^([\"]nccl.*[.]h[h]?[\"])$";
          SortPriority = 10;
        }
      ];
      BasedOnStyle = "LLVM";
    };
  };

  outputs = [
    "dev"
    "out"
  ] ++ lib.optionals enableTests [ "bin" ];
  postInstall = ''
    find $out | grep -E \.la$ | xargs rm
    mkdir -p $dev/nix-support/generated-headers/include && cp include/config.hh $dev/nix-support/generated-headers/include/
    cp config.log $dev/nix-support/config.log
  '';

  doCheck = enableTests;
  checkPhase = ''
    set -euo pipefail
    for test in $(find tests/unit/ -type f -executable -print | xargs) ; do
      echo "======================================================================"
      echo "Running $test"
      ./$test
      test $? -eq 0 && (echo "✅ Passed" || (echo "❌ Failed!" && exit 1))
    done
    echo "All unit tests passed successfully."
    set +u
  '';
}
