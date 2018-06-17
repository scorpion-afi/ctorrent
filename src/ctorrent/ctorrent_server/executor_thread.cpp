/*
 * executor_thread.cpp
 *
 *  Created on: Apr 2, 2018
 *      Author: sergs
 */

#include "config.h"

#include <thread>

#include <boost/log/trivial.hpp>

#include "executor_thread.h"

executor_thread::executor_thread( notify_lock_queue<task>& tasks_queue,
                                    notify_lock_queue<result>& results_queue ) :
  tasks_queue(tasks_queue), results_queue(results_queue)
{
}

/* a thread function */
void executor_thread::operator()()
{
  BOOST_LOG_TRIVIAL( debug ) << "executor_thread [" << get_id() << "]: start a thread";

  while( true )
  {
    /* if a thread throws an exception it causes the whole program to be terminated, so prohibit it... */
    try
    {
      BOOST_LOG_TRIVIAL( debug ) << "executor_thread [" << get_id() << "]: wait for a task to compute";

      /* block if there's no task to consume/compute */
      std::shared_ptr<task> task = tasks_queue.pop();

      BOOST_LOG_TRIVIAL( info ) << "executor_thread [" << get_id() << "]: compute a task [" << task->get_id() << "]";

      result res = task->compute();

      BOOST_LOG_TRIVIAL( debug ) << "executor_thread [" << get_id() << "]: save a result [" << res.get_id() <<
          "] of the task computation";

      results_queue.push( std::move(res) );
    }
    catch( const std::string &err )
    {
      BOOST_LOG_TRIVIAL( info ) << "executor_thread [" << get_id() << "]: an exception: " << err;
    }
    catch( const std::exception &exception )
    {
      BOOST_LOG_TRIVIAL( info ) << "executor_thread [" << get_id() << "]: an std exception: " << exception.what();
    }
    catch(...)
    {
      BOOST_LOG_TRIVIAL( error ) << "executor_thread [" << get_id() << "]: some exception happened";
      /* TODO: delay till it's time to sort out with exceptions handling */
    }
  }
}

