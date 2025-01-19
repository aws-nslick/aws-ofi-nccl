{ inputs, ... }:
{

  imports = [
    inputs.git-hooks-nix.flakeModule
    inputs.treefmt-nix.flakeModule
  ];

  perSystem =
    {
      config,
      pkgs,
      lib,
      ...
    }:
    {

      treefmt = {
        settings.formatter.xmllint = {
          includes = [ "*.xml" ];
          command = "${pkgs.libxml2}/bin/xmllint";
        };
        programs.dos2unix.enable = true;
        programs.nixfmt.enable = true;
        programs.mdformat.enable = true;
        programs.ruff.format = true;
        programs.yamlfmt.enable = true;
        programs.clang-format.enable = true;
        programs.clang-format.package = pkgs.llvmPackages_git.clang-tools;
        programs.clang-format.excludes = [
          "3rd-party/**"
        ];
        programs.cmake-format.enable = true;
      };

      pre-commit.settings.hooks = {
        ast-grep = {
          enable = false;
          name = "ast-grep scan";
          extraPackages = [ pkgs.ast-grep ];
          entry = "ast-grep rules";
          files = "\\.(cc|hh|c|h)$";
          types = [ "text" ];
          language = "system";
          pass_filenames = false;
          #stages = ["pre-push"];
        };

        treefmt.enable = true;
        treefmt.package = config.treefmt.build.wrapper;
        treefmt.packageOverrides.treefmt = config.treefmt.build.wrapper;
        treefmt.settings.formatters = builtins.attrValues config.treefmt.build.programs;

        nixfmt-rfc-style.enable = true;

        # clang-tidy = {
        #   enable = true;
        #   types_or = lib.mkForce [
        #     "c"
        #     "c++"
        #   ];
        # };

        no-commit-to-branch.enable = true;
        no-commit-to-branch.settings.branch = [
          "master"
          "main"
        ];

        deadnix.enable = true;
        typos.enable = true;
        check-merge-conflicts.enable = true;
        check-added-large-files.enable = true;
        check-case-conflicts.enable = true;
        check-executables-have-shebangs.enable = true;
        checkmake.enable = false;
        check-shebang-scripts-are-executable.enable = true;
        check-symlinks.enable = true;
        check-vcs-permalinks.enable = true;
        check-xml.enable = true;
        cmake-format.enable = true;
        detect-aws-credentials.enable = true;
        detect-private-keys.enable = true;
        #editorconfig-checker.enable = true;
        end-of-file-fixer.enable = true;
        fix-byte-order-marker.enable = true;
        fix-encoding-pragma.enable = true;
        eclint.enable = true;
        hadolint.enable = true;
        headache.enable = true;
        headache.exclude_types = [ "markdown" ];
        headache.settings.header-file =
          (pkgs.writeTextFile {
            text = ''
              Copyright (c) 2018-2023 Amazon.com, Inc. or its affiliates. All rights reserved.
              Copyright (c) 2015-2018, NVIDIA CORPORATION. All rights reserved.
            '';
            name = "project-header-headache";
          }).outPath;

        mixed-line-endings.enable = true;
        tagref.enable = true;
        topiary.enable = true;
        trim-trailing-whitespace.enable = true;
        #trufflehog.enable = true;
        actionlint.enable = false;
        check-yaml.enable = true;
        ripsecrets.enable = true;
        forbid-new-submodules.enable = true;

        #mdl.enable = true;
        #shfmt.enable = true;
        #shellcheck.enable = true;

      };

      devShells =
        let
          shell = import ./shell.nix { inherit config; };
        in
        {
          default = pkgs.callPackage shell {
            aws-ofi-nccl = config.packages.default;
            llvmPackages = pkgs.llvmPackages_git;
          };
        };
    };

}
