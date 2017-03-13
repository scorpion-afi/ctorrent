#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <semaphore.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>

#include <iostream>

const std::string semaphore_name = "/tdm-video-playback";

int main( void )
{
	pid_t child_pid;
	sem_t* sem;

	/* an initial value is 0 */
	sem = sem_open( semaphore_name.c_str(), O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR, 0 );
	if( sem == SEM_FAILED )
	{
		perror( "an attempt to create named semaphore failed" );
		return 1;
	}

	child_pid = fork();
	if( child_pid == -1 )
	{
		perror( "an attempt to fork failed" );
		return 1;
	}

	/* child related code */

	if( !child_pid )
	{
		sem = sem_open( semaphore_name.c_str(), O_RDWR );
		if( sem == SEM_FAILED )
		{
			perror( "[client] an attempt to open named semaphore failed" );
			return 1;
		}

		while( 1 )
		{
			int ret;

			/* wait until parent notices us */
			ret = sem_wait( sem );
			if( ret < 0 && errno == EINTR )
				continue;
			else
				break;
		}

		sem_close( sem );
		sem_unlink( semaphore_name.c_str() );

		return 0;
	}

	/* parent related code */

	/* we got to allow the client to open a semaphore :-) */
	sleep(1);

	sem_post( sem );
	sem_close( sem );
	sem_unlink( semaphore_name.c_str() );

	return 0;
}
