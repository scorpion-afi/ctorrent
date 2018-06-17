/*
 * module_loader_linux.h
 *
 *  Created on: Jun 8, 2018
 *      Author: sergs
 */

#ifndef MODULE_LOADER_LINUX_H
#define MODULE_LOADER_LINUX_H

#include "module_loader_base.h"

/* manages a Linux dynamic module (a .so library);
 * isn't thread-safe */
class module_loader_linux : public module_loader_base
{
public:
  module_loader_linux();
  ~module_loader_linux() override;

  /* no trivial copy semantic, so just disable it :) */
  module_loader_linux( const module_loader_linux& that ) = delete;
  module_loader_linux& operator=( const module_loader_linux& that ) = delete;

  /* if we have an explicitly defaulted dtor we have NO an implicitly declared move semantic */
  module_loader_linux( module_loader_linux&& that ) = default;
  module_loader_linux& operator=( module_loader_linux&& that ) = default;

  /* load a module to the process's address space;
   * @c module_name is an absolute path to a module file;
   * Linux specific: look at dlopen() man to get what types of
   * file paths are possible to use */
  void load( const std::string& module_name ) override;

  /* unload a managed module */
  void unload() override;

  /* find and return a specified symbol (@c symbol_name) in the managed module;
   * may return a nullptr which means that a required symbol doesn't exist (it's not an error);
   * in case of errors, an exception gets thrown */
  void* get_symbol( const std::string& symbol_name ) const override;

private:
  void* module_hndl;
};

#endif /* MODULE_LOADER_LINUX_H */
