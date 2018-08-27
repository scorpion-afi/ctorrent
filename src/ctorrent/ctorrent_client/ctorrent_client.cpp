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
#include <algorithm>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h> /* IPv4 */

#include <boost/log/trivial.hpp>

#include "task_distributer.h"

#include "ctorrent_client.h"


ctorrent_client::ctorrent_client() : current_seq_id(0)
{
  sockaddr_in server_addr;
  int remote_socket; /* TODO: wrap a socket to provide a RAII mechanism */
  int ret;

  std::list<in_addr> servers_list = get_servers_list();

  BOOST_LOG_TRIVIAL( debug ) << "ctorrent_client: servers list:";
  for( auto& server : servers_list )
    BOOST_LOG_TRIVIAL( debug ) << " " << convert_ipv4_from_binary_to_text( server ).c_str();

  for( auto& server : servers_list )
  {
    remote_socket = socket( AF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0 );
    if( remote_socket < 0 )
      throw std::system_error( errno, std::system_category(), "an error while trying to create a socket" );

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
      continue; /* just go to the next server if a current one isn't accessible */
    }

    sockaddr_in cl_socket_addr;
    socklen_t addr_len = sizeof(cl_socket_addr);
    std::stringstream tmp;

    std::memset( &cl_socket_addr, 0, sizeof(cl_socket_addr) );
    getpeername( remote_socket, reinterpret_cast<sockaddr*>(&cl_socket_addr), &addr_len );

    tmp << "remote server (" << convert_ipv4_from_binary_to_text( cl_socket_addr.sin_addr ).c_str() << ":"
        << ntohs( cl_socket_addr.sin_port ) <<  ")";

    if( addr_len > sizeof(cl_socket_addr) )
      BOOST_LOG_TRIVIAL( info ) << "ctorrent_client: a peer socket address has been truncated.\n";
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
    throw std::runtime_error( "there's no connected servers." );
}

ctorrent_client::~ctorrent_client()
{
  /* TODO: how to deal with remote_servers? */
}

void ctorrent_client::send( const std::vector<std::shared_ptr<base_calc>>& objs, bool is_order_important )
{
  if( objs.empty() )
    throw std::domain_error( "there's no tasks to send (distribute)." );

  is_obj_order_important = is_order_important;
  current_seq_id = 0;

  uint64_t id = current_seq_id;
  auto generate_id = [&id] ( const std::shared_ptr<base_calc>& task ) { task->set_sequence_id( id++ ); };

  /* provide a continuous ascending order of sequence ids for objects to send (to be able to restore
   * the order of received objects (replies/results)) */
  std::for_each( objs.begin(), objs.end(), generate_id );

  auto task_distrib = make_task_distributer();
  task_distrib->distribute( remote_servers_list, objs );
}

ctorrent_client::results ctorrent_client::receive()
{
  results res;

  while( 1 )
  {
    /* can block a calling thread if there's no events */
    epoll.handle_events();

    if( received_objects.empty() ) continue;

    /* if we got at least one object and order of objects isn't important,
     * we return this object(s) immediately */
    if( !is_obj_order_important )
      break;

    for( const auto& elm : received_objects )
      BOOST_LOG_TRIVIAL( debug ) << elm.get()->get_sequence_id();

    BOOST_LOG_TRIVIAL( debug ) << "ctorrent_client: sort a received objects, count: " << received_objects.size();
    received_objects.sort( [] ( const auto& a, const auto&b ) { return *a < *b; } );

    for( const auto& elm : received_objects )
      BOOST_LOG_TRIVIAL( debug ) << elm.get()->get_sequence_id();

    const auto front_result = received_objects.front().get();
    if( front_result->get_sequence_id() == current_seq_id )
    {
      const auto back_result = received_objects.back().get();
      current_seq_id = back_result->get_sequence_id() + 1;  /* set up a new most-top object to wait for */

      break;
    }
  }

  using std::swap;
  swap( res, received_objects );

  return res;
}

/* make up a list of ctorrent servers from txt file
 * TODO: think about some broadcast protocol to spread a list of ctorrent servers
 *       between servers and clients */
std::list<in_addr> ctorrent_client::get_servers_list()
{
  std::list<in_addr> servers_list;
  std::string line;
  int ret;

  /* TODO: magic string */
  std::ifstream servers_list_stream( "/usr/local/share/ctorrent/servers_list.txt" );

  if( !servers_list_stream.is_open() )
    throw std::runtime_error( "no \"servers_list.txt\" file." );

  while( std::getline( servers_list_stream, line ) )
  {
    in_addr address;

    ret = inet_pton( AF_INET, line.c_str(), &address );
    if( ret != 1 )
      throw std::system_error( errno, std::system_category(),
                               "an error while trying to get binary representation of ipv4 address, inet_pton" );

    servers_list.push_back( address );
  }

  return servers_list;
}
