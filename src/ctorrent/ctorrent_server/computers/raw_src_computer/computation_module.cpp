/*
 * computation_module.cpp
 *
 *  Created on: Jun 8, 2018
 *      Author: sergs
 */

#include "config.h"

#include <mutex> // for std::unique_lock/std::lock_guard

#include <boost/log/trivial.hpp>

#include "module_loader_base.h"
#include "module_compiler_base.h"
#include "regular_file.h"
#include "id_generator.h"

#include "computation_module.h"

#ifdef X86_64_BUILD
std::shared_mutex computation_module::mtx;
#endif
std::unordered_map<std::string, std::shared_ptr<computation_module>> computation_module::comp_modules;
const std::string computation_module::entry_point_name = "compute";

computation_module::computation_module( const std::string& source ) :
    dynamic_module(module_loader_base::get_dynamic_module())
{
  BOOST_LOG_TRIVIAL( debug ) << "computation_module: create a new computation module";

  const module_compiler_base& compiler = module_compiler_base::get_module_compiler();

  /* generate a unique name in a thread-safe manner */
  uint64_t prefix = id_generator::get_instance().get_id();
  std::string src_file_name = "./" + std::to_string( prefix ) + ".cpp";

  regular_file src_file( src_file_name );
  src_file << source;

  /* compile a dynamic module and load it to the process' address space */
  regular_file module_file = compiler.compile( src_file );
  dynamic_module->load( module_file.get_file_name() );

  func = reinterpret_cast<module_entry_point>( dynamic_module->get_symbol( entry_point_name ) );

  /* both files 'scr_file' and 'module_file' will be removed at this point,
   * but 'module_file' will be just removed from the file system till the
   * all references which point to 'module_file' not being closed (one reference
   * is a 'dynamic_module)*/
}

std::unique_ptr<const calc_result> computation_module::operator()( const calc_chunk& co ) const
{
  BOOST_LOG_TRIVIAL( debug ) << "computation_module: operator()";

  return func( co );
}

std::shared_ptr<computation_module> computation_module::get_computation_module( const std::string& source )
{
#ifdef X86_64_BUILD
  auto find = [] ( const std::string& source ) -> std::shared_ptr<computation_module>
  {
    auto it = comp_modules.find( source );
    if( it != comp_modules.end() )
      return it->second;
    else
      return nullptr;
  };

  auto shared_find = [&find] ( const std::string& source ) -> std::shared_ptr<computation_module>
  {
    std::shared_lock<std::shared_mutex> sh_lock( mtx );
    return find( source );
  };

  BOOST_LOG_TRIVIAL( debug ) << "computation_module: get_computation_module, look for existed computation module";

  /* at first perform a shared lookup for a computation module for sources @c source */
  auto comp_module = shared_find( source );
  if( comp_module )
    return comp_module;

  BOOST_LOG_TRIVIAL( debug ) << "computation_module: get_computation_module, computation module doesn't exist, "
      "lock and look up again";

  /* if nothing is found, create and add a new computation module for sources @c source */
  std::lock_guard<std::shared_mutex> ex_lock( mtx );

  /* but before, check if some other thread did it already (this find is performed under an exclusive lock) */
  comp_module = find( source );
  if( comp_module )
    return comp_module;

  BOOST_LOG_TRIVIAL( debug ) << "computation_module: get_computation_module, computation module doesn't exist, "
      "create a new one";

  /* computation_module has no a public ctor... */
  comp_module.reset( new computation_module(source) );
  comp_modules[source] = comp_module;

  /* TODO: think how to unload unused computation modules */

  return comp_module;
#endif
  return nullptr; // :)
}
