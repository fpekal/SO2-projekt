build:
	g++ ${GXXFLAGS} main.cpp -o server

install:
	mkdir -p ${out}/bin
	cp server ${out}/bin

debug:
	g++ ${GXXFLAGS} main.cpp -o server -ggdb
