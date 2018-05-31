/*
 * task_distributer.h
 *
 *  Created on: May 30, 2018
 *      Author: sergs
 */

#ifndef CLIENT_TASK_DISTRIBUTER_H
#define CLIENT_TASK_DISTRIBUTER_H

#include <list>
#include <vector>
#include <memory>

class remote_server;
class base_calc;

/* this abstract class provides an interface for the tasks distribution mechanism
 * on the client side */
class task_distributer
{
public:
  task_distributer() = default;
  virtual ~task_distributer() = default;

  /*
   * @brief an abstract method for objects distribution
   *
   * @param [in] server_list - list of servers objects @c objs should be distributed between
   * @param [in] objs - objects to be distributed between remote servers @c server_list
   *
   * @note @c server_list and @c objs contain at least ONE object */
  virtual void distribute( const std::list<std::shared_ptr<remote_server>>& server_list,
                           const std::vector<std::shared_ptr<base_calc>>& objs ) = 0;

  /* if we have an explicitly defaulted dtor we have NO an implicitly declared move semantic */
  task_distributer( const task_distributer& that ) = default;
  task_distributer& operator=( const task_distributer& that ) = default;

  task_distributer( task_distributer&& that ) = default;
  task_distributer& operator=( task_distributer&& that ) = default;
};

/* the factory method to get a concrete tasks' distributer */
std::unique_ptr<task_distributer> make_task_distributer();

#endif /* CLIENT_TASK_DISTRIBUTER_H */
