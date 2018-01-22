/*
 * log.cpp
 *
 *  Created on: Oct 2, 2017
 *      Author: sergs (ivan0ivanov0@mail.ru)
 */

#include <config.h>

#include <stdlib.h>

#include <iostream>
#include <fstream>
#include <string>

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>

#include <boost/core/null_deleter.hpp>

#include <boost/log/sinks.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>

namespace logging = boost::log;
namespace sinks = boost::log::sinks;

typedef sinks::synchronous_sink< sinks::text_ostream_backend > sink_t;

/* the log file within PWD */
static std::string log_file_name = "task_solver.log";

void init_boost_log( void )
{
  /* create and initiate stream based sink backend */
  boost::shared_ptr<sinks::text_ostream_backend> stream_log_backend =
      boost::make_shared<sinks::text_ostream_backend> ();
  const char* env_value;

  if( getenv ("LOG_TTY") )
    /* we can't allow to deallocate std::clog object :) */
    stream_log_backend->add_stream( boost::shared_ptr<std::ostream>( &std::clog, boost::null_deleter() ) );
  if( getenv ("LOG_FILE") )
    stream_log_backend->add_stream( boost::make_shared<std::fstream>( log_file_name ) );

  if( getenv ("LOG_DBG") )
    stream_log_backend->auto_flush( true );

  /* get reference to logging core */
  boost::shared_ptr<logging::core> log_core = logging::core::get();

  /* create sink frontend (linked with backend) and register it with logging core */
  boost::shared_ptr<sink_t> sink = boost::make_shared<sink_t>( stream_log_backend );
  log_core->add_sink( sink );

  /* set default severity to initiate logging */
  log_core->set_filter(  logging::trivial::severity >= logging::trivial::warning );

  /* set up global filter for logging core */
  if( (env_value = getenv("LOG_ERR_LVL")) && std::stoi( env_value ) == 1 )  /* std::string fears a nullptr ... */
    log_core->set_filter( logging::trivial::severity >= logging::trivial::error );
  if( (env_value = getenv("LOG_INF_LVL")) && std::stoi( env_value ) == 1 )
    log_core->set_filter( logging::trivial::severity >= logging::trivial::info );
  if( (env_value = getenv("LOG_DBG_LVL")) && std::stoi( env_value ) == 1 )
    log_core->set_filter( logging::trivial::severity >= logging::trivial::debug );

  if( getenv("ENABLE_LOG") )
    log_core->set_logging_enabled( true );
  else
    log_core->set_logging_enabled( false );
}
