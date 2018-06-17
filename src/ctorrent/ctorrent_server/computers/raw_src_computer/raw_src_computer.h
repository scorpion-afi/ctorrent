/*
 * raw_src_computer.h
 *
 *  Created on: May 31, 2018
 *      Author: sergs
 */

#ifndef RAW_SRC_COMPUTER_H
#define RAW_SRC_COMPUTER_H

#include "../base_computer.h"

/* this computer deals with plain sources passed within a task object,
 * that sources get compiled, result dynamic module gets loaded and an entry
 * point of this module is used for actual computation;
 * thread-safe */
class raw_src_computer : public base_computer
{
public:
  raw_src_computer(); /* just for logging purposes */

  std::unique_ptr<const base_calc_result> compute( const base_calc& task ) const override;
};

#endif /* RAW_SRC_COMPUTER_H */
