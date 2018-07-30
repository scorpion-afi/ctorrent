/*
 * helper_function.h
 *
 *  Created on: Jun 19, 2018
 *      Author: sergs
 */

#ifndef TASK_SOLVER_HELPER_FUNCTION_H
#define TASK_SOLVER_HELPER_FUNCTION_H

#include <string>
#include <vector>
#include <memory>
#include <cmath>

#include "ctorrent_protocols.h"
#include "ctorrent_client.h"

/* this template function splits the @c str string into
 * std::ceil( str.size() / (chunk_size * 1.0) ) chunks and for each chunk
 * call a @c f functional object
 *
 * functional object @c f has the next signature:
 *  void(*)(std::size_t chunk_idx, const char* sub_str, std::size_t sub_str_size)
 *   sub_str - points to a sub string inside @c str string */
template< class func >
void split_string( const std::string& str, std::size_t chunk_size, func f )
{
  std::size_t substr_num, substr_size, last_substr_size;

  if( !str.size() )
    throw std::string( "the string to split has no data." );

  if( !chunk_size )
    throw std::string( "the m_chunk_size can't be a zero." );

  substr_num = std::ceil( str.size() / (chunk_size * 1.0) );
  substr_size = str.size() < chunk_size ? str.size() : chunk_size;
  last_substr_size = str.size() % chunk_size;
  last_substr_size = last_substr_size ?: substr_size;

  for( std::size_t i = 0; i < substr_num; i++ )
  {
    std::size_t str_size;
    const char* s;

    /* a last chunk and non-last chunk(s) may have different sizes */
    if( i == (substr_num - 1) )
      str_size = last_substr_size;
    else
      str_size = substr_size;

    s = str.c_str() + i * chunk_size;

    f( i, s, str_size );
  }
}

/* this template function performs a distributed calculation of @c tasks tasks
 * a @c f functional object gets called for each tasks' result
 *
 * order_important - whether an order of task's result is important
 *
 * functional object @c f has the next signature:
 *  void (*)( std::unique_ptr<const base_calc_result> result )
 *   a @c result should be casted to a proper type, depend on a task's type */
template< class func >
void distribute_calculation( const std::vector<std::shared_ptr<base_calc>>& tasks, func f, bool order_important = true )
{
  ctorrent_client cl;
  std::size_t received_cnt = 0;

  cl.send( tasks, order_important );

  while( received_cnt < tasks.size() )
  {
    ctorrent_client::results results = cl.receive();

    /* steal data from results, yes, but results is going to be destroyed, so don't worry */
    for( auto& res: results )
      f( std::move(res) );

    received_cnt += results.size();
  }
}

#endif /* TASK_SOLVER_HELPER_FUNCTION_H */
