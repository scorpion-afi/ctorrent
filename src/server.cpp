
#include <config.h>

#include <iostream>
#include <array>
#include <string>
#include <vector>

#include <cstring>

#include <unistd.h>

#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>	/* IPv4 */


const uint32_t host_ip = 0x7f000001;	/* 127.0.0.1 (loopback) in host byte order (little endian) */
const uint16_t port_number = 4096 + 1;	/* why this one, why not? */


void server( void )
{
	sockaddr_in s_addr;
	int listen_socket = -1;
	int ret;

	sockaddr_in peer_addr;
	socklen_t addr_len;
	int peer_socket = -1;

	std::vector<char> request_data(4096);
	int ret_size;

	/* create a stream-based, reliable, bidirectional socket within AF_INET protocols family */
#ifdef ANDROID_BUILD
	listen_socket = socket( AF_INET, SOCK_STREAM, 0 );
#else
	listen_socket = socket( AF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0 );
#endif
	if( listen_socket < 0 )
	{
		int err = errno;
		std::array<char, 256> buf;

		std::cout << "[server] [error] socket(): " <<
				strerror_r( err, &buf.front(), buf.size() ) << std::endl;
		return;
	}

	std::memset( &s_addr, 0, sizeof(s_addr) );

	s_addr.sin_family = AF_INET;
	s_addr.sin_addr.s_addr = htonl( host_ip );
	s_addr.sin_port = htons( port_number );

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

	std::cout << "[server] the socket has been bound to the local socket address (0x" << std::hex
			<< host_ip << std::dec << ":" << port_number << ").\n";

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

	std::cout << "[server] the socket has been marked as passive (listening) socket.\n";
	std::cout << "[server] wait for incoming connection requests..." << std::flush;

	addr_len = sizeof(peer_addr);
	std::memset( &peer_addr, 0, sizeof(peer_addr) );

	/* accept incoming connection request from listen_socket's queue of pending connection requests
	 * return accepted socket that is in connected (to the remote peer) state */
#ifdef ANDROID_BUILD
	peer_socket = accept( listen_socket, reinterpret_cast<sockaddr*>(&peer_addr), &addr_len );
#else
	peer_socket = accept4( listen_socket, reinterpret_cast<sockaddr*>(&peer_addr), &addr_len, SOCK_CLOEXEC );
#endif
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

	std::cout << " success.\n" "[server] a new connection has been accepted (client address: 0x"
			<< std::hex << ntohl( peer_addr.sin_addr.s_addr ) << std::dec << ":"
			<< ntohs( peer_addr.sin_port ) << "), wait for request..." << std::flush;

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
	ret_size = send( peer_socket, "\"A.K., Moscow.\"", sizeof("\"A.K., Moscow.\""), MSG_NOSIGNAL );
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
}

int main( void )
{
	server();
}
