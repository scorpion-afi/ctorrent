/*
 * module_loader_base.h
 *
 *  Created on: Jun 8, 2018
 *      Author: sergs
 */

#ifndef MODULE_LOADER_BASE_H
#define MODULE_LOADER_BASE_H

#include <string>
#include <memory>

#include "object.h"

/* this abstract class provides API for the dynamic module management;
 * dynamic module can be:
 *  - loaded
 *  - unloaded
 *  - used to find some symbol
 * thread-safe
 */
class module_loader_base : public object
{
public:
  module_loader_base() = default;
  virtual ~module_loader_base() = default;

  /* no trivial copy semantic, so just disable it :) */
  module_loader_base( const module_loader_base& that ) = delete;
  module_loader_base& operator=( const module_loader_base& that ) = delete;

  /* if we have an explicitly defaulted dtor we have NO an implicitly declared move semantic */
  module_loader_base( module_loader_base&& that ) = default;
  module_loader_base& operator=( module_loader_base&& that ) = default;

  virtual void load( const std::string& module_name ) = 0;
  virtual void unload() = 0;
  virtual void* get_symbol( const std::string& symbol_name ) const = 0;

  /* a factory method to produce a 'module_loader' objects depend on the current platform */
  static std::unique_ptr<module_loader_base> get_dynamic_module();
};

#endif /* MODULE_LOADER_BASE_H */
