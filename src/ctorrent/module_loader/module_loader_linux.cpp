/*
 * module_loader_linux.cpp
 *
 *  Created on: Jun 8, 2018
 *      Author: sergs
 */

#include "module_loader_linux.h"

#include "config.h"

#include <dlfcn.h>
#include <sstream>


module_loader_linux::module_loader_linux() :
  module_hndl(nullptr)
{
}

module_loader_linux::~module_loader_linux()
{
  unload();
}

void module_loader_linux::load( const std::string& module_name )
{
  if( module_hndl )
    return;

  module_hndl = dlopen( module_name.c_str(), RTLD_LAZY );

  if( !module_hndl )
  {
    std::stringstream err_stream;

    err_stream << "fail to load a '" << module_name << "' module: " << dlerror() << ".";
    throw std::string( err_stream.str() );
  }
}

void module_loader_linux::unload()
{
  if( !module_hndl )
    return;

  dlclose( module_hndl );
  module_hndl = nullptr;
}

void* module_loader_linux::get_symbol( const std::string& symbol_name ) const
{
  const char* dl_err_str;
  void* symbol;

  if( !module_hndl )
    throw std::string( "no managed dynamic module" );

  dlerror();  /* reset previous errors if any */

  symbol = dlsym( module_hndl, symbol_name.c_str() );

  dl_err_str = dlerror();
  if( dl_err_str )
  {
    std::stringstream err_stream;

    err_stream << "fail to locale a '" << symbol_name << "' symbol: " << dl_err_str << ", symbol: "
        << symbol << ".";
    throw std::string( err_stream.str() );
  }

  return symbol;
}
