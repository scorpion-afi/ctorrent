/*
 * regular_file.cpp
 *
 *  Created on: Jun 16, 2018
 *      Author: sergs
 */

#include "config.h"

#include <fstream>
#include <cstdio>

#include "regular_file.h"

regular_file::regular_file( const std::string& file_name, bool adopt, std::ios::openmode mode ) :
  file_name(file_name)
{
  if( file_name.empty() )
    throw std::string( "regular_file: an empty file_name" );

  /* an OS-independent way to create a file */
  if( !adopt )
  {
    std::fstream file( file_name, mode );

    if( !file.is_open() )
    {
      file.rdstate();
      throw std::string( "regular_file: a file can't be created, file_name: " + file_name + ", stream state: "
                         + std::to_string( file.rdstate() ) );
    }
  }
}

regular_file::~regular_file()
{
  std::remove( file_name.c_str() );
}

regular_file& regular_file::operator<<( const std::string& str )
{
  {
    /* if a file doesn't exist, such a combination of flags causes an error */
    std::fstream file( file_name, std::ios::in | std::ios::out );
    if( !file.is_open() )
      throw std::string( "regular_file: a file can't be opened, 'cause it doesn't exist." );
  }

  std::fstream file( file_name, std::ios::app );
  file << str;

  return *this;
}
