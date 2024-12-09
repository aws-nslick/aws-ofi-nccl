{
  lib,
  config,
  pkgs,
  ...
}:
let
  makeOverlay = f: (import ./${f} { inherit lib config pkgs; });

  files = (builtins.readDir ./.);
  filterSelf = (builtins.removeAttrs files [ "default.nix" ]);
  namesOnly = builtins.attrNames filterSelf;
in
map makeOverlay namesOnly
