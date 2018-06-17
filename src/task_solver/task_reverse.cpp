/*
 * task_reverse.cpp
 *
 *  Created on: Mar 31, 2018
 *      Author: sergs
 */

#include <vector>
#include <cstdint>
#include <memory>

#include "ctorrent_protocols.h"

struct reverse_string_data
{
  std::size_t str_offset; /* TODO: std::size_t is a bad idea for cross-platform system... */
  std::size_t str_size;

  std::size_t start_idx;
  std::size_t base;
};

extern "C" std::unique_ptr<const calc_result> compute( const calc_chunk& co )
{
  const reverse_string_data* reverse_data = static_cast<const reverse_string_data*>( co.get_data() );
  const char* str = static_cast<const char*>( co.get_data() ) + reverse_data->str_offset;

  std::unique_ptr<calc_result> res( new calc_result( co ) );

  res->data = new char[reverse_data->str_size];
  res->data_size = reverse_data->str_size;

  /* reverse a string */
  for( std::size_t i = 0, j = reverse_data->str_size - 1; i < reverse_data->str_size; ++i, --j )
    res->data[j] = str[i];

  return res;
}

