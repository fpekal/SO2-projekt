#include <iostream>
#include "problem.h"
#include "printer.h"

int main() {
	Problem p { 3 };

	std::this_thread::sleep_for(std::chrono::seconds(10));
	return 0;
}
