{ pkgs }:
pkgs.stdenv.mkDerivation {
	name = "zad1";
	version = "1.0";

	src = ./.;
}
