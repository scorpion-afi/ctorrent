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

	std::cout << " success.\n" "[client] wait for answer..." << std::flush;

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

	std::cout << " success.\n" "[client] a received request: (size: " << ret << ")\n ";

	for( char ch : reply )
	 std::cout << ch;
	std::cout << std::endl;

	std::cout << "[client] the end.\n";

	close( cl_socket );
}

int main( void )
{
	client();
}
