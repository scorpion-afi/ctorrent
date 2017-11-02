/*
 * ctorrent.cpp
 *
 *  Created on: Oct 4, 2017
 *      Author: sergs (ivan0ivanov0@mail.ru)
 */

#include "config.h"

#include <boost/log/trivial.hpp>

#include "ctorrent.h"

int ctorrent::send( const std::vector<calc_chunk> &calc_chunks )
{
  for( auto &i : calc_chunks )
    BOOST_LOG_TRIVIAL( info ) << i.get_info() << "\n\n";

  return 0;
}

int ctorrent::receive( std::vector<calc_chunk> &calc_chunks )
{
  return 0;
}
