/*
 * file_descriptor.h
 *
 *  Created on: Sep 7, 2018
 *      Author: sergs
 */

#ifndef FILE_DESCRIPTOR_H
#define FILE_DESCRIPTOR_H

#include <unistd.h>

#include "epoll_event_loop.h"

/* this class represents a file descriptor semantic;
 * may be used as a base-wrapper over OS-objects which can be represented
 * by a file descriptor;
 * - supports copy and move semantics;
 * - provides an operator int() conversion operator for usability */
class file_descriptor
{
public:
  file_descriptor() : fd(-1) {}
  explicit file_descriptor( int fd ) : fd(fd) {}
  virtual ~file_descriptor() { if( fd >= 0 ) close( fd ); }

  file_descriptor( const file_descriptor& rhs )
  {
    fd = dup( rhs.fd );
  }

  file_descriptor& operator=( const file_descriptor& rhs )
  {
    if( &rhs == this )
      return *this;

    if( fd >= 0 )
      close( fd );
    fd = dup( rhs.fd );

    return *this;
  }

  file_descriptor( file_descriptor&& rhs )
  {
    fd = rhs.fd;
    rhs.fd = -1;
  }

  file_descriptor& operator=( file_descriptor&& rhs )
  {
    if( &rhs == this )
      return *this;

    if( fd >= 0 )
      close( fd );
    fd = rhs.fd;
    rhs.fd = -1;

    return *this;
  }

  /* TODO: is it a good decision? */
  operator int() const { return fd; }

private:
  int fd;
};

#endif /* FILE_DESCRIPTOR_H */
