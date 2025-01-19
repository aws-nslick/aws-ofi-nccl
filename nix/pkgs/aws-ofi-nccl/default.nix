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

  passthru.clangdFile = writeTextFile {
    name = "clangd-config";
    text = lib.generators.toYAML { } {
      CompileFlags = {
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
          "-isystem${cleanSrc}/3rd-party/nccl/cuda/include/"
          "-isystem${cleanSrc}/3rd-party/expected/tl/include/"
          #"-I${config.packages.default}/nix-support/generated-headers/include/"
          "-I${cleanSrc}/include/"
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
            "config.h"
          ];
        };
      };
    };
  };

  passthru.cclsFile = writeTextFile {
    name = "ccls-config";
    text = ''
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
      -isystem${cleanSrc}/3rd-party/nccl/cuda/include/
      -isystem${cleanSrc}/3rd-party/expected/tl/include/
      -I${cleanSrc}/include/
      -I${cleanSrc}/3rd-party/nccl/cuda/include/
    '';
  };

  passthru.clangFormatFile = writeTextFile {
    name = "clang-format-config";
    text = lib.generators.toYAML { } {
      AlignConsecutiveAssignments = false;
      AlignConsecutiveBitFields = {
        AcrossComments = true;
        AcrossEmptyLines = true;
        Enabled = true;
      };
      AlignConsecutiveDeclarations = false;
      AlignConsecutiveMacros = {
        AcrossComments = true;
        AcrossEmptyLines = true;
        Enabled = true;
      };
      AlignConsecutiveShortCaseStatements = {
        AcrossComments = true;
        AcrossEmptyLines = true;
        AlignCaseColons = false;
        Enabled = true;
      };
      AlignOperands = "Align";
      AlignTrailingComments = {
        Kind = "Always";
        OverEmptyLines = 0;
      };
      AllowShortCompoundRequirementOnASingleLine = true;
      KeepEmptyLines = {
        AtEndOfFile = false;
        AtStartOfBlock = false;
        AtStartOfFile = false;
      };
      AllowAllArgumentsOnNextLine = false;
      AllowShortFunctionsOnASingleLine = "None";
      AllowShortIfStatementsOnASingleLine = false;
      AllowShortLoopsOnASingleLine = false;
      BasedOnStyle = "Google";
      BinPackArguments = false;
      BinPackParameters = false;
      BracedInitializerIndentWidth = 8;
      BreakBeforeBraces = "Linux";
      ColumnLimit = 130;
      ContinuationIndentWidth = 8;
      IncludeBlocks = "Regroup";
      IncludeCategories = [
        {
          Priority = -40;
          Regex = "^([\"]config[.]h[\"])$";
          SortPriority = -40;
        }
        {
          Priority = 5;
          Regex = "^[<](rdma/|uthash/|nccl/|mpi|hwloc/|lttng/|valgrind/|cuda).*[.]h[>]$";
          SortPriority = 5;
        }
        {
          Priority = 10;
          Regex = "^([\"]nccl.*[.]h[\"])$";
          SortPriority = 10;
        }
      ];
      IndentCaseLabels = false;
      IndentWidth = 8;
      InsertBraces = true;
      InsertNewlineAtEOF = true;
      LineEnding = "LF";
      MaxEmptyLinesToKeep = 2;
      PointerAlignment = "Right";
      ReferenceAlignment = "Right";
      ReflowComments = true;
      RemoveParentheses = "MultipleParentheses";
      SortIncludes = "CaseSensitive";
      SpacesBeforeTrailingComments = 2;
      TabWidth = 8;
      BreakBinaryOperations = "RespectPrecedence";
      AllowShortCaseExpressionOnASingleLine = true;
      UseTab = "ForContinuationAndIndentation";
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
