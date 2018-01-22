/*
 * ctorrent_server.cpp
 *
 *  Created on: Jan 25, 2018
 *      Author: sergs (ivan0ivanov0@mail.ru)
 */

#include "config.h"

#include <functional>
#include <sstream>
#include <utility>
#include <cstdint>

#include <sys/types.h>
#include <sys/socket.h>

#ifdef X86_64_BUILD
#include <ifaddrs.h>
#endif

#include <netdb.h>
#include <arpa/inet.h>

#include <boost/log/trivial.hpp>

#include "executor_thread.h"
#include "send_thread.h"
#include "ctorrent_server.h"

#define REUSE_PORT


ctorrent_server::ctorrent_server() : tasks_queue(max_queue_size), results_queue(max_queue_size)
{

  std::vector<int> vi;

  std::list<int> li;

  vi.insert( vi.begin(), li.begin(), li.end() );

  sockaddr_in s_addr;
  int listen_socket;
  int ret;

  /* look https://github.com/morristech/android-ifaddrs for ifaddrs.h implementation for Android */
#ifdef ANDROID_BUILD
  throw std::string( "ifaddrs.h functionality currently isn't implemented." );
#endif

  interface_ip = get_interface_ipv4_address( interface_name );
  interface_ip_text = convert_ipv4_from_binary_to_text( interface_ip );

  BOOST_LOG_TRIVIAL( info ) << "ctorrent_server: ip address for the " << interface_name << " interface: " << interface_ip_text->c_str();

  /* create a stream-based, reliable, bidirectional socket within AF_INET protocols family */
  listen_socket = socket( AF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0 );
  if( listen_socket < 0 )
  {
    int err = errno;
    std::array<char, 256> buf;

    BOOST_LOG_TRIVIAL( error ) << "[server] [error] socket(): " <<
        strerror_r( err, &buf.front(), buf.size() ) << std::endl;

    throw std::string( "an error while trying to create a socket." );
  }

#ifdef REUSE_PORT
  int reuse = 1;
  ret = setsockopt( listen_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse) );
  if( ret < 0 )
  {
    int err = errno;
    std::array<char, 256> buf;

    BOOST_LOG_TRIVIAL( error ) << "[server] [error] setsockopt(): " <<
        strerror_r( err, &buf.front(), buf.size() ) << std::endl;

    throw std::string( "an error while trying to prepare a socket to share the same port with other sockets." );
  }
#endif

  std::memset( &s_addr, 0, sizeof(s_addr) );

  s_addr.sin_family = AF_INET;
  s_addr.sin_addr.s_addr = interface_ip.s_addr;
  s_addr.sin_port = htons( port_number );

  /* bind socket to some local socket address */
  ret = bind( listen_socket, reinterpret_cast<sockaddr*>(&s_addr), sizeof(s_addr) );
  if( ret < 0 )
  {
    int err = errno;
    std::array<char, 256> buf;

    BOOST_LOG_TRIVIAL( error ) << "[server] [error] bind(): " <<
        strerror_r( err, &buf.front(), buf.size() ) << std::endl;

    close( listen_socket );
    throw std::string( "an error while trying to bind address." );
  }

  BOOST_LOG_TRIVIAL( info ) << "ctorrent_server: the socket (fd: " << listen_socket << ") has been bound to the local socket address: "
      << interface_ip_text->c_str() << ":" << port_number;

  /* mark socket as a passive socket so it can be used to accept incoming connection requests, via accept()
   * socket is still bound to socket address I specified above */
  ret = listen( listen_socket, 2 );
  if( ret < 0 )
  {
    int err = errno;
    std::array<char, 256> buf;

    BOOST_LOG_TRIVIAL( error ) << "[server] [error] listen(): " <<
        strerror_r( err, &buf.front(), buf.size() ) << std::endl;
    throw std::string( "an error while trying to mark a socket as a listening socket." );
  }

  BOOST_LOG_TRIVIAL( debug ) << "ctorrent_server: the socket (fd: " << listen_socket << ") has been marked as passive (listening) socket";

  /* register 'listen_socket' in the epoll instance to be able to accept client's requests to connect */
  epoll.add_event_source( listen_socket, epoll_event_loop::event_type::EPOLL_IN,
                          std::bind(&ctorrent_server::new_client_handler, this, std::placeholders::_1, std::placeholders::_2),
                          nullptr, "listen socket" );

  /* it's epoll responsibility to manage fds, so we can close listen_socket here */
  close( listen_socket );

  start_threads();

  BOOST_LOG_TRIVIAL( info ) << "ctorrent_server: wait for incoming connection requests";
}

