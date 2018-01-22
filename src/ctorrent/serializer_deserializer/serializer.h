/*
 * serializer.h
 *
 *  Created on: Mar 11, 2018
 *      Author: sergs
 */

#ifndef SERIALIZER_H
#define SERIALIZER_H

#include <sstream>

#include <boost/archive/binary_oarchive.hpp>

#include "serializer_deserializer.h"

class serializer : public serializer_deserializer
{
public:
  serializer();
  virtual ~serializer();

  /* TODO: it's an awful way..., to serialize a object by a polymorphic pointer(reference) we
   * have to register a type of an actual derived class which destroys all dynamic polymorphism :),
   * as I know boost.serialize provides some way to fix it */
  template< class T >
  void register_type()
  {
    /* to instantiate a T::serialize template function */
    raw_ar.register_type( static_cast<T*>(nullptr) );
  }

  bool serialize( const base_serialize* obj );

  std::string get_serialized_objs();

private:
  void reset();

private:
  /* TODO: std::basic_stringbuf<char> isn't the best choice... */
  std::stringbuf str_streambuf;
  boost::archive::binary_oarchive raw_ar;

  std::size_t last_obj_size;
  std::size_t objs_cnt;
  bool is_exceeded;
  bool is_control_obj;
};

#endif /* SERIALIZER_H */
