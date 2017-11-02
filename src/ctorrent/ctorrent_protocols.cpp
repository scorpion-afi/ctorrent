/*
 * ctorrent_protocols.cpp
 *
 *  Created on: Oct 5, 2017
 *      Author: sergs (ivan0ivanov0@mail.ru)
 */

#include "config.h"

#include <cstring>
#include <sstream>

#include <boost/log/trivial.hpp>

#include "ctorrent_protocols.h"

calc_chunk::calc_chunk() : m_data(nullptr), m_data_size(0)
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

calc_result calc_chunk::evaluate()
{
  calc_result res;

  return res;
}

std::string calc_chunk::get_info() const
{
  std::stringstream str;

  str << "m_data: " << m_data << ", m_data_size: " << m_data_size << "\nm_method_src:\n"
      << m_method_src;

  return str.str();
}

