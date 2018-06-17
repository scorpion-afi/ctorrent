/*
 * ctorrent_protocols.h
 *
 *  Created on: Oct 5, 2017
 *      Author: sergs (ivan0ivanov0@mail.ru)
 */

#ifndef CTORRENT_PROTOCOLS_H_
#define CTORRENT_PROTOCOLS_H_

#include <string>
#include <memory>
#include <iosfwd>

#include <boost/log/trivial.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/tracking.hpp>

#include "serializer_deserializer.h"
#include "id_generator.h"


/* the base class for classes used as a result of a base_calc::compute() method;
 * derived classes have to provide a template serialize() function to make own serialization */
class base_calc_result : public base_serialize
{
public:
  /* whether the order of this package is important;
   * if the order isn't important it may decrease average time of packages' calculation */
  virtual bool is_order_important() const { return true; }

private:
  /* an 'access' class should have an access to our private method 'serialize' */
  friend class boost::serialization::access;

  template<class Archive>
  void serialize( Archive& ar, const unsigned int version )
  {
    /* an insane way to serialize base part of derived object */
    ar & boost::serialization::base_object<base_serialize>(*this);
    BOOST_LOG_TRIVIAL( debug ) << "base_calc_result::serialize";
  }
};

/* the boost.serialize library, by default, tracks addresses of objects serialized by pointers,
 * or registered, or exported, it may lead to omitting objects serialization if objects have the
 * same address, even if they're completely different objects (e.g. an object is created in stack,
 * a pointer to the object gets used for serialization, the object gets destroyed, and at the
 * same space in stack, a new object gets created), to avoid this we disable such tracking for
 * some types (one of them is base_calc_result);
 * Note: if several objects of user-defined types keep pointers to the same object of user-defined type (e.g. A)
 *       that object WON'T be serialized several times, unless you turn off the tracking for that type (A) too */
BOOST_CLASS_TRACKING( base_calc_result, track_never );

/* type of computation model to use */
enum class comp_type
{
  RAW_SRC,  /* a task object contains a task as raw sources and data as an array of bytes */
};

/* the base abstract class for classes used to present the task to calculate */
class base_calc : public base_serialize
{
public:
  /* make an actual computation;
   * what's going on under hood depends on implementation of this abstract class */
  virtual std::unique_ptr<base_calc_result> compute() = 0;

  /* implementation has to return a specific type of computation model to
   * choose a computer this task has to be computed on */
  virtual comp_type get_comp_type() const = 0;

private:
  /* an 'access' class should have an access to our private method 'serialize' */
  friend class boost::serialization::access;

  template<class Archive>
  void serialize( Archive& ar, const unsigned int version )
  {
    /* an insane way to serialize base part of derived object */
    ar & boost::serialization::base_object<base_serialize>(*this);
    BOOST_LOG_TRIVIAL( debug ) << "base_calc::serialize";
  }
};

BOOST_CLASS_TRACKING( base_calc, track_never );


/*               a calc_chunk-calc_result (raw_src_computer) task-result (a computer) pair               */


class calc_chunk;
class raw_src_computer;

/* the easiest implementation of a base_calc_result class, just controls a C-array of bytes;
 * no copy-semantic (Why, why not? What's a reason to support such a no-trivial copy-semantic?) */
class calc_result : public base_calc_result
{
public:
  /* to match a calc_result object with a counterpart calc_chunk object
   * to be able to order calc_result objects in the same order as calc_chunk objects */
  explicit calc_result( const calc_chunk& calc_obj );
  ~calc_result() override;

  calc_result( const calc_result& that ) = delete;
  calc_result& operator=( const calc_result& that ) = delete;

  calc_result( calc_result&& that ) = default;
  calc_result& operator=( calc_result&& that ) = default;

  /* way to order calc_result packages */
  bool operator<( const calc_result& rhs ) const { return calc_result_id < rhs.calc_result_id; }

public:
  char *data; /* should be a pointer to array; (may contain only a POD type) */
  uint64_t data_size;

private:
  /* an 'access' class should have an access to our private method 'serialize' */
  friend class boost::serialization::access;

  /* boost serialization library needs an access to default ctor,
   * but other world shouldn't have an access */
  calc_result();

  template<class Archive>
  void save( Archive& ar, const unsigned int version ) const
  {
    /* an insane way to serialize base part of derived object */
    ar & boost::serialization::base_object<const base_calc_result>(*this);

    ar & calc_result_id;
    ar & data_size;

    for( std::size_t i = 0; i < data_size; i++ )
      ar & data[i];

    BOOST_LOG_TRIVIAL( debug ) << "calc_result::save";
  }