ctorrent_server::~ctorrent_server()
{
  /*TODO: destroy all clients */
}

int ctorrent_server::get_fd() const
{
  /* ctorrent_server doesn't own an epoll's fd, so there's no reason to make a duplicate */
  return epoll.get_fd();
}

void ctorrent_server::handle_events()
{
  epoll.handle_events();
}

/* get an IPv4 address for an interface 'if_name' for the host this server gets launched at */
in_addr ctorrent_server::get_interface_ipv4_address( const std::string& if_name )
{
  in_addr host_ip = {0};
  //host_ip.s_addr = 0;

#ifdef X86_64_BUILD
  struct ifaddrs *if_address_list, *ifa;
  int ret;

  std::memset( &host_ip, 0, sizeof host_ip );

  ret = getifaddrs( &if_address_list );
  if( ret < 0 )
  {
    int err = errno;
    std::array<char, 256> buf;

    BOOST_LOG_TRIVIAL( error ) << "[server] [error] getifaddrs(): " <<
        strerror_r( err, &buf.front(), buf.size() ) << std::endl;

    throw std::string( "an error while trying to get interfaces addresses." );
  }

  BOOST_LOG_TRIVIAL( debug ) << "ctorrent_server: interfaces:";

  for( ifa = if_address_list; ifa != NULL; ifa = ifa->ifa_next )
  {
    if( !ifa->ifa_addr )
      continue;

    BOOST_LOG_TRIVIAL( debug ) << " " << ifa->ifa_name << ", family: " << ifa->ifa_addr->sa_family;

    if( ifa->ifa_addr->sa_family == AF_INET && !strncmp( ifa->ifa_name, if_name.c_str(), if_name.size() ) )
    {
      char host[NI_MAXHOST];

      ret = getnameinfo( ifa->ifa_addr, sizeof(struct sockaddr_in), host, sizeof(host),
                   nullptr, 0, NI_NUMERICHOST );
      if( ret < 0 )
      {
        BOOST_LOG_TRIVIAL( error ) << "[server] [error] getnameinfo(): " <<  gai_strerror( ret ) << std::endl;

        freeifaddrs( if_address_list );
        throw std::string( "an error while trying to get hostname from socket address." );
      }

      ret = inet_pton( AF_INET, host, &host_ip );
      if( ret != 1 )
      {
        BOOST_LOG_TRIVIAL( error ) << "[server] [error] inet_pton(): " << ret << std::endl;

        freeifaddrs( if_address_list );
        throw std::string( "an error while trying to get binary representation of ipv4 address." );
      }

      break;
    }
  }

  freeifaddrs( if_address_list );
#endif

  return host_ip;
}

