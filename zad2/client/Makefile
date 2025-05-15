build:
	g++ ${GXXFLAGS} main.cpp -o client

install:
	mkdir -p ${out}/bin
	cp client ${out}/bin

debug:
	g++ ${GXXFLAGS} main.cpp -o client -ggdb
