/*
 * base_computer.h
 *
 *  Created on: May 31, 2018
 *      Author: sergs
 */

#ifndef BASE_COMPUTER_H
#define BASE_COMPUTER_H

#include <shared_mutex>
#include <memory>
#include <map>

#include "ctorrent_protocols.h"
#include "object.h"

#ifndef X86_64_BUILD  // FIXME:
#warning "there's a race condition within a base_computer class for non-x86_64 architectures"
#endif

/* this abstract class provides API for the task (base_calc) computation;
 * thread-safe */
class base_computer : public object
{
public:
  base_computer() = default;
  virtual ~base_computer() = default;

  /* if we have an explicitly defaulted dtor we have NO an implicitly declared move semantic */
  base_computer( const base_computer& that ) = default;
  base_computer& operator=( const base_computer& that ) = default;

  base_computer( base_computer&& that ) = default;
  base_computer& operator=( base_computer&& that ) = default;

  /* make an actual computation;
   * what's going on under hood depends on implementation of this abstract class */
  virtual std::unique_ptr<const base_calc_result> compute( const base_calc& task ) const = 0;

  /* register all available computers */
  static void register_computers();

  static std::shared_ptr<base_computer> get_computer( comp_type type )
  {
#ifdef X86_64_BUILD
    std::shared_lock<std::shared_mutex> sh_lock( mtx );
#endif
    return registered_computers.at( type );
  }

private:
  /* create a specified computer and associate it with a @c type key (of comp_type type) */
  template< class computer_class >
  static void register_computer( comp_type type )
  {
    static_assert( std::is_base_of<base_computer, computer_class>::value, "a type of the computer to register has to be "
        "derived from a base_computer type");

#ifdef X86_64_BUILD
    std::lock_guard<std::shared_mutex> ex_lock( mtx );
#endif
    registered_computers[type] = std::make_shared<computer_class>();
  }

private:
  /* TODO: currently I use arm-android-gcc-5.3 which doesn't support std::shared_mutex as a read-write lock */
#ifdef X86_64_BUILD
  static std::shared_mutex mtx;
#endif
  static std::map<comp_type, std::shared_ptr<base_computer>> registered_computers;
};

#endif /* BASE_COMPUTER_H */
