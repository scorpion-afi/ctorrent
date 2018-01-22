/*
 * hash_calculator.h
 *
 *  Created on: Oct 4, 2017
 *      Author: sergs (ivan0ivanov0@mail.ru)
 */

#ifndef HASH_CALCULATOR_H_
#define HASH_CALCULATOR_H_

#include <string>
#include <vector>
#include <memory>

#include "ctorrent_client.h"

class hash_calculator
{
public:
  hash_calculator();

  /* trigger the hash calculation for the string 'str';
   * sleeps till gets a result;
   * return the calculated hash */
  std::uint64_t get_hash( const std::string &str );

private:
  uint64_t distribute_calculation( const std::vector<std::shared_ptr<base_calc>>& objs );

private:
  const std::size_t m_chunk_size = 4;
  const std::size_t base = 53;  /* the hash base for the Latin alphabet */

  std::string method;
};

#endif /* HASH_CALCULATOR_H_ */
