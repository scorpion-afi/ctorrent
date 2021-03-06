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
 * doesn't support a copy-semantic.
 *
 * TODO: by default, derived classes are able to get/send only base_serialize objects,
 *       if other objects have to be sent too, those types have to be registered
 *       (by using register_type_for_serialization()   to register type to send,
 *                 register_type_for_deserialization() to register type to get)
 *
 *       when you register types for a serialization on one side you have to register
 *       the same types for the deserialization on the other side in the SAME order
 *       ('cause boost.serialize library associates integers with types you register and
 *       use those integers (on the deserialize side) to chose a type to deserialize to,
 *       so it's needed to have the same association(s) on the deserialize side)
 */
class remote_connection : public object
{
public:
  using deserialized_objs = deserializer::deserialized_objs;

  /* socket_fd - a socket to wrap
   * identify_str - some string used to identify this remote connection for debug purposes */
  explicit remote_connection( int socket_fd, std::string identify_str = "remote connection" );
  virtual ~remote_connection();

  remote_connection( const remote_connection& that ) = delete;
  remote_connection& operator=( const remote_connection& that ) = delete;

  remote_connection( remote_connection&& that ) = default;
  remote_connection& operator=( remote_connection&& that );

  friend void swap( remote_connection& lhs, remote_connection& rhs ) noexcept;

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
   * it doesn't block and, if some objects were deserialized, makes a virtual call to
   * process_deserialized_objs() with deserialized objects as an argument to allow
   * a derived class to perform some processing */
  void consume_data_from_remote();

  /* the class has no enough information to process deserialized objects; */
  /* this function gets called if some objects were deserialized */
  virtual void process_deserialized_objs( deserialized_objs objs ) = 0;

  /* serializes and maybe sends serialized obj(s) to the remote side ('socket_fd'),
   * whether actual sending happens depends on internal state, if a caller needs to
   * send immediately it has to call 'flush_serialized_data' after this function,
   * but generally flush_serialized_data() can be called only at the end of bunch of
   * objects to send, thus allowing the implementation to work more effectively */
  void serialize_and_send( const base_serialize* obj );

  /* triggers serialization data to be sent to the remote side ('socket_fd')*/
  void flush_serialized_data();

  const std::string& get_identify_str() const { return identify_str; }

private:
  using base = object;

private:
  int socket_fd;

  serializer   m_serializer;
  deserializer m_deserializer;

  std::string identify_str;
};

#endif /* REMOTE_CONNECTION_H */
