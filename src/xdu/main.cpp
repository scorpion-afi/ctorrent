
#include <config.h>

#include <iostream>
#include <array>
#include <fstream>

#include <cstring>
#include <unistd.h>
#include <errno.h>

#include <boost/log/trivial.hpp>


int main( void )
{
	std::ofstream file( "/home/sergs/xdu.log" );
	int res;

	std::cout.rdbuf( file.rdbuf() );

	std::cout << "I'm process with pid: " << getpid() << std::endl;
	std::cout << "invoke daemon...\n";

	BOOST_LOG_TRIVIAL( debug ) << "wassup cruel world.";
	
	/* make fork syscall and terminate parent process inside */
	res = daemon( 0, 0 );
	if( res < 0 )
	{
		int err = errno;
		std::array<char, 256> buf;

		std::cout << "[server] [error] socket(): " <<
				strerror_r( err, &buf.front(), buf.size() ) << std::endl;
		return 1;
	}

	/* we're in a background process, gratz */

	std::cout << "I'm process with pid: " << getpid() << std::endl;
}
