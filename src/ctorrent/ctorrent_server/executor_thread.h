/*
 * executor_thread.h
 *
 *  Created on: Apr 2, 2018
 *      Author: sergs
 */

#ifndef EXECUTOR_THREAD_H
#define EXECUTOR_THREAD_H

#include "remote_client.h"
#include "notify_lock_queue.h"
#include "object.h"

class executor_thread : public object
{
public:
  executor_thread( notify_lock_queue<task_wrapper>& tasks_queue,
                   notify_lock_queue<result_wrapper>& results_queue );

  void operator()();

private:
  notify_lock_queue<task_wrapper>& tasks_queue; /* a REFERENCE */
  notify_lock_queue<result_wrapper>& results_queue; /* a REFERENCE */
};

#endif /* EXECUTOR_THREAD_H */
