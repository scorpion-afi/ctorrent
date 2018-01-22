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

class send_thread : public object
{
public:
  explicit send_thread( notify_lock_queue<result_wrapper>& results_queue );

  void operator()();

private:
  notify_lock_queue<result_wrapper>& results_queue; /* a REFERENCE */
};

#endif /* SERVER_SEND_THREAD_H */
