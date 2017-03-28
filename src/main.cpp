#include <iostream>
#include <stdio.h>

#include <unistd.h>

#include <pthread.h>

int some_check = 1;
pthread_cond_t cond_var = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;


void* thread_func( void* )
{
	pthread_mutex_lock( &mtx );

	while( some_check )
	{
		pthread_cond_wait( &cond_var, &mtx );
		std::cout << pthread_self() << ", " <<some_check << std::endl;

		if( some_check ) some_check = 0;
	}

	pthread_mutex_unlock( &mtx );

	return nullptr;
}

int main( void )
{
	pthread_t threads[4] = { 0 };

	for( auto& thr : threads )
		pthread_create( &thr, nullptr, thread_func, nullptr );

	sleep( 2 );
	//pthread_cond_signal( &cond_var );  // cause ONE thread being moved from cond-queue to mtx-queue
	pthread_cond_broadcast( &cond_var ); // cause ALL threads being moved from cond-queue to mtx-queue

	for( auto& thr : threads )
		pthread_join( thr, nullptr );
}
