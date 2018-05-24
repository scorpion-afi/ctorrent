/*
 * send_thread.h
 *
 *  Created on: Apr 3, 2018
 *      Author: sergs
 */

#ifndef SERVER_SEND_THREAD_H
#define SERVER_SEND_THREAD_H

#include "remote_client.h"
#include "notify_lock_queue.h"
#include "object.h"

/* this class describes a send thread which is responsible to send results back;
 * no copy semantic */
class send_thread : public object
{
public:
  explicit send_thread( notify_lock_queue<result_wrapper>& results_queue );

  send_thread( const send_thread& that ) = delete;
  send_thread& operator=( const send_thread& that ) = delete;

  send_thread( send_thread&& that ) = default;
  send_thread& operator=( send_thread&& that ) = default;

  void operator()();

private:
  notify_lock_queue<result_wrapper>& results_queue; /* a REFERENCE */
};

#endif /* SERVER_SEND_THREAD_H */
