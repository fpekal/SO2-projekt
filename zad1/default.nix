{ pkgs, gnumake, gcc }:
pkgs.stdenv.mkDerivation {
	name = "zad1";
	version = "0.1";

	src = ./.;
}
