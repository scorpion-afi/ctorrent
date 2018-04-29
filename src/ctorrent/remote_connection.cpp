/*
 * remote_connection.cpp
 *
 *  Created on: Mar 24, 2018
 *      Author: sergs
 */

#include "config.h"

#ifdef DEBUG
#include <fstream>
#endif

#include <string>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <boost/log/trivial.hpp>

#include "remote_connection.h"

remote_connection::remote_connection( int socket_fd, std::string identify_str ) :
  socket_fd(dup( socket_fd )), identify_str(std::move(identify_str))
{
  BOOST_LOG_TRIVIAL( debug ) << "remote_connection: create a new remote_connection obj";
}

remote_connection::~remote_connection()
{
  BOOST_LOG_TRIVIAL( debug ) << "remote_connection: destroy the remote_connection obj";

  close( socket_fd );
}

remote_connection& remote_connection::operator=( remote_connection&& that )
{
  swap( *this, that );

  return *this;
}

/* TODO: add more guarantees that the function doesn't throw an exception */
void swap( remote_connection& lhs, remote_connection& rhs ) noexcept
{
  using std::swap;

  /* swap base parts of swapping objects */
  swap( static_cast<remote_connection::base&>(lhs), static_cast<remote_connection::base&>(rhs) );

  swap( lhs.socket_fd, rhs.socket_fd );
  swap( lhs.m_serializer, rhs.m_serializer );
  swap( lhs.m_deserializer, rhs.m_deserializer );
  swap( lhs.identify_str, rhs.identify_str );
}

void remote_connection::consume_data_from_remote()
{
  int ret;

  BOOST_LOG_TRIVIAL( debug ) << "remote_connection: a new data to read...";

  while( 1 )
  {
    /* try to get up to request_data.size() bytes from the socket */
    ret = recv( socket_fd, &m_deserializer.front(), m_deserializer.size(), 0 );

    if( ret < 0 && errno == EINTR )
      continue;

    if( ret < 0 )
    {
      int err = errno;
      std::array<char, 256> buf;

      BOOST_LOG_TRIVIAL( error ) << "remote_connection: recv(): " << strerror_r( err, &buf.front(), buf.size() ) << std::endl;
      break;
    }

    /* a remote connection (socket) has been closed */
    if( ret == 0 )
    {
      /* actual work is performed by a more higher level */
      BOOST_LOG_TRIVIAL( debug ) << "remote_connection: remote client has closed a socket";
      break;
    }

    if( m_deserializer.deserialize( ret ) )
    {
      BOOST_LOG_TRIVIAL( debug ) << "remote_connection: deserialization has been finished.";

      this->process_deserialized_objs( m_deserializer.get_deserialized_objs() );
    }

    break;
  }
}

void remote_connection::serialize_and_send( const base_serialize* obj )
{
  if( !m_serializer.serialize( obj ) )
  {
    flush_serialized_data();

    /* we have to serialize an object anyway;
     * the serializer is able to serialize at least one object anyway */
    m_serializer.serialize( obj );
  }
}

void remote_connection::flush_serialized_data()
{
  std::string serialized_objs = m_serializer.get_serialized_objs();

  if( serialized_objs.empty() )
    return;

  BOOST_LOG_TRIVIAL( debug ) << "remote_connection: send serialized objects (" << serialized_objs.size() << " bytes)"
      " to the remote socket (send())";
  send( socket_fd, serialized_objs.c_str(), serialized_objs.size(), MSG_NOSIGNAL );

#ifdef DEBUG
  std::ofstream file( "serialization.txt", std::ios_base::out | std::ios_base::trunc | std::ios_base::binary );
  file << serialized_objs;
#endif
}
