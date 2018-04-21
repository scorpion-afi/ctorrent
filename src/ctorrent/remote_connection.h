/*
 * remote_connection.h
 *
 *  Created on: Mar 24, 2018
 *      Author: sergs
 */

#ifndef REMOTE_CONNECTION_H
#define REMOTE_CONNECTION_H

#include <vector>
#include <memory>
#include <string>

#include "object.h"
#include "serializer.h"
#include "deserializer.h"
#include "serializer_deserializer.h"

/* TODO: no-copy sementic */

/* this abstract-class describes a remote connection;
 * it's responsible for:
 * - holding socket's fd
 * - consuming data from a socket
 * - deserialization of consumed data
 * - serialization of data to send
 * - write serialized data to a socket
 *
 * it's up to this class to chose when it's a time to make an actually write to a socket;
 *
 * TODO: by default, derived classes are able to get/send only base_serialize objects,
 *       if other objects have to be sent too, those types have to be registered
 *       (by using register_type_for_serialization()   to register type to send,
 *                 register_type_for_deserialization() to register type to get)
 */
class remote_connection : public object
{
public:
  using deserialized_objs_t = std::vector<std::shared_ptr<base_serialize>>;

  remote_connection() = delete;

  /* socket_fd - a socket to wrap
   * identify_str - some string used to identify this remote connection for debug purposes */
  explicit remote_connection( int socket_fd, std::string identify_str = "remote connection" );

  virtual ~remote_connection();

  /* TODO: it's an awful way..., to serialize a object by a polymorphic pointer(reference) we
   * have to register a type of an actual derived class which destroys all dynamic polymorphism :),
   * as I know boost.serialize provides some way to fix it */
  template< class T >
  void register_type_for_serialization()
  {
    m_serializer.register_type<T>();
  }

  template< class T >
  void register_type_for_deserialization()
  {
    m_deserializer.register_type<T>();
  }

  /* consumes (reads) data from the remote side ('socket_fd');
   * this function has to be called if there's some data on 'socket_fd' fd,
   * this function blocks if there's no data to consume on 'socket_fd', otherwise
   * it doesn't block and, if some objects were deserialized, makes virtual call to
   * process_deserialized_objs() with deserialized objects as an argument to allow
   * a derived class perform some processing */
  void consume_data_from_remote();

  /* a class has no enough information to process deserialized objects */
  /* this function gets called if some objects were deserialized */
  virtual void process_deserialized_objs( deserialized_objs_t objs ) = 0;

  /* serializes and maybe sends serialized obj(s) to the remote side ('socket_fd'),
   * whether actual sending happens depends on internal state, if a caller needs to
   * send immediately it has to call 'flush_serialized_data' after this function,
   * but generally flush_serialized_data() can be called only at the end of bunch of
   * objects to send, thus allowing the implementation to work more effectively */
  void serialize_and_send( const base_serialize* obj );

  /* triggers serialization data to be send to the remote side ('socket_fd')*/
  void flush_serialized_data();

  const std::string& get_identify_str() const { return identify_str; }

private:
  int socket_fd;

  serializer   m_serializer;
  deserializer m_deserializer;

  std::string identify_str;
};

#endif /* REMOTE_CONNECTION_H */
