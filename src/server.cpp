#include <iostream>
#include <array>
#include <string>
#include <vector>

#include <cstring>

#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>


const std::string socket_name = "my_socket";


void server( void )
{
	sockaddr_un s_addr;
	int listen_socket = -1;
	int ret;

	sockaddr_un peer_addr;
	socklen_t addr_len;
	int peer_socket = -1;

	std::vector<char> request_data(4096);
	int ret_size;

	/* create a stream-based, reliable, bidirectional socket within AF_UNIX protocols family */
	listen_socket = socket( AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0 );
	if( listen_socket < 0 )
	{
		int err = errno;
		std::array<char, 256> buf;

		std::cout << "[server] [error] socket(): " <<
				strerror_r( err, &buf.front(), buf.size() ) << std::endl;
		return;
	}

	std::memset( &s_addr, 0, sizeof(s_addr) );

	s_addr.sun_family = AF_UNIX;
	std::memcpy( s_addr.sun_path, socket_name.c_str(),
			std::min(socket_name.size() + 1, sizeof(s_addr.sun_path)) );	/* string is char array */

	/* bind socket to some local socket address */
	ret = bind( listen_socket, reinterpret_cast<sockaddr*>(&s_addr), sizeof(s_addr) );
	if( ret < 0 )
	{
		int err = errno;
		std::array<char, 256> buf;

		std::cout << "[server] [error] bind(): " <<
				strerror_r( err, &buf.front(), buf.size() ) << std::endl;

		close( listen_socket );
		return;
	}

	std::cout << "[server] the socket has been bound to the local socket address.\n";

	/* mark socket as a passive socket so it can be used to accept incoming connection requests, via accept()
	 * socket is still bound to socket address I specified above */
	ret = listen( listen_socket, 2 );
	if( ret < 0 )
	{
		int err = errno;
		std::array<char, 256> buf;

		std::cout << "[server] [error] listen(): " <<
				strerror_r( err, &buf.front(), buf.size() ) << std::endl;
		goto finish;
	}

	addr_len = sizeof(peer_addr);
	std::memset( &peer_addr, 0, sizeof(peer_addr) );

	std::cout << "[server] wait for incoming connection requests..." << std::flush;

	/* accept incoming connection request from listen_socket's queue of pending connection requests
	 * return accepted socket that is in connected (to the remote peer) state */
	peer_socket = accept4( listen_socket, reinterpret_cast<sockaddr*>(&peer_addr), &addr_len, SOCK_CLOEXEC );
	if( peer_socket < 0 )
	{
		int err = errno;
		std::array<char, 256> buf;

		std::cout << "[server] [error] accept4(): " <<
				strerror_r( err, &buf.front(), buf.size() ) << std::endl;
		goto finish;
	}

	if( addr_len > sizeof(peer_addr) )
		std::cout << "[server] [warning] a peer socket address has been truncated.\n";

	std::cout << " success.\n" "[server] a new connection has been accepted, wait for request..." << std::flush;

	/* try to get up to request_data.size() bytes from the socket */
	ret_size = recv( peer_socket, &request_data.front(), request_data.size(), 0 );
	if( ret_size < 0 )
	{
		int err = errno;
		std::array<char, 256> buf;

		std::cout << "[server] [error] recv(): " <<
				strerror_r( err, &buf.front(), buf.size() ) << std::endl;
		goto finish;
	}

	std::cout << " success.\n" "[server] a received request: (size: " << ret_size << ")\n ";

	for( char ch : request_data )
	 std::cout << ch;
	std::cout << std::endl;

	std::cout << "[server] send a reply..." << std::flush;

	/* send data to the socket (this socket is connected to the remote socket) */
	ret_size = send( peer_socket, "A.K., Moscow.", sizeof("A.K., Moscow."), MSG_NOSIGNAL );
	if( ret_size < 0 )
	{
		int err = errno;
		std::array<char, 256> buf;

		std::cout << "[server] [error] send(): " <<
				strerror_r( err, &buf.front(), buf.size() ) << std::endl;
		goto finish;
	}

	std::cout << " success.\n";
	std::cout << "[server] the end.\n";

finish:
	close( peer_socket );
	close( listen_socket );
	unlink( socket_name.c_str() );
}

int main( void )
{
	server();
}
