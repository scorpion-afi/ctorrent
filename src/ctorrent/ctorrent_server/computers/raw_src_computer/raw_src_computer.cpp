/*
 * raw_src_computer.cpp
 *
 *  Created on: May 31, 2018
 *      Author: sergs
 */

#include "config.h"

#include <boost/log/trivial.hpp>

#include "computation_module.h"

#include "raw_src_computer.h"

raw_src_computer::raw_src_computer()
{
  BOOST_LOG_TRIVIAL( debug ) << "raw_src_computer::register a raw_src_computer computer";
}

std::unique_ptr<const base_calc_result> raw_src_computer::compute( const base_calc& obj ) const
{
  BOOST_LOG_TRIVIAL( debug ) << "raw_src_computer: compute a task with an object: " << &obj;

  /* it's a safe operation, 'cause we have registered this computer with a calc_chunk type, so
   * there's no reason to perform a dynamic cast */
  const calc_chunk& task = static_cast<const calc_chunk&>( obj );

  std::shared_ptr<computation_module> comp_module = computation_module::get_computation_module( task.task_src );

  return (*comp_module)( task );
}
