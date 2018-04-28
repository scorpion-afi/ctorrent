/*
 * serializer.h
 *
 *  Created on: Mar 11, 2018
 *      Author: sergs
 */

#ifndef SERIALIZER_H
#define SERIALIZER_H

#include <sstream>
#include <memory>

#include <boost/archive/binary_oarchive.hpp>

#include "serializer_deserializer.h"

/* this class is responsible for the serialization;
 *
 * To serialize objects by a polymorphic pointer real types of objects should be registered by a
 * register_type() call.
 * Now the serializer supports serialization only for a one object at a time.
 *
 * The serializer keeps an internal buffer where it serializes objects, this
 * buffer has a finite size, so when it's exhausted it's impossible to continue the serialization
 * and the buffer should be reset, by consuming already serialized data (get_serialized_objs() call),
 * to continue the work.
 *
 * doesn't support a copy-semantic.
 */
class serializer : public serializer_deserializer
{
public:
  serializer();

  serializer( const serializer& that ) = delete;
  serializer& operator=( const serializer& that ) = delete;

  serializer( serializer&& that ) = default;
  serializer& operator=( serializer&& that ) = default;

  /* TODO: it's an awful way..., to serialize a object by a polymorphic pointer(reference) we
   * have to register a type of an actual derived class which obliterates all dynamic polymorphism :),
   * as I know boost.serialize provides some way to fix it */
  template< class T >
  void register_type()
  {
    /* to instantiate a T::serialize template function */
    raw_ar->register_type( static_cast<T*>(nullptr) );
  }

  /* serialize one object by using a polymorphic pointer */
  bool serialize( const base_serialize* obj );

  /* get a result of serialization, as a stream of bytes, of one or more objects
   * and make preparation for the next serialization */
  std::string get_serialized_objs();

private:
  void reset();

private:
  /* TODO: std::basic_stringbuf<char> isn't the best choice... */
  std::stringbuf str_streambuf;

  /* a binary_oarchive is a no-copyable, no-movable and no-swappable type,
   * so to make a serializer a movable and swappable type we use a pointer */
  std::unique_ptr<boost::archive::binary_oarchive> raw_ar;

  std::size_t last_obj_size;
  std::size_t objs_cnt;
  bool is_exceeded;
  bool is_control_obj;
};

#endif /* SERIALIZER_H */
