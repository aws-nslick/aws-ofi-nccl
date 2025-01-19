{ config }:
{
  aws-ofi-nccl,
  mkShell,
  llvmPackages,
  include-what-you-use,
  clang-uml,
  gdb,
  ccache,
  cppcheck,
  ccls,
  universal-ctags,
  ast-grep,
  fh,
}:
mkShell {
  inputsFrom = [ aws-ofi-nccl ];
  packages = [
    llvmPackages.libclang.python
    llvmPackages.clang-tools
    #llvmPackages.lldb
    include-what-you-use
    clang-uml
    gdb
    ccache
    cppcheck
    ccls
    universal-ctags
    ast-grep
    fh
  ];
  buildInputs = [

  ] ++ config.pre-commit.settings.enabledPackages;
  shellHook =
    let
      source-dir = builtins.getEnv "PWD";
      inherit (aws-ofi-nccl.passthru)
        clangdFile
        cclsFile
        editorconfigFile
        clangFormatFile
        ;
    in
    ''
      rm -f ${source-dir}/.clangd && ln -s ${clangdFile} ${source-dir}/.clangd
      rm -f ${source-dir}/.ccls && ln -s ${cclsFile} ${source-dir}/.ccls
      rm -f ${source-dir}/.editorconfig && ln -s ${editorconfigFile} ${source-dir}/.editorconfig
      rm -f ${source-dir}/.clang-format && ln -s ${clangFormatFile} ${source-dir}/.clang-format
      ${config.pre-commit.installationScript}
    '';
}
