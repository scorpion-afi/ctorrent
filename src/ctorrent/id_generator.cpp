/*
 * id_generator.cpp
 *
 *  Created on: Feb 21, 2018
 *      Author: sergs
 */

#include "config.h"

#include "id_generator.h"

id_generator& id_generator::get_instance()
{
  static id_generator id_gen; /* C++11 guarantees that this creation is thread-safe */
  return id_gen;
}

uint64_t id_generator::get_id()
{
  return value++;
}

id_generator::id_generator() : value(1)
{
}
