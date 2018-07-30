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
remote_server::remote_server( int socket_fd, results& receive_storage, std::string identify_str ) :
  remote_connection(socket_fd, std::move(identify_str)), receive_storage(receive_storage)
{
  register_type_for_serialization<calc_chunk>();

  register_type_for_deserialization<calc_result>();
}

void remote_server::process_deserialized_objs( deserialized_objs objs )
{
  BOOST_LOG_TRIVIAL( info ) << "remote_server: " << objs.size() << " object(s) have been received"
      " from the " << get_identify_str();

  /* TODO: some way to distinguish different types of packages (control, error, task, result...) should be here,
   *       now it's considered that all packages are result-packages, so they can be casted to a proper type
   *       without any logic */

  results tmp;

  /* at this level we know that type of objects is, at least, base_calc_result, so we can perform
   * a static cast instead of a dynamic one */
  for( auto& elm : objs )
    tmp.emplace_back( static_cast<const base_calc_result*>(elm.release()) );

  /* the 'receive_storage' container is shared between several remote_servers */
  receive_storage.splice( receive_storage.cbegin(), tmp );
}
