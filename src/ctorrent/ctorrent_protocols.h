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

/* interface class to inherit from to provide the class which can be
 * used to pass user-data to a calc_chunk class */
class i_chunk_data
{
public:

  i_chunk_data() = default;
  virtual ~i_chunk_data() {}

  virtual const void* get_raw_data() const = 0;
  virtual std::size_t get_raw_data_size() const = 0;
};

/* no copy semantic */
struct calc_result
{
  calc_result() : data(nullptr), data_size(0) {}
  virtual ~calc_result() { delete [] data; }

  calc_result( const calc_result &that ) = delete;

  calc_result( calc_result &&that ) : calc_result()
  {
    swap( *this, that );
  }

  calc_result& operator=( calc_result that )
  {
    swap( *this, that );

    return *this;
  }

  friend void swap( calc_result &first, calc_result &second )
  {
    using std::swap;

    swap( first.data, second.data );
    swap( first.data_size, second.data_size );
  }

  char *data; /* should be a pointer to array; (may contain only a POD type) */
  std::size_t data_size;
};

/* no copy semantic */
class calc_chunk
{
public:

  calc_chunk();
  virtual ~calc_chunk();

  calc_chunk( const calc_chunk &that ) = delete;
  calc_chunk( calc_chunk &&that );

  calc_chunk& operator=( calc_chunk that );

  friend void swap( calc_chunk &first, calc_chunk &second );


  void grab_data( const i_chunk_data &data );
  void set_method_src( std::string method_src );

  calc_result evaluate();

  /* this function can be used to check a 'data' type;
   * calc_chunk is intended to be passed over sockets by using boost.serialization library,
   * so 'data' got set by 'set_data' is going to be passed too, but host side knows nothing
   * about the type of 'data' so to make this class universal the type of 'data' has to be
   * POD type (C-like structure) so 'data' can be passed like a chunk of memory */
  template< class T >
  static constexpr bool check_type() { return std::is_pod<T>::value ? true : throw("bad type"); }

  /* for debug purposes */
  std::string get_info() const;

private:

  char* m_data;
  std::size_t m_data_size;

  /* calc_result evaluate( void *data ); */
  std::string m_method_src;
};

#endif /* CTORRENT_PROTOCOLS_H_ */
