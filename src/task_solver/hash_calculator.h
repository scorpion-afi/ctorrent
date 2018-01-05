/*
 * hash_calculator.h
 *
 *  Created on: Oct 4, 2017
 *      Author: sergs (ivan0ivanov0@mail.ru)
 */

#ifndef HASH_CALCULATOR_H_
#define HASH_CALCULATOR_H_

#include <cstdint>
#include <string>

class hash_calculator
{
public:
  hash_calculator();
  virtual ~hash_calculator() {};

  /* trigger the hash calculation for the string 'str';
   * sleeps till gets a result;
   * return the calculated hash */
  std::uint64_t get_hash( const std::string &str );

private:

  const std::size_t m_chunk_size = 4;
  const std::size_t base = 53;  /* the hash base for the Latin alphabet */

  std::string method;
};

#endif /* HASH_CALCULATOR_H_ */
