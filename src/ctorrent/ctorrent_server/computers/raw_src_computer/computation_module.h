/*
 * computation_module.h
 *
 *  Created on: Jun 8, 2018
 *      Author: sergs
 */

#ifndef COMPUTATION_MODULE_H
#define COMPUTATION_MODULE_H

#include <memory>
#include <string>
#include <unordered_map>
#include <shared_mutex>

#include "ctorrent_protocols.h"
#include "object.h"

class module_loader_base;

/* this class describes a computation module associated with a dynamic module compiled
 * from sources passed as a parameter to ctor;
 * a computation module can be shared between several executor_threads which execute task with
 * the same sources;
 * thread-safe */
class computation_module : public object
{
public:
  /* perform a computation using an underlying dynamic module */
  std::unique_ptr<const calc_result> operator()( const calc_chunk& co ) const;

  /* a factory method to produce a 'computation_module' objects depend on the @c source source;
   * for the same source return a pointer to the same computation module */
  static std::shared_ptr<computation_module> get_computation_module( const std::string& source );

private:
  computation_module( const std::string& source );

  /* API of a module's entry point */
  using module_entry_point = std::unique_ptr<const calc_result> (*)( const calc_chunk& );
  static const std::string entry_point_name;

  /* an underlying dynamic module which provides an entry point */
  const std::unique_ptr<module_loader_base> dynamic_module;
  module_entry_point func;

  /* TODO: currently I use arm-android-gcc-5.3 which doesn't support std::shared_mutex as a read-write lock */
#ifdef X86_64_BUILD
  static std::shared_mutex mtx;
#endif
  static std::unordered_map<std::string, std::shared_ptr<computation_module>> comp_modules;
};

#endif /* COMPUTATION_MODULE_H */
