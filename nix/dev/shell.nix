{ config }:
{
  lib,
  aws-ofi-nccl,
  mkShell,
  llvmPackages,
  include-what-you-use,
  writeShellScriptBin,
  compdb,
  compiledb,
  jq,
  clang-uml,
  gdb,
  ccache,
  cppcheck,
  ccls,
  universal-ctags,
  ast-grep,
  fh,
}:
let
  stdenv = llvmPackages.libcxxStdenv;
  workdir = builtins.getEnv "PWD";
  inherit (aws-ofi-nccl.passthru)
    make-ccls-file
    make-clangd-file
    editorconfigFile
    clangFormatFile
    ;
  clangdFile = make-clangd-file { inherit workdir aws-ofi-nccl; };
  cclsFile = make-ccls-file { inherit workdir aws-ofi-nccl; };
  generate-compile-commands = writeShellScriptBin "generate-compile-commands" ''
    ${compiledb}/bin/compiledb --parse \
      <(nix log .#default | sed 's,/build/source,${workdir},g') \
      -o - | ${jq}/bin/jq \
        --argjson extraflags "$(${jq}/bin/jq '["-isystem${lib.getDev llvmPackages.libcxx}/include/c++/v1", "-isystem${lib.getLib llvmPackages.libclang}/lib/clang/${lib.versions.major llvmPackages.clang.version}/include/", "-isystem${lib.getDev stdenv.cc.libc}/include", .CompileFlags.Add[-1]]' ${clangdFile})" \
        '.[].arguments |= (.[0:4] |= . + $extraflags)'
  '';
in
mkShell.override { inherit stdenv; } {
  inputsFrom = [ aws-ofi-nccl ];
  packages = [
    llvmPackages.libclang
    llvmPackages.libclang.lib
    llvmPackages.libclang.python
    llvmPackages.clang-tools
    llvmPackages.libcxx
    stdenv.cc.libc
    llvmPackages.clang
    #llvmPackages.lldb
    include-what-you-use
    clang-uml
    gdb
    ccache
    cppcheck
    ccls
    universal-ctags
    ast-grep
    compiledb
    compdb
    generate-compile-commands
    fh
  ];
  buildInputs = [

  ] ++ config.pre-commit.settings.enabledPackages;
  shellHook = ''
    rm -f ${workdir}/.clangd && ln -s ${clangdFile} ${workdir}/.clangd
    rm -f ${workdir}/.ccls && ln -s ${cclsFile} ${workdir}/.ccls
    rm -f ${workdir}/.editorconfig && ln -s ${editorconfigFile} ${workdir}/.editorconfig
    rm -f ${workdir}/.clang-format && ln -s ${clangFormatFile} ${workdir}/.clang-format
    ${config.pre-commit.installationScript}
  '';
}
