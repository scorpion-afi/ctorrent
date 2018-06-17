/*
 * executor_thread.h
 *
 *  Created on: Apr 2, 2018
 *      Author: sergs
 */

#ifndef EXECUTOR_THREAD_H
#define EXECUTOR_THREAD_H

#include "task_result_wrappers.h"
#include "notify_lock_queue.h"
#include "object.h"

/* this class describes an executor thread which is responsible for the task computation;
 * no copy semantic */
class executor_thread : public object
{
public:
  executor_thread( notify_lock_queue<task>& tasks_queue,
                   notify_lock_queue<result>& results_queue );

  executor_thread( const executor_thread& that ) = delete;
  executor_thread& operator=( const executor_thread& that ) = delete;

  executor_thread( executor_thread&& that ) = default;
  executor_thread& operator=( executor_thread&& that ) = default;

  void operator()();

private:
  notify_lock_queue<task>& tasks_queue; /* a REFERENCE */
  notify_lock_queue<result>& results_queue; /* a REFERENCE */
};

#endif /* EXECUTOR_THREAD_H */
