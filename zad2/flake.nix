{
  description = "Czat";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs?ref=nixos-unstable";
  };

  outputs = { self, nixpkgs }:
	let
		pkgs = nixpkgs.legacyPackages.x86_64-linux;
	in
	{
		packages.x86_64-linux = {
			zad2server = pkgs.callPackage ./server {};
			zad2client = pkgs.callPackage ./client {};
		};

		apps.x86_64-linux = {
			zad2server = {
				type = "app";
				program = "${self.packages.x86_64-linux.zad2server}/bin/server";
			};
			zad2client = {
				type = "app";
				program = "${self.packages.x86_64-linux.zad2client}/bin/client";
			};
		};
  };
}
