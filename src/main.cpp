#include <iostream>
#include <stdio.h>
#include <cstring>
#include <string>

#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <sys/socket.h>
#include <wait.h>

#include <boost/regex.hpp>



int main( void )
{
	std::string line;
	boost::regex pat( "^Subject: (Re: |Aw: )*(.*)" );

	while( std::cin )
	{
		std::getline( std::cin, line );
		boost::smatch matches;

		if( boost::regex_match( line, matches, pat ) )
			std::cout << matches[2] << std::endl;

	}
}
