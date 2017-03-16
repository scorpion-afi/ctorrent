#include <iostream>
#include <stdio.h>
#include <cstring>

#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <sys/mman.h>
#include <sys/wait.h>

const std::string mq_name = "/my_queue";

int main( void )
{
	char* buf;
	pid_t child_pid;
	int page_size = getpagesize();

	/*buf = static_cast<char*>( mmap( nullptr, page_size, PROT_WRITE | PROT_READ,
			MAP_PRIVATE | MAP_ANONYMOUS, -1, 0 ) );*/
	buf = static_cast<char*>( mmap( nullptr, page_size, PROT_WRITE | PROT_READ,
			MAP_SHARED | MAP_ANONYMOUS, -1, 0 ) );
	if( buf == MAP_FAILED )
	{
		perror( "an attempt to mmap memory anonymously failed" );
		return 1;
	}

	child_pid = fork();
	if( child_pid < 0 )
	{
		perror( "an attempt to fork failed" );
		return 1;
	}

	/* client code */
	if( !child_pid )
	{
		sleep(1);

		std::cout << "child's reading..." << std::endl;
		std::cout << buf << std::endl;

		std::cout << "child's writing..." << std::endl;
		snprintf( buf, page_size, "by" );
		buf[2] = ',';	/* to see all string the parent has written */

		std::cout << buf << std::endl;

		return 0;
	}

	std::cout << "parent's writing..." << std::endl;
	snprintf( buf, page_size, "hi, cruel world."  );

	wait( nullptr );

	return 0;
}
