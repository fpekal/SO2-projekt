#pragma once
#include <vector>
#include <thread>
#include <mutex>
#include <memory>
#include <condition_variable>
#include <atomic>

#include "printer.h"

class Problem {
public:
	Problem(int phil_num) :
		phil_num{ phil_num }, fork_num{ phil_num }, arbiter{ phil_num - 1 }, running{ true } {

		for (int i = 0; i < fork_num; i++) {
			forks.push_back(std::make_shared<std::mutex>());
		}
		
		for (int i = 0; i < phil_num; i++) {
			philosophers.push_back(std::thread(&Problem::run_philosopher, this, i));
		}
	}
	
	~Problem() {
		// Close all philosophers' threads
		running = false;

		for (int i = 0; i < phil_num; i++) {
			philosophers[i].join();
		}
	}

	// How many forks (for eating) there are
	int fork_num;

	// How many philosophers there are. This number should be equal to the number of forks
	int phil_num;

	// Fork can be used by only one philosopher at a time, so it's modeled as a mutex
	std::vector<std::shared_ptr<std::mutex>> forks;

	std::vector<std::thread> philosophers;


	// Arbiter is used to coordinate philosophers
	// It makes sure that only N-1 philosophers try to eat at the same time
	int arbiter;
	std::mutex arbiter_mutex;

	// Notifies philosophers that there is a new slot for new philosopher to eat
	std::condition_variable arbiter_cv;

	// Thread-Safe printing to console
	Printer p;

	// Is the problem still running
	std::atomic<bool> running;

private:
	static void run_philosopher(Problem* problem, int index) {
		// Get neighboring forks
		auto left_fork = problem->forks[index];
		auto right_fork = problem->forks[(index + 1) % problem->fork_num];

		while (problem->running) {
			problem->p << std::string{"Philosopher "} + std::to_string(index) + " tries to eat\n";
			// Create locks but don't lock them
			std::unique_lock<std::mutex> guard(*left_fork, std::defer_lock);
			std::unique_lock<std::mutex> guard2(*right_fork, std::defer_lock);

			{
				// Wait for arbitrator to allow philosopher to eat
				std::unique_lock<std::mutex> arbiter_guard(problem->arbiter_mutex);
				problem->arbiter_cv.wait(arbiter_guard, [&] { return problem->arbiter > 0; });
				problem->arbiter--;
				arbiter_guard.unlock();

				// Take forks and eat for some time
				problem->p << std::string{"Philosopher "} + std::to_string(index) + " is eating\n";
				guard.lock();
				guard2.lock();
				std::this_thread::sleep_for(std::chrono::milliseconds(std::rand() % 500 + 500));
				guard.unlock();
				guard2.unlock();
				problem->p << std::string{"Philosopher "} + std::to_string(index) + " is no longer eating\n";

				// Return slot and notify other philosophers that there is a new slot for them
				arbiter_guard.lock();
				problem->arbiter++;
				problem->arbiter_cv.notify_one();
				arbiter_guard.unlock();

				// Think
				std::this_thread::sleep_for(std::chrono::milliseconds(std::rand() % 500 + 1000));
			}
		}
	}
};
