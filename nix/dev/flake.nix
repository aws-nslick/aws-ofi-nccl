{
  description = "Private inputs for development purposes. These are used by the top level flake in the `dev` partition, but do not appear in consumers' lock files.";
  inputs = {
    nixpkgs.url = "https://flakehub.com/f/NixOS/nixpkgs/0.1.*.tar.gz";
    treefmt-nix.url = "github:numtide/treefmt-nix";
    git-hooks-nix.url = "https://flakehub.com/f/cachix/git-hooks.nix/0.1.958.tar.gz";

    treefmt-nix.inputs.nixpkgs.follows = "nixpkgs";
    git-hooks-nix.inputs.nixpkgs.follows = "nixpkgs";
    git-hooks-nix.inputs.treefmt-nix.follows = "treefmt-nix";
  };

  # This flake is only used for its inputs.
  outputs = { ... }: { };
}
