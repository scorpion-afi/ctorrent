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

	std::cout << "[server] a received request: (size: " << ret_size << ")\n ";

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

void client( void )
{
	sockaddr_un server_addr;
	int cl_socket;
	int ret;

	std::string request = "If you know Why it doesn't matter How.";
	std::vector<char> reply( 4096 );

	cl_socket = socket( AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0 );
	if( cl_socket < 0 )
	{
		int err = errno;
		std::array<char, 256> buf;

		std::cout << "[client] [error] socket(): " <<
				strerror_r( err, &buf.front(), buf.size() ) << std::endl;
		return;
	}

	std::memset( &server_addr, 0, sizeof(server_addr) );

	server_addr.sun_family = AF_UNIX;
	std::memcpy( server_addr.sun_path, socket_name.c_str(),
			std::min(socket_name.size() + 1, sizeof(server_addr.sun_path)) );

	/* try to connect socket to the remote socket address,
	 * send a connection request for remote socket which listens for connection requests */
	ret = connect( cl_socket, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr) );
	if( ret < 0 )
	{
		int err = errno;
		std::array<char, 256> buf;

		std::cout << "[client] [error] connect(): " <<
				strerror_r( err, &buf.front(), buf.size() ) << std::endl;

		close( cl_socket );
		return;
	}

	sockaddr_un cl_socket_addr;
	socklen_t addr_len = sizeof(cl_socket_addr);

	std::memset( &cl_socket_addr, 0, sizeof(cl_socket_addr) );
	getpeername( cl_socket, reinterpret_cast<sockaddr*>(&cl_socket_addr), &addr_len );

	if( addr_len > sizeof(cl_socket_addr) )
		std::cout << "[client] [warning] a peer socket address has been truncated.\n";
	else
		std::cout << "[client] connection to the server has been established (peer name: "
		    << cl_socket_addr.sun_path << "), send request..." << std::flush;

	ret = send( cl_socket, request.c_str(), request.size() + 1, MSG_NOSIGNAL );
	if( ret < 0 )
	{
		int err = errno;
		std::array<char, 256> buf;

		std::cout << "[client] [error] send(): " <<
				strerror_r( err, &buf.front(), buf.size() ) << std::endl;

		close( cl_socket );
		return;
	}

	std::cout << " success\n" "[client] wait for answer..." << std::flush;

	ret = recv( cl_socket, &reply.front(), reply.size(), 0 );
	if( ret < 0 )
	{
		int err = errno;
		std::array<char, 256> buf;

		std::cout << "[client] [error] recv(): " <<
				strerror_r( err, &buf.front(), buf.size() ) << std::endl;

		close( cl_socket );
		return;
	}

	std::cout << "[client] a received request: (size: " << ret << ")\n ";

	for( char ch : reply )
	 std::cout << ch;
	std::cout << std::endl;

	std::cout << "[client] the end.\n";

	close( cl_socket );
}


int main( void )
{
	pid_t child_pid;

	child_pid = fork();
	if( child_pid < 0 )
	{
		std::cout << "an attempt to fork() failed.\n";
		return 1;
	}

	/* parent process */
	if( child_pid )
		server();
	else
	{
		/* wait until server is ready */
		usleep( 20000 );

		client();
	}
}
