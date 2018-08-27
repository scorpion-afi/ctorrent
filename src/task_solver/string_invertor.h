/*
 * string_invertor.h
 *
 *  Created on: Jun 19, 2018
 *      Author: sergs
 */

#ifndef STRING_INVERTOR_H
#define STRING_INVERTOR_H

#include "string"

#include "object.h"

class string_invertor : public object
{
public:
  string_invertor();

  std::string get_invert_str( const std::string& str );

private:
  static const std::size_t m_chunk_size = 4;
  const std::string src_path = "/usr/local/share/task_solver/task_reverse.cpp";

  std::string method;

  static_assert( m_chunk_size != 0, "m_chunk_size has to not be equal zero" );
};

#endif /* STRING_INVERTOR_H */
