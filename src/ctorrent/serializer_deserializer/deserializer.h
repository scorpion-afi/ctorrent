/*
 * deserializer.h
 *
 *  Created on: Mar 11, 2018
 *      Author: sergs
 */

#ifndef DESERIALIZER_H
#define DESERIALIZER_H

#include <array>
#include <vector>
#include <memory>
#include <sstream>

#include <boost/archive/binary_iarchive.hpp>

#include "serializer_deserializer.h"

/* this class is responsible for the deserialization;
 *
 * To deserialize objects by a polymorphic pointer real types of objects should be registered by a
 * register_type() call.
 *
 * The deserializer implements stl-containers' like functions front() and size() to provide
 * a convenient way for a client to provide data to deserialize from. The client has to put
 * the data to deserialize from to the internal storage [ &front(), &front() + size() ) .
 *
 * The deserializer keeps an internal buffer where it stores data to deserialize from, this
 * buffer has a finite size, so when it's exhausted it's impossible to consume data for
 * the deserialization and the buffer should be reset, by consuming already deserialized objects
 * (get_deserialized_objs() call), to continue the work.
 *
 * doesn't support a copy-semantic.
 */
class deserializer : public serializer_deserializer
{
public:
  using deserialized_objs = std::vector<std::unique_ptr<const base_serialize>>;

  deserializer();

  deserializer( const deserializer& that ) = delete;
  deserializer& operator=( const deserializer& that ) = delete;

  deserializer( deserializer&& that ) = default;
  deserializer& operator=( deserializer&& that ) = default;

  /* TODO: it's an awful way..., to deserialize a object by a polymorphic pointer(reference) we
   * have to register a type of an actual derived class which destroys all dynamic polymorphism :),
   * as I know boost.serialize provides some way to fix it */
  template< class T >
  void register_type()
  {
    /* to instantiate a T::serialize template function */
    raw_ar->register_type( static_cast<T*>(nullptr) );
  }

  /* return a reference to a front element of an internal storage the data, to deserialize from
   * (up to size()), should be stored on by a caller */
  char& front();

  /* a caller should write up to size() bytes */
  std::size_t size() const;

  /* make an actual deserialization;
   * num_of_read_bytes - amount of data (in bytes) written into the internal storage by a caller;
   * return a true if the deserialiation has finished, in such a case deserialized objects have to be
   * retrieved by get_deserialized_objs() call */
  bool deserialize( std::size_t num_of_read_bytes );

  /* retrieve deserialized objects and make preparation for the next deserialization */
  deserialized_objs get_deserialized_objs();

private:
  void reset();

private:
  /* TODO: std::basic_stringbuf<char> isn't the best choice... */
  std::stringbuf str_streambuf;

  /* a binary_iarchive is a no-copyable, no-movable and no-swappable type,
   * so to make a serializer a movable and swappable type we use a pointer */
  std::unique_ptr<boost::archive::binary_iarchive> raw_ar;

  deserialized_objs objs;
  std::array<char, client_package_size> raw_data;
  bool deserialization_finished;
  bool is_ctrl_obj_caught;
  std::size_t m_size;
  std::size_t offset;

  control_object control_obj;
};

#endif /* DESERIALIZER_H */
