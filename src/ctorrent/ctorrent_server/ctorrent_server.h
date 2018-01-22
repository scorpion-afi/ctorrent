/*
 * ctorrent_server.h
 *
 *  Created on: Jan 25, 2018
 *      Author: sergs (ivan0ivanov0@mail.ru)
 */

#ifndef CTORRENT_SERVER_H
#define CTORRENT_SERVER_H

#include <list>
#include <memory>
#include <string>
#include <thread>

#include "ctorrent.h"
#include "remote_client.h"
#include "notify_lock_queue.h"

class ctorrent_server : public ctorrent
{
public:
  ctorrent_server();
  ~ctorrent_server();

  /* TODO: think how to work around version */
  long get_version() const override { return 1; }

  /* returns fd to build in the app's main loop;
   * should be closed after it's not necessary */
  int get_fd() const;

  /* has to be called as fd, returned by 'get_fd', gets active
   * to handle library events */
  void handle_events();

private:
  in_addr get_interface_ipv4_address( const std::string& if_name );
  void new_client_handler( int listen_socket, void* data );

  using destroy_cl_tuple = std::tuple<std::shared_ptr<remote_client>, uint64_t, uint64_t>;

  void destroy_client_handler( int listen_socket, void* data, std::shared_ptr<destroy_cl_tuple> tuple );
  void start_threads();

private:
  const std::string interface_name = "eth0";  /* TODO: think how can it be automated */
  std::shared_ptr<std::string> interface_ip_text;
  in_addr interface_ip;

  epoll_event_loop epoll;

  /* list of connected remote clients */
  std::list<std::shared_ptr<remote_client>> client_list;

  static const std::size_t max_queue_size = 100;
  static const std::size_t min_executor_threads_amount = 2;

  notify_lock_queue<task_wrapper> tasks_queue;      /* a queue to hold clients' tasks after receiving/deserialization */
  notify_lock_queue<result_wrapper> results_queue;  /* a queue to hold results of clients' tasks after execution */

  std::vector<std::thread> executor_thread_pool;  /* a pool of threads which execute clients' tasks */
  std::thread sending_thread; /* a thread responsible for sending results and errors to clients */
};

#endif /* CTORRENT_SERVER_H */
