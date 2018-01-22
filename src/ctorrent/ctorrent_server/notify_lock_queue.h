/*
 * notify_lock_queue.h
 *
 *  Created on: Apr 2, 2018
 *      Author: sergs
 */

#ifndef NOTIFY_LOCK_QUEUE_H
#define NOTIFY_LOCK_QUEUE_H

#include <queue>
#include <memory>
#include <mutex>
#include <condition_variable>

template <class T>
class notify_lock_queue
{
public:
  notify_lock_queue( std::size_t max_size );

  notify_lock_queue( const notify_lock_queue& other ) = delete;
  notify_lock_queue( notify_lock_queue&& other ) = delete;

  notify_lock_queue operator=( notify_lock_queue other ) = delete;

  void push( T elm );

  std::shared_ptr<T> pop();

private:
  std::mutex mtx;
  std::condition_variable cv;

  std::size_t max_size;
  std::queue<T> queue;
};

template <class T>
notify_lock_queue<T>::notify_lock_queue( std::size_t max_size )
  : max_size(max_size)
{
}

template <class T>
void notify_lock_queue<T>::push( T elm )
{
  std::unique_lock<std::mutex> lk( mtx );

  /* block a calling thread till the queue has a place to push */
  cv.wait( lk, [this] { return queue.size() != max_size; } );

  queue.push( std::move(elm) );

  lk.unlock();  /* it's desirable to unlock a mutex before make a notification */

  /* TODO: all threads waiting to pop and threads waiting to push get notified,
           it's not the best solution, 'cause after some element has been pushed
           we have to wake up only threads waiting to pop */
  cv.notify_all();
}

template <class T>
std::shared_ptr<T> notify_lock_queue<T>::pop()
{
  std::unique_lock<std::mutex> lk( mtx );

  /* block a calling thread till the queue has an element to pop */
  cv.wait( lk, [this] { return queue.size() != 0; } );

  /* TODO: check why a move ctor isn't called implicitly (for type which provides both copy and move ctors)? */
  auto tmp = std::make_shared<T>( std::move(queue.front()) ); /* due to recommendation about an exception-safe code */
  queue.pop();

  lk.unlock();  /* it's desirable to unlock a mutex before make a notification */
  cv.notify_all();

  return tmp;
}

#endif /* NOTIFY_LOCK_QUEUE_H */
