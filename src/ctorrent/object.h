/*
 * object.h
 *
 *  Created on: Feb 24, 2018
 *      Author: sergs
 */

#ifndef OBJECT_H
#define OBJECT_H

#include <cstdint>

#include "id_generator.h"

/* this is the base class for some library's classes;
 * a move operation doesn't affect an id */
class object
{
public:
  object() : id(id_generator::get_instance().get_id()) {}
  virtual ~object() {}

  object( const object& that ) : id(id_generator::get_instance().get_id()) {}
  object( object&& that ) : id(that.id) {}

  object& operator=( object that ) = delete;

  uint64_t get_id() const { return id; }

private:
  uint64_t id;
};

#endif /* OBJECT_H */
