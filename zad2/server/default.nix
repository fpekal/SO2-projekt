{ pkgs, stdenv, ... }:
stdenv.mkDerivation (finalAttrs: {
  pname = "server";
  version = "1.0";

  src = ./.;
})