  template<class Archive>
  void load( Archive& ar, const unsigned int version )
  {
    /* an insane way to serialize base part of derived object */
    ar & boost::serialization::base_object<base_calc_result>(*this);

    ar & calc_result_id;
    ar & data_size;

    data = new char[data_size];

    for( std::size_t i = 0; i < data_size; i++ )
      ar & data[i];

    BOOST_LOG_TRIVIAL( debug ) << "calc_result::load";
  }

  /* to split a serialization into 'save' and 'load' */
  BOOST_SERIALIZATION_SPLIT_MEMBER();

private:
  /* stores an id of a counterpart calc_chunk object */
  uint64_t calc_result_id;
};

BOOST_CLASS_TRACKING( calc_result, track_never );


/* if a task can be divided into several parts - chunks this class can be used
 * to represent such parts; each chunk contains an independent task to do (sources
 * as a std::string object) and a data to be processed by this task (data can be only
 * a POD-type object).
 *
 * no copy semantic (Why, why not? What's a reason to support such a no-trivial copy-semantic?);
 */
class calc_chunk : public base_calc
{
public:
  /* @param [in] task_scr - source of task to compute
   *   An API of a task to compute:
   *
   *   [in] chunk - a calc_chunk the method (compute) gets called for
   *   extern "C" std::unique_ptr<const calc_result> compute( const calc_chunk& chunk );
   *
   *   Note: the function has to be declared with C linkage to allow being found by libdl
   *   Note: the function has to be a thread-safe one (several threads may execute it concurrenlty)
   *
   * @param [in] data - a pointer to a dynamically allocated ARRAY of chars (only POD types)
   * @param [in] data_size - a size of @c data array (used as to pass the data over network the size is necessary)
   */
  calc_chunk( std::string task_src, std::unique_ptr<char[]> data, uint64_t data_size );

  calc_chunk( const calc_chunk& that ) = delete;
  calc_chunk& operator=( const calc_chunk& that ) = delete;

  calc_chunk( calc_chunk&& that ) = default;
  calc_chunk& operator=( calc_chunk&& that ) = default;

  std::unique_ptr<base_calc_result> compute() override;
  comp_type get_comp_type() const override { return comp_type::RAW_SRC; }

  /* get an access to the task's data */
  const void* get_data() const { return data.get(); }

  /* for loging and debug purposes */
  friend std::ostream& operator<<( std::ostream& stream, const calc_chunk& co );

private:
  /* a calc_result object needs the same id as a counterpart calc_chunk object */
  friend class calc_result;

  /* an 'access' class should have an access to our private method 'serialize' */
  friend class boost::serialization::access;

  /* raw_src_computer is a computer for this type of task, it knows how to work with
   * this task, so it needs an access */
  friend class raw_src_computer;

  /* only boost.serialization library has to be able to use this ctor;
   * after boost.serialization library has created object using this ctor
   * object gets initialized by serialize/load method so it's safe to have
   * such the ctor as a default one */
  calc_chunk() = default;

  uint64_t get_calc_chunk_id() const { return calc_chunk_id; }

  template<class Archive>
  void save( Archive& ar, const unsigned int version ) const
  {
    /* provide a unique, per a client-server connection, id during serialization/sending of an object */
    calc_chunk_id = id_generator::get_instance().get_id();

    /* an insane way to serialize base part of derived object */
    ar & boost::serialization::base_object<const base_calc>(*this);

    ar & calc_chunk_id;
    ar & data_size;

    for( std::size_t i = 0; i < data_size; i++ )
      ar & data[i];

    ar & task_src;

    BOOST_LOG_TRIVIAL( debug ) << "calc_chunk::save";
  }

  template<class Archive>
  void load( Archive& ar, const unsigned int version )
  {
    /* an insane way to serialize base part of derived object */
    ar & boost::serialization::base_object<base_calc>(*this);

    ar & calc_chunk_id;
    ar & data_size;

    data.reset( new char[data_size] );

    for( std::size_t i = 0; i < data_size; i++ )
      ar & data[i];

    ar & task_src;

    BOOST_LOG_TRIVIAL( debug ) << "calc_chunk::load";
  }

  /* to split a serialization into 'save' and 'load' */
  BOOST_SERIALIZATION_SPLIT_MEMBER();

private:
  std::string task_src;

  /* this data is going to be passed over network AS IS, so can hold only POD types */
  std::unique_ptr<char[]> data;
  uint64_t data_size;

  /* server doesn't care about this id, it's up to a client side to handle it properly */
  mutable uint64_t calc_chunk_id;
};

BOOST_CLASS_TRACKING( calc_chunk, track_never );

#endif /* CTORRENT_PROTOCOLS_H_ */
