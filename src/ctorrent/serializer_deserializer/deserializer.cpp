/*
 * deserializer.cpp
 *
 *  Created on: Mar 11, 2018
 *      Author: sergs
 */

#include "config.h"

#ifdef DEBUG
#include <fstream>
#endif

#include <boost/log/trivial.hpp>

#include "deserializer.h"

#include "ctorrent_protocols.h"
deserializer::deserializer() :
  /* the serializer doesn't provide any meta-data, so the deserializer have to
   * not expect it too */
  str_streambuf(std::ios_base::in), raw_ar(str_streambuf, boost::archive::no_header)
{
  BOOST_LOG_TRIVIAL( debug ) << "deserializer: creaete a deserializer";
  reset();
}

deserializer::~deserializer()
{
  BOOST_LOG_TRIVIAL( debug ) << "deserializer: destroy the deserializer";
}

char& deserializer::front()
{
  return *(raw_data.begin() + offset);
}

std::size_t deserializer::size() const
{
  return m_size;
}

bool deserializer::deserialize( std::size_t num_of_read_bytes )
{
  if( deserialization_finished )
    return true;

  BOOST_LOG_TRIVIAL( debug ) << "deserializer: consume " << num_of_read_bytes << " bytes";

  offset += num_of_read_bytes;
  m_size -= num_of_read_bytes;

  if( offset >= control_object_size && !is_ctrl_obj_caught )
  {
    std::stringstream str_stream( std::string(raw_data.cbegin(), raw_data.cbegin() + control_object_size) );
    std::size_t pkg_size;

    str_stream >> control_obj;

    pkg_size = static_cast<std::size_t>(control_obj.pkg_size);
    m_size = std::min( client_package_size - offset, pkg_size - offset );

    is_ctrl_obj_caught = true;

    BOOST_LOG_TRIVIAL( debug ) << "deserializer: a control object has been caught";
  }

  if( m_size == 0 )
  {
    BOOST_LOG_TRIVIAL( debug ) << "deserializer: start the deserialization...";

#ifdef DEBUG
    std::ofstream file( "full-deserialization.txt", std::ios_base::out | std::ios_base::trunc | std::ios_base::binary );

    file << std::string( raw_data.begin(), raw_data.begin() + control_obj.pkg_size );
    file.close();

    file.open( "deserialization.txt", std::ios_base::out | std::ios_base::trunc | std::ios_base::binary );
    file << std::string( raw_data.begin() + control_object_size, raw_data.begin() + control_obj.pkg_size );
#endif

    /* re-arm boost::archive::binary_iarchive to the byte stream of serialized objects */
    str_streambuf.str( std::string( raw_data.begin() + control_object_size, raw_data.begin() + control_obj.pkg_size ) );

    for( std::size_t i = 0; i < control_obj.objs_amount; ++i )
    {
      base_serialize* so = nullptr;

      BOOST_LOG_TRIVIAL( debug ) << " deserialize an object";
      raw_ar >> so; /* TODO: how to get if the operation was successful? */

      deserialized_objs.push_back( std::shared_ptr<base_serialize>(so) );
    }

    deserialization_finished = true;

    return true;
  }

  return false;
}

std::vector<std::shared_ptr<base_serialize>> deserializer::get_deserialized_objs()
{
  std::vector<std::shared_ptr<base_serialize>> tmp;

  if( !deserialization_finished )
    return tmp;

  using std::swap;
  swap( tmp, deserialized_objs ); /* tmp has to be empty */

  reset();

  return tmp;
}

void deserializer::reset()
{
  deserialization_finished = false;
  m_size = client_package_size;
  offset = 0;

  is_ctrl_obj_caught = false;
}
