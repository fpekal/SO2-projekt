{
  description = "Problem Jedzacych Filozofow";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs?ref=nixos-unstable";
  };

  outputs = { self, nixpkgs }:
	let
		pkgs = nixpkgs.legacyPackages.x86_64-linux;
	in
	{
		packages.x86_64-linux = {
			zad1 = pkgs.callPackage ./default.nix {};
			default = self.packages.x86_64-linux.zad1;
		};

		apps.x86_64-linux = {
			zad1 = {
				type = "app";
				program = "${self.packages.x86_64-linux.zad1}/bin/zad1";
			};
			default = self.apps.x86_64-linux.zad1;
		};
  };
}
