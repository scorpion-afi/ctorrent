/*
 * epoll_event_loop.cpp
 *
 *  Created on: Feb 7, 2018
 *      Author: sergs (ivan0ivanov0@mail.ru)
 */

#include "config.h"

#include <iostream>

#include <boost/log/trivial.hpp>

#include "epoll_event_loop.h"


epoll_event_loop::epoll_event_loop()
{
  epoll_fd = epoll_create( 1 );
  if( epoll_fd < 0 )
    throw std::system_error( errno, std::system_category(), "epoll_create" );

  BOOST_LOG_TRIVIAL( debug ) << "epoll: create the epoll instance";
}

epoll_event_loop::~epoll_event_loop()
{
  /* TODO: if iterators are valid after erase call? */
  /* unregister all registered event sources */
  for( const auto& id_ev_src_pair : id_to_ev_src_map )
    remove_event_source( id_ev_src_pair.first  );

  /* till clients keep itself references to the epoll instance (obtained by 'get_fd')
   * it won't be closed */
  close( epoll_fd );

  BOOST_LOG_TRIVIAL( debug ) << "epoll: destroy the epoll wrapper.";
}

epoll_event_loop& epoll_event_loop::operator=( epoll_event_loop&& that )
{
  swap( *this, that );

  return *this;
}

void swap( epoll_event_loop& lhs, epoll_event_loop& rhs ) noexcept
{
  using std::swap;

  swap( static_cast<epoll_event_loop::base&>(lhs), static_cast<epoll_event_loop::base&>(rhs) );

  swap( lhs.epoll_fd, rhs.epoll_fd );
  swap( lhs.id_to_ev_src_map, rhs.id_to_ev_src_map );
}

uint64_t epoll_event_loop::add_event_source( int fd, event_type type, std::function<void(int, void*)> func, void* data,
                                             const std::string& debug_str )
{
  std::shared_ptr<event_source> ev_src =
      std::shared_ptr<event_source>( new event_source() );  /* TODO: fix to std::make_shared */
  epoll_event ev;
  int new_fd;

  std::memset( &ev, 0, sizeof ev );
  new_fd = dup( fd );

  BOOST_LOG_TRIVIAL( debug ) << "epoll: register a new event source (" << debug_str << ") ...";
  BOOST_LOG_TRIVIAL( debug ) << " epoll: dup ev_src fd " << fd << " to " << new_fd;

  ev_src->fd = new_fd;
  ev_src->type = type;
  ev_src->func = std::move(func);
  ev_src->data = data;
  ev_src->debug_str = debug_str;

  id_to_ev_src_map[ev_src->get_id()] = ev_src;

  /* TODO: though it's safe, is it correct way? (it's not thread-safe) */
  /* it's free to have a direct pointer to a managed object as after
   * the managed object gets destroyed there won't be access to it
   * (if an event_source got destroyed, an fd had been disarmed already ) */
  ev.data.ptr = ev_src.get();

  ev.events = static_cast<uint32_t>(type);
  epoll_ctl( epoll_fd, EPOLL_CTL_ADD, new_fd, &ev );

  ev_src->armed = true;

  BOOST_LOG_TRIVIAL( debug ) << " ev_src: " << ev_src.get() << " (ref_cnt: " << ev_src.use_count() << ")" <<
       " (" << ev_src->debug_str << "), ev_id: " << ev_src->get_id() << ", fd: " << ev_src->fd << ", ev_type: " << ev_src->type <<
       ", armed: " << ev_src->armed;

  return ev_src->get_id();
}

void epoll_event_loop::remove_event_source( uint64_t ev_src_id )
{
  /* TODO: check if an ev_src_id is valid */
  std::shared_ptr<event_source> ev_src = id_to_ev_src_map[ev_src_id];

  /* TODO: I hope that the next epoll_wait won't return this event even if it happened
   * before/after a fd (event) was disarmed :), should be checked... */
  epoll_ctl( epoll_fd, EPOLL_CTL_DEL, ev_src->fd, nullptr );

  /* mark the event_source as disarmed to free a memory within handle_events() */
  ev_src->armed = false;

  close( ev_src->fd );
  ev_src->fd = -1;

  BOOST_LOG_TRIVIAL( debug ) << "epoll: unregister an event source...";
  BOOST_LOG_TRIVIAL( debug ) << " ev_src: " << ev_src.get() << " (ref_cnt: " << ev_src.use_count() << ")" <<
      " (" << ev_src->debug_str << "), ev_id: " << ev_src->get_id() << ", fd: " << ev_src->fd << ", ev_type: " << ev_src->type <<
       ", armed: " << ev_src->armed;

  /* the event_source object can't be freed immediately here 'cause kernel keeps pointer to it
   * and may return it via epoll_wait() */
}

