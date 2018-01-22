/*
 * remote_server.h
 *
 *  Created on: Mar 23, 2018
 *      Author: sergs
 */

#ifndef REMOTE_SERVER_H
#define REMOTE_SERVER_H

#include "serializer_deserializer.h"
#include "remote_connection.h"

class remote_server : public remote_connection
{
public:
  remote_server() = delete;
  remote_server( int socket_fd, deserialized_objs_t& receive_storage, std::string identify_str );

  /* this function gets called if some objects were deserialized */
  void process_deserialized_objs( deserialized_objs_t objs ) override;

private:
  deserialized_objs_t& receive_storage;  /* a REFERENCE */
};

#endif /* REMOTE_SERVER_H */
