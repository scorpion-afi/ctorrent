/*
 * ctorrent_protocols.h
 *
 *  Created on: Oct 5, 2017
 *      Author: sergs (ivan0ivanov0@mail.ru)
 */

#ifndef CTORRENT_PROTOCOLS_H_
#define CTORRENT_PROTOCOLS_H_

#include <string>
#include <type_traits>
#include <memory>

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

/* the base abstract class for classes used to present the task to calculate */
class base_calc : public base_serialize
{
public:
  /* make an actual computation;
   * what's going on under hood depends on implementation of this abstract class */
  virtual std::shared_ptr<base_calc_result> compute() = 0;

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

class calc_chunk;

/* the easiest implementation of a base_calc_result class, just controls a C-array of bytes;
 * no copy-semantic (Why, why not? What's a reason to support such a no-trivial copy-semantic?) */
class calc_result : public base_calc_result
{
public:
  /* a calc_result object needs the same id as a counterpart calc_chunk object */
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
  uint64_t calc_result_id;
};

BOOST_CLASS_TRACKING( calc_result, track_never );

/* interface class to inherit from to provide the class which can be
 * used to pass the user-data to a calc_chunk class */
class i_chunk_data
{
public:
  virtual ~i_chunk_data() {}

  virtual const void* get_raw_data() const = 0;
  virtual std::size_t get_raw_data_size() const = 0;
};

/* the easiest implementation of a base_calc class,
 * manages string representing sources of code and C-array of data;
 * no copy semantic */
class calc_chunk : public base_calc
{
public:
  calc_chunk();
  ~calc_chunk() override;

  calc_chunk( const calc_chunk& that ) = delete;
  calc_chunk& operator=( const calc_chunk& that ) = delete;

  calc_chunk( calc_chunk&& that ) = default;
  calc_chunk& operator=( calc_chunk&& that ) = default;

  void grab_data( const i_chunk_data &data );
  void set_method_src( std::string method_src );

  std::shared_ptr<base_calc_result> compute() override;

  /* this function can be used to check a 'm_data' type;
   * calc_chunk is intended to be passed over sockets by using boost.serialization library,
   * so 'm_data' got set by 'grab_data' is going to be passed too, but host side knows nothing
   * about the type of 'm_data' so to make this class universal the type of 'm_data' has to be
   * POD type (C-like structure) so 'm_data' can be passed like a chunk of memory */
  template< class T >
  static constexpr bool check_type() { return std::is_pod<T>::value ? true : throw std::string( "bad type" ); }

  /* a calc_result object needs the same id as a counterpart calc_chunk object */
  uint64_t get_calc_chunk_id() const { return calc_chunk_id; }

  /* for debug purposes */
  std::string get_info() const;

private:
  /* an 'access' class should have an access to our private method 'serialize' */
  friend class boost::serialization::access;

  template<class Archive>
  void save( Archive& ar, const unsigned int version ) const
  {
    /* provide a unique, per a client-server connection, id during serialization/sending of an object */
    calc_chunk_id = id_generator::get_instance().get_id();

    /* an insane way to serialize base part of derived object */
    ar & boost::serialization::base_object<const base_calc>(*this);

    ar & calc_chunk_id;
    ar & m_data_size;

    for( std::size_t i = 0; i < m_data_size; i++ )
      ar & m_data[i];

    ar & m_method_src;

    BOOST_LOG_TRIVIAL( debug ) << "calc_chunk::save";
  }

  template<class Archive>
  void load( Archive& ar, const unsigned int version )
  {
    /* an insane way to serialize base part of derived object */
    ar & boost::serialization::base_object<base_calc>(*this);

    ar & calc_chunk_id;
    ar & m_data_size;

    m_data = new char[m_data_size];

    for( std::size_t i = 0; i < m_data_size; i++ )
      ar & m_data[i];

    ar & m_method_src;

    BOOST_LOG_TRIVIAL( debug ) << "calc_chunk::load";
  }

  /* to split a serialization into 'save' and 'load' */
  BOOST_SERIALIZATION_SPLIT_MEMBER();

private:
  /* server doesn't care about this id, it's up to a client side to handle it properly */
  mutable uint64_t calc_chunk_id;

  /* data to process */
  char* m_data;
  std::size_t m_data_size;

  /* extern "C" std::shared_ptr<base_calc_result> compute( const calc_chunk&, const void* );
   *
   * Note: the function has to be declared with C linkage to allow being found by libdl */
  std::string m_method_src;
};

BOOST_CLASS_TRACKING( calc_chunk, track_never );

#endif /* CTORRENT_PROTOCOLS_H_ */
