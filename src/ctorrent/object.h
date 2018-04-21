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
 * it provides:
 *  - unique, per process, id
 *
 * a dtor is NOT virtual;
 * a copy operation affects an id;
 * a copy assignment operation affects an id;
 * a move operation doesn't affect an id;
 * a move assignment operation doesn't affect an id;
 */
class object
{
public:
  object() : id(id_generator::get_instance().get_id()) {}

  /* TODO: I'm not sure is it a suitable approach to leave a base's dtor not virtual,
   *       but having a reference or pointer of this type points to derived object seems
   *       insane in my case, so if a virtual dtor is needed it has to be provided by
   *       the derived classes and references and pointers of that derived types have
   *       to be used to achieve the dynamic polymorphism */
  ~object() = default;

  /* if we make a copy we make an another object, so generate a new id */
  object( const object& that ) : id(id_generator::get_instance().get_id()) {}
  object& operator=( const object& that ) { id = id_generator::get_instance().get_id(); return *this; }

  object( object&& that ) = default;
  object& operator=( object&& that ) = default;

  uint64_t get_id() const { return id; }

private:
  uint64_t id;
};

#endif /* OBJECT_H */
