/*
 * string_invertor.cpp
 *
 *  Created on: Jun 19, 2018
 *      Author: sergs
 */

#include "config.h"

#include <vector>
#include <memory>
#include <fstream>
#include <sstream>
#include <cstdint>

#include <boost/log/trivial.hpp>

#include "ctorrent_protocols.h"
#include "helper_function.h"

#include "string_invertor.h"

struct reverse_string_data
{
  uint64_t str_offset;
  uint64_t str_size;
};

static_assert( std::is_pod<reverse_string_data>::value, "reverse_string_data has to be a POD type" );

string_invertor::string_invertor()
{
  std::ifstream method_src_file( src_path );

  if( !method_src_file.is_open() )
    throw std::runtime_error( "string_invertor: no \"" + src_path + "\" file." );

  std::stringstream tmp;

  tmp << method_src_file.rdbuf(); /* to read all content regardless of any '\n' symbols */
  method = tmp.str();
}

std::string string_invertor::get_invert_str( const std::string& str )
{
  std::vector<std::shared_ptr<base_calc>> chunks;

  /* split the string into several substrings and create for each substring a
   * data chunk which will be used during computation on the server side by
   * a @c method, create for each substring a calc_chunk object to distribute
   * the computation */

  auto create_task = [&chunks, this]( std::size_t chunk_idx, const char* sub_str, std::size_t sub_str_size )
  {
    std::size_t data_chunk_size;
    std::unique_ptr<char[]> data_chunk;
    reverse_string_data* data;

    data_chunk_size = sizeof(reverse_string_data) + sub_str_size;

    data_chunk.reset( new char[data_chunk_size] );
    data = new( data_chunk.get() ) reverse_string_data;

    data->str_offset = sizeof(reverse_string_data);
    data->str_size = sub_str_size;

    std::copy_n( sub_str, data->str_size, data_chunk.get() + data->str_offset );

    chunks.emplace_back( std::make_shared<calc_chunk>( method, std::move(data_chunk), data_chunk_size ) );
  };

  split_string( str, m_chunk_size, create_task );

  BOOST_LOG_TRIVIAL( info ) << "string_invertor: amount of tasks to calculate: " << chunks.size();

  /* compute... */

  std::string inverted_str;
  auto handle_results = [&inverted_str] ( std::unique_ptr<const base_calc_result> result )
  {
    /* we know that we sent calc_chunk objects which have calc_result objects as results,
     * so we can make a static_cast instead of a dynamic_cast */
    auto res = static_cast<const calc_result*>(result.get());

    /* we have to perform a left-side lexicographic add */
    inverted_str = std::string( res->data, res->data_size ) + inverted_str;
  };

  /* make the actual computation... */
  distribute_calculation( chunks, handle_results );

  return inverted_str;
}

