/*
 * module_compiler_linux.cpp
 *
 *  Created on: Jun 8, 2018
 *      Author: sergs
 */

#include "config.h"

#include <sstream>
#include <cstdlib>

#include <boost/log/trivial.hpp>

#include <unistd.h>
#include <sys/wait.h>

#include "id_generator.h"

#include "module_compiler_linux.h"

regular_file module_compiler_linux::compile( const regular_file& src_file ) const
{
  uint64_t prefix = id_generator::get_instance().get_id();
  std::string module_file_name = "./" + std::to_string( prefix ) + ".so"; // should be an absolute/relative path
  pid_t child_pid;

  /* object will be created by an external compiler (gcc), so we need just to adopt */
  regular_file module_file( module_file_name, true );

  /* TODO: to guarantee a consistent of memory covered by mutexs owned by this library,
   *       only async-signal-safe calls may be performed between fork() and execle(),
   *       but what about std::string() is it a such one? */
  child_pid = fork();
  if( child_pid < 0 )
    throw std::system_error( errno, std::system_category(), "fork" );

  if( !child_pid )
  {
    /* to look for ld, assembler,... */
    const char* env_vars[] = { "PATH=/usr/bin", nullptr };

    execle( "/usr/bin/g++", "/usr/bin/g++", "--shared", "-o", module_file.get_file_name().c_str(),
            src_file.get_file_name().c_str(), "-fpic", "-fPIC", "-g3", "-O0", (char*)NULL, env_vars );

    throw std::system_error( errno, std::system_category(), "execle couldn't launch a compiler" );
  }

  int child_status;
  int ret;

  /* wait till the compiler makes its work */

  ret = waitpid( child_pid, &child_status, 0 );
  if( ret < 0)
    throw std::system_error( errno, std::system_category(), "waitpid" );

  ret = WIFEXITED(child_status);
  if( ret )
  {
    if( WEXITSTATUS(child_status) != EXIT_SUCCESS)
      throw std::runtime_error( "a complier process returned back an error" );
  }
  else
    throw std::runtime_error( "a compiler process has stopped abnormally." );

  return module_file;
}
