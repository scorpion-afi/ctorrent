#include <iostream>
#include <stdio.h>
#include <cstring>

#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <mqueue.h>

#include <vector>
#include <thread>

const std::string mq_name = "/my_queue";

void read_thread_body( void )
{
	mqd_t mq;
	mq_attr _mq_attr;
	int ret;

	mq = mq_open( mq_name.c_str(), O_RDONLY );
	if( mq < 0 )
	{
		char buf[256];
		std::cout << "an attempt to open message queue failed: " <<
				strerror_r( errno, buf, sizeof(buf) ) << std::endl;

		return;
	}

	mq_getattr( mq, &_mq_attr );

	std::vector<char> message( _mq_attr.mq_msgsize );

	while( 1 )
	{
		ret = mq_receive( mq, message.data(), message.size(), nullptr );
		if( ret < 0 )
		{
			if( errno == EINTR )
				continue;

			char buf[256];
			std::cout << "an attempt to dequeue a message failed: " <<
					strerror_r( errno, buf, sizeof(buf) ) << std::endl;

			goto exit;
		}

		break;
	}

	std::cout << "a message has been received: \"" << message.data() <<
			"\", amount of data received (in bytes): " << ret << std::endl;

exit:
	mq_close( mq );
	mq_unlink( mq_name.c_str() );
}

int main( void )
{
	mqd_t mq;
	std::string message = "hi, cruel world.";

	mq = mq_open( mq_name.c_str(), O_CREAT | O_EXCL | O_WRONLY, S_IRUSR | S_IWUSR, nullptr );
	if( mq < 0 )
	{
		char buf[256];
		std::cout << "an attempt to create message queue failed: " <<
				strerror_r( errno, buf, sizeof(buf) ) << std::endl;

		return 1;
	}

	std::thread read_thread(read_thread_body);

	mq_send( mq, message.data(), message.size(), 0 );
	read_thread.join();

	mq_close( mq );
	mq_unlink( mq_name.c_str() );

	return 0;
}
