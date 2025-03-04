#include <iostream>
#include "problem.h"
#include "printer.h"

int main(int argc, char* argv[]) {
	if (argc != 2) {
		std::cout << "Usage: " << argv[0] << " <number>" << std::endl;
		return 1;
	}

	int n = std::stoi(argv[1]);
	Problem p { n };

	std::this_thread::sleep_for(std::chrono::seconds(10));
	return 0;
}
