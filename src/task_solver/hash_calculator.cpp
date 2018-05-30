/*
 * hash_calculator.cpp
 *
 *  Created on: Oct 4, 2017
 *      Author: sergs (ivan0ivanov0@mail.ru)
 */

#include "config.h"

#include <fstream>
#include <vector>
#include <cmath>
#include <cstring>
#include <sstream>
#include <memory>

#include <boost/log/trivial.hpp>

#include "ctorrent_protocols.h"
#include "ctorrent_client.h"
#include "hash_calculator.h"


/* is a POD type */
struct hash_data
{
  std::size_t str_offset;
  std::size_t str_size;

  std::size_t start_idx;
  std::size_t base;
};

static_assert( std::is_pod<hash_data>::value, "hash_data has to be a POD type" );

hash_calculator::hash_calculator()
{
  std::ifstream method_src_file( "/usr/local/share/task_solver/task_reverse.cpp" );

  if( !method_src_file.is_open() )
    throw std::string( "no \"task.cpp\" file." );

  std::stringstream tmp;

  tmp << method_src_file.rdbuf(); /* to read all content regardless of any '\n' symbols */
  method = tmp.str();
}

uint64_t hash_calculator::get_hash( const std::string& str )
{
  std::vector<std::shared_ptr<base_calc>> chunks;
  std::size_t substr_num, substr_size, last_substr_size;

  if( !str.size() )
    throw std::string( "the string to calculate a hash for has no data." );

  /* split the string into several substrings and create for each substring a
   * data chunk which will be used during computation on the server side by
   * a @c method, create for each substring a calc_chunk object to distribute
   * the computation */

  substr_num = std::ceil( str.size() / (m_chunk_size * 1.0) );
  substr_size = str.size() < m_chunk_size ? str.size() : m_chunk_size;
  last_substr_size = str.size() % m_chunk_size;
  last_substr_size = last_substr_size ?: substr_size;

  BOOST_LOG_TRIVIAL( info );
  BOOST_LOG_TRIVIAL( info ) << "hash_calculator: amount of calculation objects: " << substr_num;

  for( std::size_t i = 0; i < substr_num; i++ )
  {
    std::size_t start_idx, str_size, data_chunk_size;
    std::unique_ptr<char[]> data_chunk;
    hash_data* data;
    const char* s;

    /* to avoid memory overhead for the last chunk */
    if( i == (substr_num - 1) )
      str_size = last_substr_size;
    else
      str_size = substr_size;
    
    data_chunk_size = sizeof(hash_data) + str_size;

    data_chunk.reset( new char[data_chunk_size] );
    data = new( data_chunk.get() ) hash_data;
     
    s = str.c_str() + i * m_chunk_size;
    start_idx = i * m_chunk_size;

    data->str_offset = sizeof(hash_data);
    data->str_size = str_size;
    data->start_idx = start_idx;
    data->base = base;

    std::memcpy( data_chunk.get() + data->str_offset, s, data->str_size );

    chunks.emplace_back( std::make_shared<calc_chunk>( method, std::move(data_chunk), data_chunk_size ) );
  }

  /* make the actual computation... */
  return distribute_calculation( chunks );
}

uint64_t hash_calculator::distribute_calculation( const std::vector<std::shared_ptr<base_calc>>& objs )
{
  std::size_t received_cnt = 0;
  std::string res_str;
  uint64_t hash = 0;

  ctorrent_client cl;

  cl.send( objs, false );

  while( received_cnt < objs.size() )
  {
    ctorrent_client::results_t results = cl.receive();

    for( const auto& res : results )
    {
      /* ctorrent_client library returns base_serialize objects and it's our task to cast them to proper
       * type, as ctorrent_client library guarantees that result objects are at least base_calc_result based
       * and we know that we sent calc_chunk objects which have calc_result objects as results we can make
       * static_cast instead of dynamic_cast */
      auto calc_res = std::static_pointer_cast<const calc_result>(res);

      /* an action specific to the task we're computing */
      //hash += *reinterpret_cast<uint64_t*>(calc_res->data);

      /* we have to perform a left-side lexicographic add */
      res_str = std::string( calc_res->data, calc_res->data_size ) + res_str;
    }

    received_cnt += results.size();
  }

  BOOST_LOG_TRIVIAL( info );
  BOOST_LOG_TRIVIAL( info ) << "hash_calculator: a reversed string: " << res_str;
  BOOST_LOG_TRIVIAL( info );

  return hash;
}
