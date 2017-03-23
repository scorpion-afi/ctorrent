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

#include <boost/lambda/lambda.hpp>
#include <iterator>
#include <algorithm>


int main( void )
{
	using namespace boost::lambda;
	typedef std::istream_iterator<int> in;

	std::for_each( in(std::cin), in(), std::cout << (_1 * 3) << " " );

	return 0;
}
