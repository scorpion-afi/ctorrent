/*
 * module_compiler_linux.h
 *
 *  Created on: Jun 8, 2018
 *      Author: sergs
 */

#ifndef MODULE_COMPILER_LINUX_H
#define MODULE_COMPILER_LINUX_H

#include "module_compiler_base.h"

/* a module (.so library) compiler for Linux;
 * thread-safe */
class module_compiler_linux : public module_compiler_base
{
public:
  /* compile a dynamic module from the @c src_file source file;
   * may block till finish compilation;
   *
   * @c src_file - is a file with the code to compile
   * return a regular_file which represents a dynamic module;
   *
   * throw an exception in case of an error */
  regular_file compile( const regular_file& src_file ) const override;
};

#endif /* MODULE_COMPILER_LINUX_H */
