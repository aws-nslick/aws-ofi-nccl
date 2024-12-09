{ inputs, ... }:
{
  imports = [ inputs.treefmt-nix.flakeModule ];
  perSystem =
    {
      pkgs,
      config,
      lib,
      ...
    }:
    {
      treefmt = {
        projectRootFile = "LICENSE";
        programs.nixfmt.enable = pkgs.lib.meta.availableOn pkgs.stdenv.buildPlatform pkgs.nixfmt-rfc-style.compiler;
        programs.nixfmt.package = pkgs.nixfmt-rfc-style;
        programs.shellcheck.enable = true;
        programs.ruff.check = true;
        programs.ruff.format = true;
        programs.clang-format.enable = true;
        programs.clang-format.package = config.devShells.default.passthru.clangPackages.clang-format;

        settings.formatter.ruff-check.priority = 1;
        settings.formatter.ruff-format.priority = 2;
        settings.formatter.shellcheck.options = [
          "-s"
          "bash"
        ];

        programs.mypy = {
          enable = pkgs.stdenv.buildPlatform.isLinux;
          package = pkgs.buildbot.python.pkgs.mypy;
          directories."." = {
            modules = [
              "buildbot_nix"
            ];
            extraPythonPackages = [
              (pkgs.python3.pkgs.toPythonModule pkgs.buildbot)
              pkgs.buildbot-worker
              pkgs.python3.pkgs.twisted
              pkgs.python3.pkgs.pydantic
              pkgs.python3.pkgs.zope-interface
            ];
          };
        };

        # the mypy module adds `./buildbot_nix/**/*.py` which does not appear to work
        # furthermore, saying `directories.""` will lead to `/buildbot_nix/**/*.py` which
        # is obviously incorrect...
        settings.formatter."mypy-." = lib.mkIf pkgs.stdenv.buildPlatform.isLinux {
          includes = [ "buildbot_nix/**/*.py" ];
        };

      };
    };
}
