/*
 * id_generator.h
 *
 *  Created on: Feb 21, 2018
 *      Author: sergs
 */

#ifndef ID_GENERATOR_H
#define ID_GENERATOR_H

#include <cstdint>
#include <mutex>

/* TODO: this class mixes the singleton with the id generation, is it appropriate? */

/* purpose of this class is to generate a unique, per a process, number in a thread-safe manner
 * (from 1 and up to MAX_UNSIGNED_LONG) */
class id_generator
{
public:
  static id_generator& get_instance();
  uint64_t get_id();

private:
  id_generator();
  ~id_generator() {}

  id_generator( const id_generator& that ) = delete;
  id_generator& operator=( const id_generator& that ) = delete;

  id_generator( id_generator&& that ) = delete;
  id_generator& operator=( id_generator&& that ) = delete;

private:
  std::mutex mtx;
  uint64_t value;
};

#endif /* ID_GENERATOR_H */
