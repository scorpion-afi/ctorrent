#include <iostream>

#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>

int some_check = 1;
std::condition_variable cond_var;
std::mutex mtx;


void thread_func( void )
{
	std::unique_lock<std::mutex> lock( mtx );

	cond_var.wait( lock,
			[] { std::cout << "some_check: " << some_check << std::endl; return some_check == 0; } );

	std::cout << "finishing...\n";
}

int main( void )
{
	std::thread threads[4] = { 	std::thread(thread_func),
								std::thread(thread_func),
								std::thread(thread_func),
								std::thread(thread_func) };

	std::this_thread::__sleep_for( std::chrono::seconds(2), std::chrono::nanoseconds(0) );

	/* cause ALL threads being moved from cond-queue to mtx-queue */
	std::cout << "fail notifying...\n";
	cond_var.notify_all();

	std::this_thread::__sleep_for( std::chrono::seconds(2), std::chrono::nanoseconds(0) );

	{
		std::lock_guard<std::mutex> lk( mtx );
		some_check = 0;
	}

	std::cout << "real notifying...\n";
	cond_var.notify_all();

	for( auto& thr : threads )
		thr.join();

	std::cout << "the end.\n";
}
