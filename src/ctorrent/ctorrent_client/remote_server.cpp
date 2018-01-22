/*
 * remote_server.cpp
 *
 *  Created on: Mar 23, 2018
 *      Author: sergs
 */

#include "config.h"

#include <boost/log/trivial.hpp>

#include "ctorrent_protocols.h"
#include "remote_server.h"

/* no duplication of fd, as a remote_connection is an 'fd holder' */
remote_server::remote_server( int socket_fd, deserialized_objs_t& receive_storage, std::string identify_str ) :
  remote_connection(socket_fd, std::move(identify_str)), receive_storage(receive_storage)
{
  register_type_for_serialization<calc_chunk>();

  register_type_for_deserialization<calc_result>();
}

void remote_server::process_deserialized_objs( deserialized_objs_t objs )
{
  BOOST_LOG_TRIVIAL( info ) << "remote_server: " << objs.size() << " object(s) have been received"
      " from the " << get_identify_str();

  /* a 'receive_storage' container is shared between several remote_servers, that's why it's
   * impossible to make a swap operation, so just make an insertion operation instead */
  receive_storage.insert( receive_storage.cend(), objs.cbegin(), objs.cend() );
}
