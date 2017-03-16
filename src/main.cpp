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
#include <thread>


void read_thread_func( int socket_fd )
{
	msghdr msg = { 0, };
	iovec iov = { 0, };
	char str[512];
	int ret;

	iov.iov_base = str;
	iov.iov_len = sizeof( str );

	msg.msg_name = nullptr;
	msg.msg_namelen = 0;

	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

	msg.msg_control = nullptr;
	msg.msg_controllen = 0;

	ret = recvmsg( socket_fd, &msg, 0 );
	if( ret < 0 )
	{
		perror( "an attempt to receive a message failed" );
		goto exit;
	}

	if( ( msg.msg_flags & MSG_TRUNC ) || ( msg.msg_flags & MSG_ERRQUEUE ) )
	{
		std::cout << "msg.msg_flags: " << msg.msg_flags << std::endl;

		goto exit;
	}

	std::cout << "size of a message: " << ret << "(bytes), message: " << str << std::endl;

exit:
	close( socket_fd );
}

void send_message( int socket_fd )
{
	msghdr msg = { 0, };
	iovec iov = { 0, };
	std::string str = "hi cruel world.";
	int ret;

	iov.iov_base = const_cast<char*>( str.c_str() );
	iov.iov_len = str.size() + 1;	/* null-terminated character */

	msg.msg_name = nullptr;
	msg.msg_namelen = 0;

	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

	msg.msg_control = nullptr;
	msg.msg_controllen = 0;

	ret = sendmsg( socket_fd, &msg, MSG_NOSIGNAL );
	if( ret < 0 )
		perror( "an attempt to send a message failed" );

	close( socket_fd );
}

int main( void )
{
	int ipc_sockes[2];
	int ret;

	ret = socketpair( AF_UNIX, SOCK_SEQPACKET, 0, ipc_sockes );
	if( ret < 0 )
	{
		perror( "an attempt to create socketpair with a socket type SOCK_SEQPACKET failed" );
		return 1;
	}

	/* disable SIGIO delivering */
	for( int i : ipc_sockes )
	{
		int status_flags;

		status_flags = fcntl( ipc_sockes[i], F_GETFL );
		fcntl( ipc_sockes[i], F_SETFL, status_flags & ~O_ASYNC );
	}

	std::thread read_thread( read_thread_func, dup( ipc_sockes[0] ) );
	close( ipc_sockes[0] );

	send_message( dup( ipc_sockes[1] ) );
	close( ipc_sockes[1] );

	read_thread.join();

	std::cout << "the end." << std::endl;
}
