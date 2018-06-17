/*
 * module_compiler_base.cpp
 *
 *  Created on: Jun 8, 2018
 *      Author: sergs
 */

#include "config.h"

#if defined(X86_64_BUILD) || defined(ANDROID_BUILD)
#include "module_compiler_linux.h"
#endif

#include "module_compiler_base.h"

const module_compiler_base& module_compiler_base::get_module_compiler()
{
#if defined(X86_64_BUILD) || defined(ANDROID_BUILD)
  static module_compiler_linux compiler;
#endif

  return compiler;
}
