/*
 * serializer_deserializer.cpp
 *
 *  Created on: Mar 11, 2018
 *      Author: sergs
 */

#include "config.h"

#include <iostream>

#include "serializer_deserializer.h"

std::ostream& operator<<( std::ostream& o_stream, const serializer_deserializer::control_object& control_obj )
{
  return o_stream.write( reinterpret_cast<const char*>(&control_obj), serializer_deserializer::control_object_size );
}

std::istream& operator>>( std::istream& i_stream, serializer_deserializer::control_object& control_obj )
{
  return i_stream.read( reinterpret_cast<char*>(&control_obj), serializer_deserializer::control_object_size );
}
