/*
 * id_generator.h
 *
 *  Created on: Feb 21, 2018
 *      Author: sergs
 */

#ifndef ID_GENERATOR_H
#define ID_GENERATOR_H

#include <cstdint>
#include <atomic>

/* purpose of this class is to generate a unique, per a process, number in a thread-safe manner
 * (from 1 and up to MAX_UNSIGNED_LONG_LONG) */
class id_generator
{
public:
  ~id_generator() = default;

  id_generator( const id_generator& that ) = delete;
  id_generator( id_generator&& that ) = delete;
  id_generator& operator=( id_generator that ) = delete;

  static id_generator& get_instance();
  uint64_t get_id();

private:
  id_generator();

private:
  std::atomic_ullong value; /* till c++17 there's no way to guarantee that it's a lock free atomic... */
};

#endif /* ID_GENERATOR_H */