void ctorrent_server::new_client_handler( int listen_socket, void* data )
{
  sockaddr_in peer_addr;
  socklen_t addr_len;
  int peer_socket;
  std::stringstream tmp;

  addr_len = sizeof(peer_addr);
  std::memset( &peer_addr, 0, sizeof(peer_addr) );

  /* accept incoming connection request from listen_socket's queue of pending connection requests
   * return accepted socket that is in connected (to the remote peer) state */
  peer_socket = accept4( listen_socket, reinterpret_cast<sockaddr*>(&peer_addr), &addr_len, SOCK_CLOEXEC );
  if( peer_socket < 0 )
  {
    int err = errno;
    std::array<char, 256> buf;

    BOOST_LOG_TRIVIAL( error ) << "[server] [error] accept4(): " <<
        strerror_r( err, &buf.front(), buf.size() ) << std::endl;
    throw std::string( "an error while trying to accept client request." );
  }

  tmp << "remote client (" << convert_ipv4_from_binary_to_text( peer_addr.sin_addr )->c_str() << ":"
      << ntohs( peer_addr.sin_port ) <<  ")";

  if( addr_len > sizeof(peer_addr) )
    BOOST_LOG_TRIVIAL( info ) << "[server] [warning] a peer socket address has been truncated.\n";
  else
    BOOST_LOG_TRIVIAL( info ) << "ctorrent_server: a new connection has been accepted (fd: " << peer_socket <<
      ") -- " << tmp.str();

  auto cl = std::make_shared<remote_client>( peer_socket, tasks_queue, tmp.str() );
  cl->set_self_reference( std::weak_ptr<remote_client>(cl) );

  auto tuple = std::make_shared<destroy_cl_tuple>( std::shared_ptr<remote_client>(cl), 0, 0 );
  uint64_t in_idx, rdhup_idx;

  /* register an event handler to process a 'some data to read presents on a socket' event */
  in_idx = epoll.add_event_source( peer_socket, epoll_event_loop::event_type::EPOLL_IN,
                                   std::bind( [] (int fd, void* data, std::shared_ptr<remote_client> ptr) { ptr->consume_data_from_remote(); },
                                              std::placeholders::_1, std::placeholders::_2, std::shared_ptr<remote_client>(cl) ),
                                   nullptr, "peer socket" );

  /* register an event handler to process a 'remote side closed a connection' event */
  rdhup_idx = epoll.add_event_source( peer_socket, epoll_event_loop::event_type::EPOLL_RDHUP,
                          std::bind(&ctorrent_server::destroy_client_handler, this, std::placeholders::_1, std::placeholders::_2,
                                    std::shared_ptr<destroy_cl_tuple>(tuple)), nullptr, "peer socket" );
  client_list.push_back( std::move(cl) );

  std::get<1>(*tuple) = in_idx;
  std::get<2>(*tuple) = rdhup_idx;

  /* it's epoll responsibility to manage fds, so we can close peer_socket here */
  close( peer_socket );
}

void ctorrent_server::destroy_client_handler( int listen_socket, void* data, std::shared_ptr<destroy_cl_tuple> tuple )
{
  std::shared_ptr<remote_client> cl;
  uint64_t in_idx, rdhup_idx;

  std::tie( cl, in_idx, rdhup_idx ) = *tuple;

  BOOST_LOG_TRIVIAL( info ) << "ctorrent_server: close a client connection...";

  epoll.remove_event_source( in_idx );
  epoll.remove_event_source( rdhup_idx );

  /* TODO: does an existence checking have to be performed? */
  client_list.remove( cl );
}

void ctorrent_server::start_threads()
{
  /* calculate amount of threads we can concurrently launch;
   * the result has to be not less then min_executor_threads_amount;
   * if it possible 3 threads are reserved for a main thread, a send thread and a thread for some OS background tasks */
  std::size_t threads_amount = (std::thread::hardware_concurrency() - 3) >= min_executor_threads_amount ?
      std::thread::hardware_concurrency() - 3 : min_executor_threads_amount;

  threads_amount = 1;
  BOOST_LOG_TRIVIAL( info ) << "ctorrent_server: launch " << threads_amount << " executor threads";

  /* launch and detach executors threads */
  for( std::size_t i = 0; i < threads_amount; ++i )
  {
    std::thread thread( executor_thread( tasks_queue, results_queue ) );
    thread.detach();

    executor_thread_pool.push_back( std::move(thread) );
  }

  BOOST_LOG_TRIVIAL( info ) << "ctorrent_server: launch a sending thread";
  sending_thread = std::thread( send_thread( results_queue ) );

}
