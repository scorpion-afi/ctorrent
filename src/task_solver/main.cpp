/*
 * main.cpp
 *
 *  Created on: Oct 2, 2017
 *      Author: sergs (ivan0ivanov0@mail.ru)
 */

#include "config.h"

#include <iostream>
#include <string>

#include "hash_calculator.h"
#include "log.h"

int main( void )
{
	hash_calculator hash_calculator;
  std::string str;

	init_boost_log();

	str = "that it's over for a while.";

  BOOST_LOG_TRIVIAL( info ) << "get started to calculate a hash for the string...";

  try
  {
    std::cout << "hash for the: \"" << str << "\": " << hash_calculator.get_hash( str ) << std::endl;
  }
  catch ( const char* err )
  {
    std::cout << err << std::endl;
    return -1;
  }

  BOOST_LOG_TRIVIAL( info ) << "the has got calculated.";
}
