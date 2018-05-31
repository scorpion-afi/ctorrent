/*
 * task_distributer.cpp
 *
 *  Created on: May 30, 2018
 *      Author: sergs
 */

#include "config.h"

#include "task_distributer.h"

#ifdef CLIENT_PLAIN_TASK_DISTRIBUTER
#include "plain_task_distributer.h"
#endif

/* TODO: think about caching */
std::unique_ptr<task_distributer> make_task_distributer()
{
  std::unique_ptr<task_distributer> ptr;

#ifdef CLIENT_PLAIN_TASK_DISTRIBUTER
  ptr.reset( new plain_task_distributer );
#endif

  if( !ptr )
    throw std::string( "no chosen task distributer (client side)." );

  return ptr;
}
