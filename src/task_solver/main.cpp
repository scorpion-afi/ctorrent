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

#include "hash_calculator.h"
#include "log.h"

int main( void )
{
  std::string str;

  init_boost_log();

  std::getline( std::cin, str );

  BOOST_LOG_TRIVIAL( info );
  BOOST_LOG_TRIVIAL( info ) << "get started to calculate a hash for the string...";

  try
  {
    hash_calculator hash_calculator;

    BOOST_LOG_TRIVIAL( info );
    BOOST_LOG_TRIVIAL( info ) << "hash for the: \"" << str << "\": " << hash_calculator.get_hash( str ) << std::endl;
  }
  catch( const std::string &err )
  {
    BOOST_LOG_TRIVIAL( info ) << "an exception: " << err;
    return -1;
  }
  catch( const std::exception &exception )
  {
    BOOST_LOG_TRIVIAL( info ) << "an exception: " << exception.what();
  }
  catch(...)
  {
    BOOST_LOG_TRIVIAL( info ) << "some exception has been caught.";
  }
}
