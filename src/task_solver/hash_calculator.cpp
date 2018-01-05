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

#include <boost/log/trivial.hpp>

#include "ctorrent_protocols.h"
#include "ctorrent.h"

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
  constexpr bool t = calc_chunk::check_type<hash_data>();
  (void)t;  /* to suppress the warning */

  std::ifstream method_src_file( "/usr/local/share/task_solver/task.cpp" );

  if( !method_src_file.is_open() )
    throw std::string( "no \"task.cpp\" file." );

  std::stringstream tmp;

  tmp << method_src_file.rdbuf(); /* to read all content regardless of any '\n' symbols */
  method = tmp.str();
}

uint64_t hash_calculator::get_hash( const std::string &str )
{
  std::vector<calc_chunk> chunks;
  std::size_t packages_cnt;

  if( !str.size() )
    throw std::string( "the string to calculate a hash for has no data." );

  packages_cnt = std::ceil( str.size() / (m_chunk_size * 1.0) );

  for( std::size_t i = 0; i < packages_cnt; i++ )
  {
    calc_chunk chunk;
    hash_data_impl data;

    const char *s;
    std::size_t str_size;
    std::size_t start_idx;

    s = str.c_str() + i * m_chunk_size;
    str_size = str.size() < m_chunk_size ? str.size() : m_chunk_size;
    start_idx = i * m_chunk_size;

    data.set_data( s, str_size, start_idx, base );

    chunk.grab_data( data );
    chunk.set_method_src( method );

    chunks.push_back( std::move(chunk) );
  }

  /* make the actual computation... */

  std::vector<calc_result> results;

  for( auto& chunk : chunks )
    results.push_back( chunk.compute() );

  uint64_t hash = 0;
  for( auto& res : results )
    hash += *reinterpret_cast<uint64_t*>(res.data);

  return hash;
}
