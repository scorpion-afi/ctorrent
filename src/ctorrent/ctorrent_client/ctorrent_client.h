/*
 * ctorrent_client.h
 *
 *  Created on: Jan 25, 2018
 *      Author: sergs (ivan0ivanov0@mail.ru)
 */

#ifndef CTORRENT_CLIENT_H
#define CTORRENT_CLIENT_H

#include <vector>
#include <memory>
#include <list>

#include "remote_server.h"
#include "ctorrent.h"

/* public API to distribute the calculation between eXecutian and Dispatching Units.
 *
 * this class represent a client - that who makes a calculation request.
 *
 * it's up to an user of this class to provide tasks to calculate (e.g. divide some big task
 * into several small sub-tasks). After tasks are provided to this class by send() method
 * it's a class responsibility to send them to remote XDUs, collect responds and provide them
 * to the caller via receive() method.
 *
 * doens't support a copy semantic.
 *
 * TODO: isn't thread-safe (can't be used for sending/receiving in several threads)
 */
class ctorrent_client : public ctorrent
{
public:
  /* TODO: should we expose base_serialize or not? */
  using results_t = remote_server::deserialized_objs_t;

  ctorrent_client();
  ~ctorrent_client();

  ctorrent_client( const ctorrent_client& that ) = delete;
  ctorrent_client& operator=( const ctorrent_client& that ) = delete;

  ctorrent_client( ctorrent_client&& that ) = default;
  ctorrent_client& operator=( ctorrent_client&& that ) = default;

  /* protocol v0.1 */

  /* will return immediately, actual sending may be delayed by internal reasons;
   * an exception is thrown in a case of an error;
   * if an order of response objects is important 'is_order_important' has to be true (this may impact a performance,
   * 'cause this prevents the library from returning response objects immediately as they've came from XDUs); */
  void send( const std::vector<std::shared_ptr<base_calc>>& objs, bool is_order_important = true );

  /* returns fd to build in the app's main loop;
   * should be closed after it's not necessary */
  int get_fd() const { return epoll.get_fd(); /* ctorrent_client doesn't own an epoll's fd, so there's no reason to make a duplicate */ }

  /* will block till at least one item can be returned;
   * an exception is thrown in a case of an error;
   * a caller has to call this function till it gets all items (like with a read() syscall);
   * the library guarantees that all received objects are at least a base_calc_result based */
  results_t receive();

private:
  std::list<in_addr> get_servers_list();

private:
  std::list<std::shared_ptr<remote_server>> remote_servers_list; /* list of servers we've connected to */
  bool is_obj_order_important;
  epoll_event_loop epoll;
  uint64_t current_seq_id;

  remote_server::deserialized_objs_t received_objects;
};

#endif /* CTORRENT_CLIENT_H */
