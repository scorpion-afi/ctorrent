/*
 * plain_task_distributer.h
 *
 *  Created on: May 30, 2018
 *      Author: sergs
 */

#ifndef CLIENT_PLAIN_TASK_DISTRIBUTER_H
#define CLIENT_PLAIN_TASK_DISTRIBUTER_H

#include "task_distributer.h"

/* an easiest implementation of a task_distributer interface;
 * objects are distributed equally between available servers */
class plain_task_distributer : public task_distributer
{
public:
  void distribute( const std::list<std::shared_ptr<remote_server>>& server_list,
                             const std::vector<std::shared_ptr<base_calc>>& objs ) override;
};

#endif /* CLIENT_PLAIN_TASK_DISTRIBUTER_H */
