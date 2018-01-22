/*
 * main.cpp
 *
 *  Created on: Jan 24, 2017
 *      Author: sergs (ivan0ivanov0@mail.ru)
 */


#include <config.h>

#include <csignal>
#include <exception>
#include <string>
#include <cstdlib>
#include <memory>

#include <poll.h>
#include <sys/signalfd.h>

#include "ctorrent_server.h"
#include "log.h"

/* eXecutable and Dispatching Unit - has to be installed on the host side to
 * dispatch and execute the packages */

/* TODO: this executable should be executed in background, options can be changed
 * from a foreground process through an UNIX_SOCKET */


int main( void )
{
  try
  {
    init_boost_log();

    ctorrent_server xdu;
    pollfd events_fd[2];
    sigset_t sig_mask;
    int sig_fd;

    sigemptyset( &sig_mask );
    sigaddset( &sig_mask, SIGINT );

    /* block signals (current thread signal mask + sig_mask) so that they aren't handled
     * according to their default dispositions;
     * signals will be pending till someone consumes 'em from the signal queue */
    if( sigprocmask( SIG_BLOCK, &sig_mask, NULL ) < 0 )
      throw std::string( "an error while sigprocmask syscall." );

    sig_fd = signalfd( -1, &sig_mask, 0 );
    if( sig_fd < 0 )
      throw std::string( "an error while signalfd syscall." );

    /* fds should be set to '-1' to be ignored by poll */
    for( auto& ev : events_fd )
      ev.fd = -1;

    events_fd[0].events = POLLIN;
    events_fd[0].fd = xdu.get_fd(); /* TODO: returned fd should be closed */
    events_fd[1].events = POLLIN;
    events_fd[1].fd = sig_fd;

    BOOST_LOG_TRIVIAL( info );  /* this puts an empty log line :) */
    BOOST_LOG_TRIVIAL( info ) << "get started to execute and dispatch...";
    BOOST_LOG_TRIVIAL( info );

    while( 1 )
    {
      int ret;

      ret = poll( events_fd, sizeof(events_fd) / sizeof(pollfd), -1 );
      if( ret < 0 && errno == EINTR )
        continue;

      if( ret < 0 )
        throw std::string( "an error while poll syscall." );

      if( events_fd[0].revents == POLLIN )
        xdu.handle_events();

      if( events_fd[1].revents == POLLIN )
      {
        signalfd_siginfo fd_si;

        read( events_fd[1].fd, &fd_si, sizeof(fd_si) );
        BOOST_LOG_TRIVIAL( info ) << "a signal with number " << fd_si.ssi_signo << " has been caught.";
      }
    }
  }
  catch( const std::string &err )
  {
    BOOST_LOG_TRIVIAL( info );
    BOOST_LOG_TRIVIAL( info ) << "an exception: " << err;
  }
  catch( const std::exception &exception )
  {
    BOOST_LOG_TRIVIAL( info );
    BOOST_LOG_TRIVIAL( info ) << "an exception: " << exception.what();
  }
  catch(...)
  {
    BOOST_LOG_TRIVIAL( info );
    BOOST_LOG_TRIVIAL( info ) << "some exception has been caught.";
  }
}
