#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include <iostream>

int main( void )
{
	int fifo_write_fd;
	int ret;

	ret = mkfifo( "fifo", S_IRUSR | S_IWUSR );
	if( ret )
	{
		perror( "try to create fifo file" );
		return 1;
	}

	fifo_write_fd = open( "fifo", O_WRONLY );
	if( fifo_write_fd < 0 )
	{
		perror( "try to open fifo file" );
		return 1;
	}

	int size_to_write = sizeof("hi cruel world.");
	const char* str = "hi cruel world.";

	while( size_to_write )
	{
		ret = write( fifo_write_fd, str, size_to_write );
		if( ret < 0 )
		{
			if( errno == EINTR )
			{
				size_to_write -= ret;
				str += ret;
				continue;
			}

			perror( "try to write to  fifo file" );
			return 1;
		}

		size_to_write -= ret;
		str += ret;
	}

	close( fifo_write_fd );

	std::cout << "the end." << std::endl;
}
