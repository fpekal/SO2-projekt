#pragma once
#include <mutex>
#include <iostream>

class Printer {
public:
	template<class T>
	Printer& operator<<(const T& t) {
		std::unique_lock<std::mutex> lock(mutex);
		std::cout << t;
		return *this;
	}

	std::mutex mutex;
};
