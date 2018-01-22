/*
 * ctorrent_client.cpp
 *
 *  Created on: Jan 25, 2018
 *      Author: sergs (ivan0ivanov0@mail.ru)
 */

#include "config.h"

#include <array>
#include <fstream>
#include <sstream>
#include <chrono>
#include <thread>
#include <cmath>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h> /* IPv4 */

#include <boost/log/trivial.hpp>

#include "ctorrent_client.h"


ctorrent_client::ctorrent_client()
{
  sockaddr_in server_addr;
  int remote_socket;
  int ret;

  std::list<in_addr> servers_list = get_servers_list();

  BOOST_LOG_TRIVIAL( debug ) << "ctorrent_client: servers list:";
  for( auto& server : servers_list )
    BOOST_LOG_TRIVIAL( debug ) << " " << convert_ipv4_from_binary_to_text( server )->c_str();

  for( auto& server : servers_list )
  {
    remote_socket = socket( AF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0 );
    if( remote_socket < 0 )
    {
      int err = errno;
      std::array<char, 256> buf;

      BOOST_LOG_TRIVIAL( error ) << "ctorrent_client: socket(): " <<
          strerror_r( err, &buf.front(), buf.size() );
      throw std::string( "an error while trying to create a AF_INET socket." );
    }

    std::memset( &server_addr, 0, sizeof(server_addr) );

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = server.s_addr;
    server_addr.sin_port = htons( port_number );

    /* try to connect socket to the remote socket address,
     * send a connection request for remote socket which listens for connection requests */
    ret = connect( remote_socket, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr) );
    if( ret < 0 )
    {
      int err = errno;
      std::array<char, 256> buf;

      BOOST_LOG_TRIVIAL( error ) << "ctorrent_client: connect(): " <<
          strerror_r( err, &buf.front(), buf.size() );

      close( remote_socket );
      continue; /* just go to the next server if current isn't accessible */
    }

    sockaddr_in cl_socket_addr;
    socklen_t addr_len = sizeof(cl_socket_addr);
    std::stringstream tmp;

    std::memset( &cl_socket_addr, 0, sizeof(cl_socket_addr) );
    getpeername( remote_socket, reinterpret_cast<sockaddr*>(&cl_socket_addr), &addr_len );

    tmp << "remote server (" << convert_ipv4_from_binary_to_text( cl_socket_addr.sin_addr )->c_str() << ":"
        << ntohs( cl_socket_addr.sin_port ) <<  ")";

    if( addr_len > sizeof(cl_socket_addr) )
      BOOST_LOG_TRIVIAL( info ) << "[client] [warning] a peer socket address has been truncated.\n";
    else
      BOOST_LOG_TRIVIAL( info ) << "ctorrent_client: connection to the server has been established -- " << tmp.str();

    auto rs = std::make_shared<remote_server>( remote_socket, received_objects, tmp.str() );

    /* TODO: deal with ECONNREFUSED (for recv()); */
    /* TODO: perform the clean operation */

    /* register an event handler to process a 'some data to read presents on a socket' event */
    epoll.add_event_source( remote_socket, epoll_event_loop::event_type::EPOLL_IN,
                            std::bind( [] (int fd, void* data, std::shared_ptr<remote_server> ptr) { ptr->consume_data_from_remote(); },
                                       std::placeholders::_1, std::placeholders::_2, std::shared_ptr<remote_server>(rs) ),
                            nullptr, "remote server socket" );

    remote_servers_list.push_back( std::move(rs) );

    /* it's epoll responsibility to manage fds, so we can close peer_socket here */
    close( remote_socket );
  }

  BOOST_LOG_TRIVIAL( debug ) << "ctorrent_client: amount of connected servers: " << remote_servers_list.size();

  if( remote_servers_list.size() < 1 ) /* TODO: what an edge should be? */
    throw std::string( "there's no connected servers." );
}

ctorrent_client::~ctorrent_client()
{
  /* TODO: how to deal with remote_servers? */
}

void ctorrent_client::send( const std::vector<std::shared_ptr<base_calc>>& objs, bool is_order_important )
{
  std::size_t objs_chunk_size;
  int i = 0;

  if( objs.empty() ) return;

  is_obj_order_important = is_order_important;

  objs_chunk_size = std::ceil( objs.size() / (remote_servers_list.size() * 1.0) );

  BOOST_LOG_TRIVIAL( info );

  /* spread objects to calculate between remote calculation servers as equally as possible */
  for( auto& remote_server : remote_servers_list )
  {
    auto begin_it = objs.cbegin() + objs_chunk_size * i++;
    int sent_obj_cnt = 0;

    for( auto it = begin_it; it != objs.cend() && it < begin_it + objs_chunk_size; ++it, ++sent_obj_cnt )
      remote_server->serialize_and_send( (*it).get() );

    remote_server->flush_serialized_data();

    BOOST_LOG_TRIVIAL( info ) << "ctorrent_client: " << sent_obj_cnt << " object(s) have been sent"
        " to the " << remote_server->get_identify_str();
  }

  BOOST_LOG_TRIVIAL( info );
}

ctorrent_client::results_t ctorrent_client::receive()
{
  results_t res;
  using std::swap;

  while( 1 )
  {
    /* can block a calling thread if there's no events */
    epoll.handle_events();

    /* if we got at least one object and order of objects isn't important,
     * due to receive() API we return this object(s) immediately */
    if( !is_obj_order_important && !received_objects.empty() )
      break;
  }

  if( !is_obj_order_important )
    swap( res, received_objects );

  return res;
}

/* make up a list of ctorrent servers from txt file
 * TODO: think about some broadcast protocol to spread list of ctorrent servers
 *       between servers and clients */
std::list<in_addr> ctorrent_client::get_servers_list()
{
  std::list<in_addr> servers_list;
  std::string line;
  int ret;

  std::ifstream servers_list_stream( "/usr/local/share/ctorrent/servers_list.txt" );

  if( !servers_list_stream.is_open() )
    throw std::string( "no \"servers_list.txt\" file." );

  while( std::getline( servers_list_stream, line ) )
  {
    in_addr address;

    ret = inet_pton( AF_INET, line.c_str(), &address );
    if( ret != 1 )
    {
      BOOST_LOG_TRIVIAL( error ) << "[client] [error] inet_pton(): " << ret << std::endl;
      throw std::string( "an error while trying to get binary representation of ipv4 address." );
    }

    servers_list.push_back( address );
  }

  return servers_list;
}

