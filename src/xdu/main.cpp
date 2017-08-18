
#include <config.h>

#include <iostream>

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>

#include <boost/log/sinks.hpp>
#include <boost/log/sinks/sink.hpp>
#include <boost/log/sinks/syslog_backend.hpp>

namespace logging = boost::log;
namespace sinks = logging::sinks;

// Complete sink type
typedef sinks::synchronous_sink< sinks::syslog_backend > sink_t;

void init_native_syslog()
{
    boost::shared_ptr< logging::core > core = logging::core::get();

    // Create a backend
    boost::shared_ptr< logging::sinks::syslog_backend > backend(new sinks::syslog_backend(
    		logging::keywords::facility = logging::sinks::syslog::user,
			logging::keywords::use_impl = logging::sinks::syslog::udp_socket_based
    ));

    // Set the straightforward level translator for the "Severity" attribute of type int
    backend->set_severity_mapper(sinks::syslog::direct_severity_mapping< int >("Severity"));

    // Wrap it into the frontend and register in the core.
    // The backend requires synchronization in the frontend.
    core->add_sink(boost::make_shared< sink_t >(backend));
    
    core->set_filter
    (
        logging::trivial::severity >= logging::trivial::info
    );
}

int main( void )
{
	std::cout << "wassup!\n";

	init_native_syslog();
	
	BOOST_LOG_TRIVIAL( info )  << "lemme know when u get ready.";
}
