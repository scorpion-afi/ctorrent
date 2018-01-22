/*
 * ctorrent_protocols.cpp
 *
 *  Created on: Oct 5, 2017
 *      Author: sergs (ivan0ivanov0@mail.ru)
 */

#include "config.h"

#include <vector>
#include <sstream>
#include <fstream>
#include <cstring>

#include <unistd.h>
#include <dlfcn.h>
#include <sys/wait.h>

#include <boost/log/trivial.hpp>

#include "ctorrent_protocols.h"


class compute_module
{
public:
  ~compute_module();

  /* no copy semantic */
  compute_module( const compute_module &that ) = delete;
  compute_module& operator=( const compute_module& that ) = delete;

  compute_module( compute_module &&that );
  compute_module& operator=( compute_module that );

  friend void swap( compute_module &first, compute_module &second )
  {
    using std::swap;

    swap( first.module_src,    second.module_src );
    swap( first.module_hndl,   second.module_hndl );
    swap( first.module_symbol, second.module_symbol );
  }

  static compute_module& get_compute_module( const std::string &task_source );
  std::shared_ptr<base_calc_result> operator()( const calc_chunk& co, const void* data ) const;

private:
  compute_module();
  compute_module( const std::string &task_src );

  void create_sources();
  void compile_module( );
  void load_module();
  void get_module_symbol();

  const std::string file_name   = "module.cpp";
  const std::string module_name = "libmodule.so";
  const std::string symbol_name = "compute";

  std::string module_src;
  void* module_hndl;
  void* module_symbol;

  static std::vector<compute_module> loaded_modules;
};

/* TODO: [memory] how many modules we have to supply at one moment? */
std::vector<compute_module> compute_module::loaded_modules;


compute_module::compute_module() : module_hndl(nullptr), module_symbol(nullptr)
{
}

compute_module::compute_module( const std::string &task_src ) : compute_module()
{
  module_src = task_src;

  create_sources();
  compile_module();
  load_module();

  get_module_symbol();

  BOOST_LOG_TRIVIAL( debug ) << "[debug] a new compute module has been loaded.";
}

compute_module::~compute_module()
{
  if( module_hndl )
    dlclose( module_hndl );
}

compute_module::compute_module( compute_module &&that ) : compute_module()
{
  swap( *this, that );
}

compute_module& compute_module::operator=( compute_module that )
{
  swap( *this, that );

  return *this;
}

void compute_module::create_sources()
{
  /* it's very hard to pass an O_EXCL flag, so just remove the file unconditionally... */
  std::remove( file_name.c_str() );

  std::fstream file( file_name, std::ios_base::out );

  if( !file.is_open() )
    throw std::string( "file can't be created." );

  file << module_src;
}

void compute_module::compile_module()
{
  pid_t child_pid;

  child_pid = fork();
  if( child_pid < 0 )
    throw std::string( "a fail while 'fork' syscall." );

  if( !child_pid )
  {
    /* to look for ld, assembler,... */
    const char* env_vars[] = { "PATH=/usr/bin", nullptr };

    /* forcely update a dynamic loadable module */
    std::remove( module_name.c_str() );

    execle( "/usr/bin/g++", "/usr/bin/g++", "--shared", "-o", module_name.c_str(), file_name.c_str(),
            "-fpic", "-fPIC", "-g3", "-O0", (char*)NULL, env_vars );

    std::stringstream err_stream;

    err_stream << "CHILD: execle couldn't launch compiler, err: " << errno << ".";
    throw std::string( err_stream.str() );
  }

  int child_status;
  int ret;

  /* wait till compiler makes its work */

  ret = waitpid( child_pid, &child_status, 0 );
  if( ret < 0)
    throw std::string( "a fail while 'waitpid' syscall." );

  ret = WIFEXITED(child_status);
  if( ret )
  {
    if( WEXITSTATUS(child_status) != EXIT_SUCCESS)
      throw std::string( "a complier process returned back an error." );
  }
  else
    throw std::string( "a compiler process has stopped abnormally." );
}

void compute_module::load_module()
{
  module_hndl = dlopen( ("./" + module_name).c_str(), RTLD_LAZY );

  if( !module_hndl )
  {
    std::stringstream err_stream;

    err_stream << "fail to load calculation module: " << errno << ".";
    throw std::string( err_stream.str() );
  }
}

void compute_module::get_module_symbol()
{
  const char *dl_err_str;

  dlerror();  /* reset previous errors if any */

  module_symbol = dlsym( module_hndl, symbol_name.c_str() );

  dl_err_str = dlerror();
  if( dl_err_str )
  {
    std::stringstream err_stream;

    err_stream << "fail to locale '" << symbol_name << "' symbol: " << dl_err_str << ", symbol: "
        << module_symbol << ".";
    throw std::string( err_stream.str() );
  }
}

/* TODO: compute_module supports move-semantic, so it can be stolen from loaded_modules vector */
compute_module& compute_module::get_compute_module( const std::string &task_source )
{
  for( auto& module : compute_module::loaded_modules )
    if( module.module_src == task_source )
      return module;

  compute_module module( task_source );

  compute_module::loaded_modules.push_back( std::move(module) );

  return compute_module::loaded_modules.back();
}

std::shared_ptr<base_calc_result> compute_module::operator()( const calc_chunk& co, const void* data ) const
{
  typedef std::shared_ptr<base_calc_result> (*compute_fn)(const calc_chunk& co, const void*);

  BOOST_LOG_TRIVIAL( debug ) << "[debug] use the compute module.";

  return reinterpret_cast<compute_fn>(module_symbol)( co, data );
}

calc_result::calc_result() : data(nullptr), data_size(0),
    calc_result_id(0) /* by default initialize by an invalid value */
{
}

calc_result::calc_result( const calc_chunk& calc_obj ) : calc_result()
{
  calc_result_id = calc_obj.get_calc_chunk_id();
}

calc_result::~calc_result()
{
  delete [] data;
}


calc_chunk::calc_chunk() : calc_chunk_id(0), /* by default initialize by an invalid value */
    m_data(nullptr), m_data_size(0)
{
}

calc_chunk::~calc_chunk()
{
  delete [] m_data;
}

calc_chunk::calc_chunk( calc_chunk &&that ) : calc_chunk()
{
  swap( *this, that );
}

calc_chunk& calc_chunk::operator=( calc_chunk that )
{
  swap( *this, that );

  return *this;
}

void swap( calc_chunk &first, calc_chunk &second )
{
  using std::swap;

  swap( first.m_data, second.m_data );
  swap( first.m_data_size, second.m_data_size );
  swap( first.m_method_src, second.m_method_src );
}

void calc_chunk::grab_data( const i_chunk_data &data )
{
  m_data_size = data.get_raw_data_size();

  m_data = new char[m_data_size];
  std::memcpy( m_data, data.get_raw_data(), m_data_size );
}

void calc_chunk::set_method_src( std::string method_src )
{
  m_method_src = std::move(method_src);
}

std::shared_ptr<base_calc_result> calc_chunk::compute()
{
  compute_module& module = compute_module::get_compute_module( m_method_src );

  return module( *this, m_data );
}

std::string calc_chunk::get_info() const
{
  std::stringstream str;

  str << "m_data: " << m_data << ", m_data_size: " << m_data_size << "\nm_method_src:\n"
      << m_method_src;

  return str.str();
}