int epoll_event_loop::get_fd() const
{
  int fd = dup( epoll_fd );
  if( fd < 0 )
    throw std::system_error( errno, std::system_category(), "dup" );

  BOOST_LOG_TRIVIAL( debug ) << "epoll: dup epoll_fd " << epoll_fd << " to " << fd;

  return fd;
}

/* TODO: fix error handling */
void epoll_event_loop::handle_events()
{
  std::array<epoll_event, 64> events; /* TODO: how much? */
  int evs_amount;

  /* as the 'handle_events' function should be called after an 'epoll_fd'
   * gets triggered the next call will never block */
  evs_amount = epoll_wait( epoll_fd, &events.front(), events.size(), -1 );
  if( evs_amount < 0 && errno == EINTR )  /* TODO: maybe it's more preferable just to block INTERRUPT signal? */
    return;

  if( evs_amount < 0 )
    throw std::system_error( errno, std::system_category(), "epoll_wait" );

  BOOST_LOG_TRIVIAL( debug ) << "epoll: handle the bunch of events...";

  /* call registered callbacks to handle the events */
  for( int i = 0; i < evs_amount; i++ )
  {
    if( events[i].events == EPOLLIN || events[i].events == EPOLLOUT || events[i].events == EPOLLRDHUP )
    {
      event_source* ev_src = static_cast<event_source*>(events[i].data.ptr);

      BOOST_LOG_TRIVIAL( debug ) << "-ev_src: " << ev_src << " (" << ev_src->debug_str << "), ev_id: " << ev_src->get_id() << ", fd: "
          << ev_src->fd << ", ev_type: " << ev_src->type << ", armed: " << ev_src->armed;

      /* epoll_wait may return several events by one call, during processing them
       * a remove_event_source() function can be called which means that a client
       * doens't want to be notified about that event so just apply that demand and
       * skip processing the event marked as disarmed */
      if( !ev_src->armed ) continue;

      /* provide user a duplicate of fd to avoid access to class internals */
      int fd = dup( ev_src->fd );
      ev_src->func( fd, ev_src->data );
      close( fd );  /*TODO: isn't exception-safe */
    }
    else
      BOOST_LOG_TRIVIAL( debug ) << "some not foreseeable event: " << events[i].events;

    BOOST_LOG_TRIVIAL( debug );
  }

  bool first = true;

  /* free the memory for disarmed event_sources, as there won't be any access to them anymore */
  for( auto it = id_to_ev_src_map.cbegin(); it != id_to_ev_src_map.cend(); )
  {
    std::shared_ptr<event_source> ev_src = (*it).second;

    if( !ev_src->armed )
    {
      if( first )
      {
        first = false;

        BOOST_LOG_TRIVIAL( debug ) << "epoll: free a disarmed event_source...";
      }

      BOOST_LOG_TRIVIAL( debug ) << " ev_src: " << ev_src << " (" << ev_src->debug_str << "), ev_id: " << ev_src->get_id() << ", fd: "
          << ev_src->fd << ", ev_type: " << ev_src->type << ", armed: " << ev_src->armed;

      it = id_to_ev_src_map.erase( it );
    }
    else
      it++;
  }
}

const std::map<uint32_t, std::string> epoll_event_loop::event_type_to_string_map =
{
  { EPOLLIN, "EPOLLIN" },
  { EPOLLOUT, "EPOLLOUT" },
  { EPOLLRDHUP, "EPOLLRDHUP" },
};

const std::string& epoll_event_loop::get_event_name( uint32_t ev )
{
  return event_type_to_string_map.at( ev );
}

std::ostream& operator <<( std::ostream& out, epoll_event_loop::event_type ev_type )
{
  return out << epoll_event_loop::get_event_name( static_cast<uint32_t>(ev_type) );
}

event_source::event_source() : data(nullptr), type(), fd(-1), armed(false)
{
}

