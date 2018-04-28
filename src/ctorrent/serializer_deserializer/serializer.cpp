/*
 * serializer.cpp
 *
 *  Created on: Mar 11, 2018
 *      Author: sergs
 */

#include "config.h"

#include <boost/log/trivial.hpp>

#include "serializer.h"

serializer::serializer() :
  /* the simplest way to reuse boost::archive::binary_oarchive object is just omit some
   * meta-data inserted by default by using no_header flag; otherwise it should be
   * recreated each time we're going to serialize another set of objects (after
   * get_serialized_objs() call) */
  str_streambuf(std::ios_base::out), raw_ar(new boost::archive::binary_oarchive(str_streambuf, boost::archive::no_header))
{
  BOOST_LOG_TRIVIAL( debug ) << "serializer: create a serializer";

  reset();
}

bool serializer::serialize( const base_serialize* obj )
{
  std::size_t serialized_objs_size, prev_size;

  if( is_exceeded )
    return false;

  prev_size = str_streambuf.str().size();

  BOOST_LOG_TRIVIAL( debug ) << " serialize an object";

  *raw_ar << obj; /* serialize by using a polymorphic pointer */

  serialized_objs_size = str_streambuf.str().size();

  if( serialized_objs_size > (client_package_size - control_object_size) )
  {
    BOOST_LOG_TRIVIAL( debug ) << " full size of serialized objs exceeds the max allowed size (" <<
        client_package_size - control_object_size << ")";

    is_exceeded = true;
    last_obj_size = serialized_objs_size - prev_size;

    return false;
  }

  objs_cnt++;

  return true;
}

std::string serializer::get_serialized_objs()
{
  std::string tmp = str_streambuf.str();

  if( tmp.empty() )
    return std::string();

  std::stringstream str_stream;
  control_object control_obj;

  control_obj.pkg_size = tmp.size() + control_object_size;
  control_obj.objs_amount = objs_cnt;

  str_stream << control_obj;

  if( is_exceeded )
    tmp.erase( tmp.size() - last_obj_size, last_obj_size );

  reset();

  /* currently there's no other way to add a control object
   * to a client package to send */
  return str_stream.str() + tmp; /* TODO: shrink_to_fit() is it so necessary? */
}

void serializer::reset()
{
  /* allow a serialization being started at the beginning :) */
  std::string tmp;
  str_streambuf.str( tmp );

  last_obj_size = 0;
  objs_cnt = 0;
  is_exceeded = false;
  is_control_obj = true;
}
