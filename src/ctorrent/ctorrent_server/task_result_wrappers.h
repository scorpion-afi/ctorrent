/*
 * task_result_wrappers.h
 *
 *  Created on: Jun 7, 2018
 *      Author: sergs
 */

#ifndef TASK_RESULT_WRAPPERS_H
#define TASK_RESULT_WRAPPERS_H

#include <memory>

#include <boost/log/trivial.hpp>

#include "ctorrent_protocols.h"
#include "./computers/base_computer.h"
#include "object.h"

class remote_client;

/* it's a wrapper over a base_calc_result object
 *  -base_calc_result itself doesn't refer to a remote_client it has to be sent to */
class result : public object
{
public:
  result( std::weak_ptr<remote_client> cl,
                    std::shared_ptr<const base_calc_result> result ) :
                      cl(cl), res(std::move(result)) {}

  std::weak_ptr<remote_client> get_client() const { return cl; }
  std::shared_ptr<const base_calc_result> get_pkg() && { return std::move(res); }

private:
  std::weak_ptr<remote_client> cl;
  std::shared_ptr<const base_calc_result> res;
};

/* it's a wrapper over a base_calc object;
 *  -base_calc itself doesn't contain enough information to be computed,
 *  -base_calc itself doesn't refer to a remote_client it belongs to */
class task : public object
{
public:
  task( std::weak_ptr<remote_client> cl, std::shared_ptr<const base_calc> task ) :
                      cl(cl), tsk(std::move(task)),
                      computer(base_computer::get_computer(tsk->get_comp_type()))
  {
    BOOST_LOG_TRIVIAL( debug ) << "task wrapper: create a task: " << get_id() << " from an object: " << tsk.get();
  }

  /* compute a task from an underlying base_calc object */
  result compute() const
  {
    BOOST_LOG_TRIVIAL( debug ) << "task wrapper: compute a task: " << get_id() << " for an object: " << tsk.get();

    auto res = computer->compute( *tsk );
    return result( cl, std::move(res) );
  }

private:
  std::weak_ptr<remote_client> cl;
  std::shared_ptr<const base_calc> tsk;
  std::shared_ptr<base_computer> computer;
};

#endif /* TASK_RESULT_WRAPPERS_H */
