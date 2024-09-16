{
  accelerator = [ "-cuda" "-neuron" ];
  platform = [ "-aws" "" ];
  tracing = [ [ "" ] [ "-nvtx" ] [ "-lttng" ] [ "-nvtx" "-lttng" ] ];
  memory = [ "-valgrind" "" ];
  traceprints = [ "-trace" "" ];
  cpp = [ "-cpp" "" ];
  stdenv = [ (pkgs: pkgs.gcc7Stdenv) (pkgs: pkgs.gcc14Stdenv) (pkgs: pkgs.clang19Stdenv) ];
}
