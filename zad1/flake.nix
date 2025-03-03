{
  description = "Problem Jedzacych Filozofow";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs?ref=nixos-unstable";
  };

  outputs = { self, nixpkgs }: {
		apps =
		let
			pkgs = nixpkgs.legacyPackages.x86_64-linux;
			package = pkgs.callPackage (import ./default.nix) {};
		in {
			x86_64-linux = {
				default = {
					type = "app";
					program = "${package}/bin/zad1";
				};
			};
		};
  };
}
