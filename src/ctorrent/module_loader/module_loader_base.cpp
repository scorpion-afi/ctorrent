/*
 * module_loader_base.cpp
 *
 *  Created on: Jun 8, 2018
 *      Author: sergs
 */

#include "config.h"

#if defined(X86_64_BUILD) || defined(ANDROID_BUILD)
#include "module_loader_linux.h"
#endif

#include "module_loader_base.h"

std::unique_ptr<module_loader_base> module_loader_base::get_dynamic_module()
{
  std::unique_ptr<module_loader_base> ret;

#if defined(X86_64_BUILD) || defined(ANDROID_BUILD)
  ret.reset( new module_loader_linux );
#endif

  return ret;
}
