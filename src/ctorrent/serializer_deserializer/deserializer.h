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

class deserializer : public serializer_deserializer
{
public:
  deserializer();
  virtual ~deserializer();

  /* TODO: it's an awful way..., to deserialize a object by a polymorphic pointer(reference) we
   * have to register a type of an actual derived class which destroys all dynamic polymorphism :),
   * as I know boost.serialize provides some way to fix it */
  template< class T >
  void register_type()
  {
    /* to instantiate a T::serialize template function */
    raw_ar.register_type( static_cast<T*>(nullptr) );
  }

  char& front();
  std::size_t size() const;

  bool deserialize( std::size_t num_of_read_bytes );

  std::vector<std::shared_ptr<base_serialize>> get_deserialized_objs();

private:
  void reset();

private:
  /* TODO: std::basic_stringbuf<char> isn't the best choice... */
  std::stringbuf str_streambuf;
  boost::archive::binary_iarchive raw_ar;

  std::vector<std::shared_ptr<base_serialize>> deserialized_objs;
  std::array<char, client_package_size> raw_data;
  bool deserialization_finished;
  bool is_ctrl_obj_caught;
  std::size_t m_size;
  std::size_t offset;

  control_object control_obj;
};

#endif /* DESERIALIZER_H */
