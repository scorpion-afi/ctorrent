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

/* this class implements a thread-safe queue which blocks
 * calling threads (both who try to push and pop )
 * till there's place to push to and something to pop.
 *
 * doesn't support a move semantic.
 */
template <class T>
class notify_lock_queue
{
public:
  explicit notify_lock_queue( std::size_t max_size );

  notify_lock_queue( const notify_lock_queue<T>& other );
  notify_lock_queue<T> operator=( const notify_lock_queue<T>& other );

  /* if we had a move semantic provided it might lead to a circumstances
   * when an awaken thread accesses a moved object, so we disable such a semantic */
  notify_lock_queue( notify_lock_queue<T>&& other ) = delete;
  notify_lock_queue<T> operator=( notify_lock_queue<T>&& other ) = delete;

  /* TODO: make a swap when it's needed, think about cv.notify about swapping */
  template <class U>
  friend void swap( notify_lock_queue<U>& lhs, notify_lock_queue<U>& rhs );

  void push( T elm );

  std::unique_ptr<T> pop();

private:
  mutable std::mutex mtx;
  std::condition_variable cv;

  std::size_t max_size;
  std::queue<T> queue;
};

template <class U>
void swap( notify_lock_queue<U>& lhs, notify_lock_queue<U>& rhs ) = delete;

template <class T>
notify_lock_queue<T>::notify_lock_queue( std::size_t max_size )
  : max_size(max_size)
{
}

template <class T>
notify_lock_queue<T>::notify_lock_queue( const notify_lock_queue<T>& other )
{
  std::lock_guard<std::mutex> lk( other.mtx );

  max_size = other.max_size;
  queue = other.queue;
}

template <class T>
notify_lock_queue<T> notify_lock_queue<T>::operator=( const notify_lock_queue<T>& other )
{
  if( this == &other )
    return *this;

  std::unique_lock<std::mutex> lk_lhs( mtx, std::defer_lock );
  std::unique_lock<std::mutex> lk_rhs( other.mtx, std::defer_lock );

  std::lock( lk_lhs, lk_rhs );

  max_size = other.max_size;
  queue = other.queue;

  lk_lhs.unlock(); /* it's desirable to unlock a mutex before make a notification */

  cv.notify_all();

  return *this;
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
std::unique_ptr<T> notify_lock_queue<T>::pop()
{
  std::unique_lock<std::mutex> lk( mtx );

  /* block a calling thread till the queue has an element to pop */
  cv.wait( lk, [this] { return queue.size() != 0; } );

  /* TODO: check why a move ctor isn't called implicitly (for type which provides both copy and move ctors)? */
  auto tmp = std::make_unique<T>( std::move(queue.front()) ); /* due to recommendation about an exception-safe code */
  queue.pop();

  lk.unlock();  /* it's desirable to unlock a mutex before make a notification */
  cv.notify_all();

  return tmp;
}

#endif /* NOTIFY_LOCK_QUEUE_H */
