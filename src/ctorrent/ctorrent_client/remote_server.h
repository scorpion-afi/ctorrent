/*
 * remote_server.h
 *
 *  Created on: Mar 23, 2018
 *      Author: sergs
 */

#ifndef REMOTE_SERVER_H
#define REMOTE_SERVER_H

#include <string>
#include <list>

#include "remote_connection.h"

class base_calc_result;

/* this class describes connection to a remote server;
 * can be used concurrently for receiving and sending from different threads
 * without synchronization */
class remote_server : public remote_connection
{
public:
  using results = std::list<std::unique_ptr<const base_calc_result>>;

  remote_server( int socket_fd, results& receive_storage, std::string identify_str );

  remote_server( const remote_server& that ) = delete;
  remote_server& operator=( const remote_server& that ) = delete;

  remote_server( remote_server&& that ) = default;
  remote_server& operator=( remote_server&& that ) = default;

  /* this function gets called if some objects were deserialized */
  void process_deserialized_objs( deserialized_objs objs ) override;

private:
  results& receive_storage;  /* a REFERENCE */
};

#endif /* REMOTE_SERVER_H */
