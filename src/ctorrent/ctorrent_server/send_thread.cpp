/*
 * send_thread.cpp
 *
 *  Created on: Apr 3, 2018
 *      Author: sergs
 */

#include "config.h"

#include <thread>

#include <boost/log/trivial.hpp>

#include "remote_client.h"

#include "send_thread.h"

send_thread::send_thread( notify_lock_queue<result>& results_queue )
  : results_queue(results_queue)
{
}

/* a thread function */
void send_thread::operator()()
{
  BOOST_LOG_TRIVIAL( debug ) << "send_thread [" << get_id() << "]: start a thread";

  while( true )
  {
    /* if a thread throws an exception it causes the whole program to be terminated, so prohibit it... */
    try
    {
      BOOST_LOG_TRIVIAL( debug ) << "send_thread [" << get_id() << "]: wait for a result to send...";

      /* block if there's no results to consume/send */
      std::unique_ptr<result> result = results_queue.pop();

      BOOST_LOG_TRIVIAL( debug ) << "send_thread [" << get_id() << "]: consume a result [" << result->get_id() << "] to send";

      auto remote_cl = result->get_client().lock();

      /* result_wrapper.get_client() returns a weak_ptr to a remote_client, so if the result
       * of the lock operation return false, it means that the remote_client was destroyed (a remote client
       * has closed connection), so we just skip this result package */
      if( !remote_cl )
        continue;

      /* at this point a lifetime of a remote_client object gets extended to the end of the loop iteration;
       * it's not a problem to use the remote_client to write, if a remote client closed connection, we'll
       * be notified about such situation via an exception */

      BOOST_LOG_TRIVIAL( info ) << "send_thread [" << get_id() << "]: send a result [" << result->get_id() << "] to client " <<
          remote_cl->get_identify_str();

      /* call to a remote_client version of serialize_and_send() which performs the flushing automatically
       * when it's needed */
      remote_cl->serialize_and_send( *result );
    }
    catch( const std::system_error& ex )
    {
      BOOST_LOG_TRIVIAL( error ) << "send_thread [" << get_id() << "]: " << ex.what() << " (" << ex.code().value() << ")";
    }
    catch( const std::exception& ex )
    {
      BOOST_LOG_TRIVIAL( error ) << "send_thread [" << get_id() << "]: " << ex.what();
    }
  }
}

