#pragma once
#include <vector>
#include <thread>
#include <mutex>
#include <memory>
#include <condition_variable>

#include "printer.h"

class Problem {
public:
	Problem(int phil_num) :
		phil_num{ phil_num }, fork_num{ phil_num }, arbiter{ phil_num - 1 } {

		for (int i = 0; i < fork_num; i++) {
			forks.push_back(std::make_shared<std::mutex>());
		}
		
		for (int i = 0; i < phil_num; i++) {
			philosophers.push_back(std::thread(&Problem::run_philosopher, this, i));
		}
	}
	
	~Problem() {
		for (int i = 0; i < phil_num; i++) {
			philosophers[i].join();
		}
	}

	int fork_num;
	int phil_num;

	std::vector<std::shared_ptr<std::mutex>> forks;
	std::vector<std::thread> philosophers;

	int arbiter;
	std::mutex arbiter_mutex;
	std::condition_variable arbiter_cv;

	Printer p;

private:
	static void run_philosopher(Problem* problem, int index) {
		auto left_fork = problem->forks[index];
		auto right_fork = problem->forks[(index + 1) % problem->fork_num];

		while (true) {
			problem->p << std::string{"Philosopher "} + std::to_string(index) + " tries to eat\n";
			std::unique_lock<std::mutex> guard(*left_fork, std::defer_lock);
			std::unique_lock<std::mutex> guard2(*right_fork, std::defer_lock);

			{
				std::unique_lock<std::mutex> arbiter_guard(problem->arbiter_mutex);
				problem->arbiter_cv.wait(arbiter_guard, [&] { return problem->arbiter > 0; });
				problem->arbiter--;
				problem->p << std::string{"Philosopher "} + std::to_string(index) + " is eating\n";
				arbiter_guard.unlock();

				guard.lock();
				guard2.lock();
				std::this_thread::sleep_for(std::chrono::milliseconds(std::rand() % 500 + 500));
				guard.unlock();
				guard2.unlock();

				problem->p << std::string{"Philosopher "} + std::to_string(index) + " is no longer eating\n";
				arbiter_guard.lock();
				problem->arbiter++;
				problem->arbiter_cv.notify_one();
				arbiter_guard.unlock();
				std::this_thread::sleep_for(std::chrono::milliseconds(std::rand() % 500 + 1000));
			}
		}
	}
};
