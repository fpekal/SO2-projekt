GXXFLAGS=-Wall -Werror

analyze:
	GXXFLAGS="${GXXFLAGS}"
	cd zad1; GXXFLAGS="${GXXFLAGS}" make build
	cd zad2/server; GXXFLAGS="${GXXFLAGS}" make build
	cd zad2/client; GXXFLAGS="${GXXFLAGS}" make build

build-docs:
	cd docs/zad2/client/doxygen; doxygen ../../../../zad2/client/Doxyfile; mv html/* .; rm -r html latex
	cd docs/zad2/server/doxygen; doxygen ../../../../zad2/server/Doxyfile; mv html/* .; rm -r html latex
