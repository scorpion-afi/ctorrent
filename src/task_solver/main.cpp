/*
 * main.cpp
 *
 *  Created on: Oct 2, 2017
 *      Author: sergs (ivan0ivanov0@mail.ru)
 */

#include "config.h"

#include <iostream>
#include <string>
#include <exception>
#include <system_error>

#include "hash_calculator.h"
#include "string_invertor.h"
#include "log.h"

int main( void )
{
  try
  {
    std::string str;

    init_boost_log();

    std::getline( std::cin, str );

    BOOST_LOG_TRIVIAL( info );
    BOOST_LOG_TRIVIAL( info ) << "get started to perform some actions on the string...";
    BOOST_LOG_TRIVIAL( info );

    BOOST_LOG_TRIVIAL( info ) << "calculate a hash...";
    BOOST_LOG_TRIVIAL( info );

    uint64_t hash = hash_calculator().get_hash( str );

    BOOST_LOG_TRIVIAL( info );
    BOOST_LOG_TRIVIAL( info ) << "a hash for the: \"" << str << "\": " << hash;
    BOOST_LOG_TRIVIAL( info );


    BOOST_LOG_TRIVIAL( info ) << "calculate an inversion...";
    BOOST_LOG_TRIVIAL( info );

    std::string iverted_str = string_invertor().get_invert_str( str );

    BOOST_LOG_TRIVIAL( info );
    BOOST_LOG_TRIVIAL( info ) << "an inversion for the: \"" << str << "\": " << iverted_str;
  }
  catch( const std::system_error& ex )
  {
    BOOST_LOG_TRIVIAL( error );
    BOOST_LOG_TRIVIAL( error ) << ex.what() << " (" << ex.code().value() << ")";
  }
  catch( const std::exception& ex )
  {
    BOOST_LOG_TRIVIAL( error );
    BOOST_LOG_TRIVIAL( error ) << "an exception: " << ex.what();
  }
}
