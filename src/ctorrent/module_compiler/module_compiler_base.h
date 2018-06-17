/*
 * module_compiler_base.h
 *
 *  Created on: Jun 8, 2018
 *      Author: sergs
 */

#ifndef MODULE_COMPILER_BASE_H
#define MODULE_COMPILER_BASE_H

#include "regular_file.h"
#include "object.h"

/* this abstract class provides API for the dynamic module compilation;
 * thread-safe */
class module_compiler_base : public object
{
public:
  module_compiler_base() = default;
  virtual ~module_compiler_base() = default;

  module_compiler_base( const module_compiler_base& that ) = default;
  module_compiler_base& operator=( const module_compiler_base& that ) = default;

  /* if we have an explicitly defaulted dtor we have NO an implicitly declared move semantic */
  module_compiler_base( module_compiler_base&& that ) = default;
  module_compiler_base& operator=( module_compiler_base&& that ) = default;

  /* compile a dynamic module from the @c src_file source file;
   * may block till finish compilation;
   *
   * @c src_file - is a file with the code to compile
   * return a regular_file which represents a dynamic module;
   *
   * throw an exception in case of an error */
  virtual regular_file compile( const regular_file& src_file ) const = 0;

  /* a factory method to produce a 'module_compiler' objects depend on the current platform */
  static const module_compiler_base& get_module_compiler();
};

#endif /* MODULE_COMPILER_BASE_H */
