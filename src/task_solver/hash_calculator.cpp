/*
 * hash_calculator.cpp
 *
 *  Created on: Oct 4, 2017
 *      Author: sergs (ivan0ivanov0@mail.ru)
 */

#include "config.h"

#include <fstream>
#include <vector>
#include <deque>
#include <cmath>
#include <cstring>
#include <sstream>
#include <memory>

#include <boost/log/trivial.hpp>

#include "hash_calculator.h"


/* is a POD type */
struct hash_data
{
  std::size_t str_offset;
  std::size_t str_size;

  std::size_t start_idx;
  std::size_t base;
};

/* no copy semantic */
class hash_data_impl : public i_chunk_data
{
public:

  hash_data_impl() : i_chunk_data(), raw_data(nullptr), raw_data_size(0) {}
  ~hash_data_impl() { delete [] raw_data; }

  hash_data_impl( const hash_data_impl &that ) = delete;

  hash_data_impl( hash_data_impl &&that ) : hash_data_impl()
  {
    swap( *this, that );
  }

  hash_data_impl& operator=( hash_data_impl that )
  {
    swap( *this, that );

    return *this;
  }

  friend void swap( hash_data_impl &first, hash_data_impl &second )
  {
    using std::swap;

    swap( first.raw_data, second.raw_data );
    swap( first.raw_data_size, second.raw_data_size );
  }

  void set_data( const char *str, std::size_t str_size, std::size_t start_idx, std::size_t base );

  std::size_t get_raw_data_size() const override { return raw_data_size; }
  const void* get_raw_data() const override  { return raw_data; }

private:

  char *raw_data;
  std::size_t raw_data_size;
};

void hash_data_impl::set_data( const char *str, std::size_t str_size, std::size_t start_idx,
                               std::size_t base)
{
  hash_data *data;

  raw_data_size = sizeof *data + str_size;
  raw_data = new char[raw_data_size];

  data = reinterpret_cast<hash_data*>( raw_data );

  data->str_offset = sizeof *data;
  data->str_size = str_size;
  data->start_idx = start_idx;
  data->base = base;

  std::memcpy( raw_data + data->str_offset, str, data->str_size );
}

hash_calculator::hash_calculator()
{
  constexpr bool is_pod = calc_chunk::check_type<hash_data>();
  (void)is_pod;  /* to suppress the warning */

  std::ifstream method_src_file( "/usr/local/share/task_solver/task_reverse.cpp" );

  if( !method_src_file.is_open() )
    throw std::string( "no \"task.cpp\" file." );

  std::stringstream tmp;

  tmp << method_src_file.rdbuf(); /* to read all content regardless of any '\n' symbols */
  method = tmp.str();
}

uint64_t hash_calculator::get_hash( const std::string &str )
{
  std::vector<std::shared_ptr<base_calc>> chunks;
  std::size_t packages_cnt;

  if( !str.size() )
    throw std::string( "the string to calculate a hash for has no data." );

  packages_cnt = std::ceil( str.size() / (m_chunk_size * 1.0) );

  BOOST_LOG_TRIVIAL( info ) << "hash_calculator: amount of calculation objects: " << packages_cnt;
  BOOST_LOG_TRIVIAL( info );

  for( std::size_t i = 0; i < packages_cnt; i++ )
  {
    auto chunk = std::make_shared<calc_chunk>();
    hash_data_impl data;

    const char *s;
    std::size_t str_size;
    std::size_t start_idx;

    s = str.c_str() + i * m_chunk_size;
    str_size = str.size() < m_chunk_size ? str.size() : m_chunk_size;
    start_idx = i * m_chunk_size;

    data.set_data( s, str_size, start_idx, base );

    chunk->grab_data( data );
    chunk->set_method_src( method );

    chunks.push_back( std::move(chunk) );
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
