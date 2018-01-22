/*
 * serializer_deserializer_test.cpp
 *
 *  Created on: Mar 29, 2018
 *      Author: sergs
 */

#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <numeric>

#include <cassert>

#include "serializer.h"
#include "deserializer.h"
#include "serializer_deserializer.h"


struct dumb : public base_serialize
{
  dumb() = default; /* to serialize/deserialize an object a default ctr has to be provided */
  dumb( uint32_t a ) : a(a) {}

  uint32_t a;

private:
  /* an 'access' class should have an access to our private method 'serialize' */
  friend class boost::serialization::access;

  template<class Archive>
  void serialize( Archive& ar, const unsigned int version )
  {
    /* an insane way to serialize base part of derived object */
    ar & boost::serialization::base_object<base_serialize>(*this);

    ar & a;
  }
};

std::string serialize( const std::vector<dumb>& objs )
{
  serializer ser;

  ser.register_type<dumb>();

  for( const auto& elm : objs )
    ser.serialize( &elm );

  return ser.get_serialized_objs();
}

std::vector<std::shared_ptr<base_serialize>> deserialize( const std::string& serialized_data )
{
  deserializer deser;
  std::size_t size;

  deser.register_type<dumb>();

  size = serialized_data.size() < deser.size() ? serialized_data.size() : deser.size();
  std::copy( serialized_data.cbegin(), serialized_data.cbegin() + size, &deser.front() );

  if( deser.deserialize( size ) )
    return deser.get_deserialized_objs();

  return std::vector<std::shared_ptr<base_serialize>>();
}

/* just for fun... */
class kyky
{
public:
  std::size_t operator()( std::size_t val, const std::shared_ptr<base_serialize>& obj )
  {
    return val + static_cast<dumb*>(obj.get())->a;
  }
};

int main( void )
{
  std::vector<dumb> dump_array{ 1, 2, 3, 4, 5 };
  std::string serialized_data;
  std::vector<std::shared_ptr<base_serialize>> deserialized_objs;

  serialized_data = serialize( dump_array );
  deserialized_objs = deserialize( serialized_data );

  std::size_t res = std::accumulate( deserialized_objs.cbegin(), deserialized_objs.cend(), 0l, kyky() );
  assert( res == 15 );
}





