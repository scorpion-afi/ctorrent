/*
 * remote_client.h
 *
 *  Created on: Feb 21, 2018
 *      Author: sergs
 */

#ifndef REMOTE_CLIENT_H
#define REMOTE_CLIENT_H

#include <memory>
#include <string>
#include <atomic>

#include "ctorrent_protocols.h"
#include "notify_lock_queue.h"
#include "remote_connection.h"
#include "task_result_wrappers.h"

/* this class describes connection to a remote client;
 * both receiving and sending threads may use objects of this type concurrently without
 * any synchronization;
 * remote_client has enough information to perform an automatic flushing */
class remote_client : public remote_connection
{
public:
  remote_client( int socket_fd, notify_lock_queue<task>& task_queue, std::string identify_str );

  remote_client( const remote_client& that ) = delete;
  remote_client& operator=( const remote_client& that ) = delete;

  remote_client( remote_client&& that ) = default;
  remote_client& operator=( remote_client&& that ) = default;

  /* this function gets called if some objects were deserialized */
  void process_deserialized_objs( deserialized_objs objs ) override;

  /* set up a weak self reference to be used for internal purposes */
  void set_self_reference( std::weak_ptr<remote_client> self_ref ) { this->self_ref = self_ref; }

  /* shadows the based's implementation to provide an automatically flushing behavior */
  void serialize_and_send( const base_serialize* obj );

private:
  notify_lock_queue<task>& tasks_queue;  /* a REFERENCE */

  /* a weak self reference which is used to provide weak references to this object from
   * task_wrapper objects */
  std::weak_ptr<remote_client> self_ref;

  /* The next two counters are used to make explicit flushing for serialized objects
   * when an amount of serialized objects gets amount of received objects to prevent
   * a possible deadlock when a remote client waits for all objects and the server
   * waits for the new objects from the client before it makes an implicit flushing */

  /* used by both receiving and sending threads, so has to be protected against races */
  std::atomic_ullong received_objs_cnt;
  unsigned long long serialized_objs_cnt;
};

#endif /* REMOTE_CLIENT_H */
