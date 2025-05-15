GXXFLAGS=-Wall -Werror

analyze:
	GXXFLAGS="${GXXFLAGS}"
	cd zad2/server; GXXFLAGS="${GXXFLAGS}" make build
	cd zad2/client; GXXFLAGS="${GXXFLAGS}" make build


