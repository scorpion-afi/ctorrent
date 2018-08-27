/*
 * task_reverse.cpp
 *
 *  Created on: Mar 31, 2018
 *      Author: sergs
 */

#include <cstdint>
#include <memory>
#include <algorithm>

#include "ctorrent_protocols.h"

struct reverse_string_data
{
  uint64_t str_offset;
  uint64_t str_size;
};

extern "C" std::unique_ptr<const calc_result> compute( const calc_chunk& co )
{
  const reverse_string_data* reverse_data = static_cast<const reverse_string_data*>( co.get_data() );
  const char* str = static_cast<const char*>( co.get_data() ) + reverse_data->str_offset;

  auto res = std::make_unique<calc_result>( co );

  res->data = new char[reverse_data->str_size];
  res->data_size = reverse_data->str_size;

  std::reverse_copy( str, str + reverse_data->str_size, res->data );

  return res;
}

