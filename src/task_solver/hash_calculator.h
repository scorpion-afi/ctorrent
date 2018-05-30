/*
 * hash_calculator.h
 *
 *  Created on: Oct 4, 2017
 *      Author: sergs (ivan0ivanov0@mail.ru)
 */

#ifndef HASH_CALCULATOR_H
#define HASH_CALCULATOR_H

#include <string>
#include <vector>
#include <memory>

class base_calc;

class hash_calculator
{
public:
  hash_calculator();

  /* trigger the hash calculation for the string 'str';
   * sleeps till gets a result;
   * return the calculated hash */
  uint64_t get_hash( const std::string& str );

private:
  uint64_t distribute_calculation( const std::vector<std::shared_ptr<base_calc>>& objs );

private:
  static const std::size_t m_chunk_size = 4;
  const std::size_t base = 53;  /* the hash base for the Latin alphabet */

  std::string method;

  static_assert( m_chunk_size != 0, "m_chunk_size should not equal zero" );
};

#endif /* HASH_CALCULATOR_H */
