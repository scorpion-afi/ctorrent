/*
 * ctorrent.h
 *
 *  Created on: Oct 4, 2017
 *      Author: sergs (ivan0ivanov0@mail.ru)
 */

#ifndef CTORRENT_H_
#define CTORRENT_H_

#include <string>
#include <memory>

#include <arpa/inet.h> /* IPv4 */

#include "ctorrent_protocols.h"
#include "epoll_event_loop.h"

class ctorrent
{
protected:
  /* to get correct result use string.c_str() */
  static std::shared_ptr<std::string> convert_ipv4_from_binary_to_text( in_addr ipv4 )
  {
    /* man-page for inet_ntop says that at least an INET_ADDRSTRLEN-bytes buffer should
     * be provided, but there's no mentions about whether inet_ntop puts the '\0' symbol
     * as a terminate symbol or not, so make it explicitly */
    auto address = std::make_shared<std::string>( INET_ADDRSTRLEN, '\0' );

    if( !inet_ntop( AF_INET, &ipv4, &address->front(), address->size() ) )
      throw std::string( "an error while trying to get the text representation of an ivp4 address." );

    return address;
  }

protected:
  static const in_port_t port_number = 50610;  /* why this one, why not? */
};

#endif /* CTORRENT_H_ */
