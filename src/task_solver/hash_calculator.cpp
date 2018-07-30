/*
 * hash_calculator.cpp
 *
 *  Created on: Oct 4, 2017
 *      Author: sergs (ivan0ivanov0@mail.ru)
 */

#include "config.h"

#include <vector>
#include <memory>
#include <fstream>
#include <sstream>
#include <algorithm>

#include <boost/log/trivial.hpp>

#include "ctorrent_protocols.h"
#include "helper_function.h"

#include "hash_calculator.h"

/* is a POD type */
struct hash_data
{
  uint64_t str_offset;
  uint64_t str_size;

  uint64_t start_idx;
  uint64_t base;
};

static_assert( std::is_pod<hash_data>::value, "hash_data has to be a POD type" );

hash_calculator::hash_calculator()
{
  assert( m_chunk_size != 0 );

  std::ifstream method_src_file( src_path );

  if( !method_src_file.is_open() )
    throw std::string( "no \"" + src_path + "\" file." );

  std::stringstream tmp;

  tmp << method_src_file.rdbuf(); /* to read all content regardless of any '\n' symbols */
  method = tmp.str();
}

uint64_t hash_calculator::get_hash( const std::string& str )
{
  std::vector<std::shared_ptr<base_calc>> chunks;

  /* split the string into several substrings and create for each substring a
   * data chunk which will be used during computation on the server side by
   * a @c method, create for each substring a calc_chunk object to distribute
   * the computation */

  auto create_task = [&chunks, this]( std::size_t chunk_idx, const char* sub_str, std::size_t sub_str_size )
  {
    std::size_t start_idx, data_chunk_size;
    std::unique_ptr<char[]> data_chunk;
    hash_data* data;

    data_chunk_size = sizeof(hash_data) + sub_str_size;

    data_chunk.reset( new char[data_chunk_size] );
    data = new( data_chunk.get() ) hash_data;

    start_idx = chunk_idx * m_chunk_size;

    data->str_offset = sizeof(hash_data);
    data->str_size = sub_str_size;
    data->start_idx = start_idx;
    data->base = base;

    std::copy_n( sub_str, data->str_size, data_chunk.get() + data->str_offset );

    chunks.emplace_back( std::make_shared<calc_chunk>( method, std::move(data_chunk), data_chunk_size ) );
  };

  split_string( str, m_chunk_size, create_task );

  BOOST_LOG_TRIVIAL( info ) << "hash_calculator: amount of tasks to calculate: " << chunks.size();

  /* compute... */

  uint64_t hash = 0;
  auto handle_results = [&hash] ( std::unique_ptr<const base_calc_result> result )
  {
    /* we know that we sent calc_chunk objects which have calc_result objects as results,
     * so we can make a static_cast instead of a dynamic_cast */
    auto res = static_cast<const calc_result*>(result.get());

    hash += *reinterpret_cast<uint64_t*>(res->data);
  };

  /* make the actual computation... */
  distribute_calculation( chunks, handle_results, false );

  return hash;
}
