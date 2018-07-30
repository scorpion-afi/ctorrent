/*
 * remote_client.cpp
 *
 *  Created on: Feb 21, 2018
 *      Author: sergs
 */

#include "config.h"

#include <boost/log/trivial.hpp>

#include "remote_client.h"

/* no duplication of fd, as a remote_connection is an 'fd holder' */
remote_client::remote_client( int socket_fd, notify_lock_queue<task>& tasks_queue,
                              std::string identify_str ) :
  remote_connection(socket_fd, std::move(identify_str)), tasks_queue(tasks_queue),
  received_objs_cnt(0), serialized_objs_cnt(0)
{
  register_type_for_serialization<calc_result>();

  register_type_for_deserialization<calc_chunk>();
}

void remote_client::process_deserialized_objs( deserialized_objs objs )
{
  BOOST_LOG_TRIVIAL( info ) << "remote_client [" << get_id() << "]: " << objs.size() << " object(s) have been received"
      " from the " << get_identify_str();

  /* TODO: some way to distinguish different types of packages (control, error, task, result...) should be here,
   *       now it's considered that all packages are task-packages, so they can be casted to a proper type
   *       without any logic */

  received_objs_cnt += objs.size();  /* amount of objects which expect replies to be sent back */

  /* put task-packages to the tasks queue;
   * the tasks queue is shared between several remote_clients */
  for( auto& elm : objs )
  {
    /* at this level we know that type of objects is, at least, base_calc, so we can perform
     * a static cast instead of a dynamic one */
    std::unique_ptr<const base_calc> bs_calc( static_cast<const base_calc*>(elm.release()) );
    task task( self_ref, std::move(bs_calc) );

    BOOST_LOG_TRIVIAL( debug ) << "remote_client [" << get_id() << "]: queue a task [" << task.get_id() << "] to compute";

    tasks_queue.push( std::move(task) ); /* TODO: check maybe a compiler is clever enough to perform this automatically? */
  }
}

void remote_client::serialize_and_send( const base_serialize* obj )
{
  remote_connection::serialize_and_send( obj );
  serialized_objs_cnt++;

  /* compare_exchange_strong may modify its argument */
  unsigned long long expected = serialized_objs_cnt;

  /* if an amount of serialized objects equals to an amount of received
   * perform an explicit flushing */
  if( received_objs_cnt.compare_exchange_strong( expected, 0 ) )
  {
    serialized_objs_cnt = 0;
    flush_serialized_data();
  }
}

