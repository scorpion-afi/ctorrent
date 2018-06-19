/*
 * ctorrent_protocols.cpp
 *
 *  Created on: Oct 5, 2017
 *      Author: sergs (ivan0ivanov0@mail.ru)
 */

#include "config.h"

#include <fstream>

#include <boost/log/trivial.hpp>

#include "ctorrent_protocols.h"

calc_result::calc_result() : data(nullptr), data_size(0)
{
}

calc_result::calc_result( const calc_chunk& calc_obj ) : base_calc_result(calc_obj),
    data(nullptr), data_size(0)
{
}

calc_result::~calc_result()
{
  delete [] data;
}


calc_chunk::calc_chunk( std::string m_method_src, std::unique_ptr<char[]> data, uint64_t data_size ) :
    task_src( std::move(m_method_src) ), data(std::move(data)), data_size(data_size)
{
}

std::ostream& operator<<( std::ostream& stream, const calc_chunk& co )
{
  stream << "calc_chunk [" << &co << "], m_data:\n";

  for( std::size_t i = 0; i < co.data_size; i++ )
    stream << co.data[i];

  stream << "\n m_data_size: " << co.data_size << "\nm_method_src:\n" << co.task_src;

  return stream;
}

