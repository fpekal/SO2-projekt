GXXFLAGS=-Wall -Werror

analyze:
	GXXFLAGS="${GXXFLAGS}"
	cd zad1; GXXFLAGS="${GXXFLAGS}" make build
	cd zad2/server; GXXFLAGS="${GXXFLAGS}" make build
	cd zad2/client; GXXFLAGS="${GXXFLAGS}" make build

build-docs:
	cd zad2/client; doxygen; mv html/* ../../docs/zad2/client/doxygen
	cd zad2/server; doxygen; mv html/* ../../docs/zad2/server/doxygen
