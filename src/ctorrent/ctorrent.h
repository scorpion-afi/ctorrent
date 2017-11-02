/*
 * ctorrent.h
 *
 *  Created on: Oct 4, 2017
 *      Author: sergs (ivan0ivanov0@mail.ru)
 */

#ifndef CTORRENT_H_
#define CTORRENT_H_

#include <vector>

#include "ctorrent_protocols.h"

class ctorrent
{
public:
  ctorrent() {}
  virtual ~ctorrent() {}

  int send( const std::vector<calc_chunk> &calc_chunks );
  int receive( std::vector<calc_chunk> &calc_chunks );
};

#endif /* CTORRENT_H_ */
