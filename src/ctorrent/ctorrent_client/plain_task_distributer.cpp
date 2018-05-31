/*
 * plain_task_distributer.cpp
 *
 *  Created on: May 30, 2018
 *      Author: sergs
 */

#include "config.h"

#include <cmath>

#include <boost/log/trivial.hpp>

#include "remote_server.h"
#include "ctorrent_protocols.h"

#include "plain_task_distributer.h"


void plain_task_distributer::distribute( const std::list<std::shared_ptr<remote_server>>& server_list,
                             const std::vector<std::shared_ptr<base_calc>>& objs )
{
  const std::size_t packages_per_server = objs.size() / server_list.size();
  std::size_t remainder_pkg_cnt, pkg_cnt, log_cnt;
  auto server = server_list.begin();

  remainder_pkg_cnt = objs.size() % server_list.size();
  pkg_cnt = packages_per_server;
  log_cnt = 0;

  /* take an object from the remainder pool (if any) and put it to
   * the objects set for the FIRST server */
  if( remainder_pkg_cnt > 0 )
  {
    remainder_pkg_cnt--;
    pkg_cnt++;
  }

  /* spread objects to calculate between remote calculation servers as equally as possible;
   * objects from remainder pool (remainder_pkg_cnt objects) are spread equally between servers */
  for( const auto& obj : objs )
  {
    (*server)->serialize_and_send( obj.get() );
    log_cnt++;

    /* it's time to switch a server */
    if( --pkg_cnt == 0 )
    {
      pkg_cnt = packages_per_server;

      /* take an object from the remainder pool (if any) and put it to
       * the objects set for the next server */
      if( remainder_pkg_cnt > 0 )
      {
        remainder_pkg_cnt--;
        pkg_cnt++;
      }

      (*server)->flush_serialized_data();

      BOOST_LOG_TRIVIAL( info ) << "plain_task_distributer: " << log_cnt << " object(s) have been sent"
          " to the " << (*server)->get_identify_str();

      log_cnt = 0;

      server++;
    }
  }

  BOOST_LOG_TRIVIAL( info );
}
