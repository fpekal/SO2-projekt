{
  description = "Systemy Operacyjne 2 projekt";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs?ref=nixos-unstable";

		zad1 = {
			url = "path:./zad1";
			inputs.nixpkgs.follows = "nixpkgs";
		};
  };

  outputs = { self, nixpkgs, zad1 }: {
		packages.x86_64-linux.zad1 = zad1.packages.x86_64-linux.zad1;
		apps.x86_64-linux.zad1 = zad1.apps.x86_64-linux.zad1;
  };
}
