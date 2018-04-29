/*
 * epoll_event_loop.h
 *
 *  Created on: Feb 7, 2018
 *      Author: sergs (ivan0ivanov0@mail.ru)
 */

#ifndef EPOLL_EVENT_LOOP_H
#define EPOLL_EVENT_LOOP_H

#include <map>
#include <functional>
#include <memory>
#include <cstdint>
#include <iosfwd>
#include <string>

#include <sys/epoll.h>

#include "object.h"

class event_source;

/* this class implements Linux epoll functionality;
 * it can be used to implement app's event loop
 *
 * instance of this class contains and monitors the set of event sources
 * (generally speaking file descriptors), if some event source issues
 * an event the fd, returned by 'get_fd', gets triggered thus providing
 * an ability to handle this event by invoking 'handle_events';
 *
 * any errors are reported via exceptions
 * doesn't support a copy semantic
 *
 * TODO: isn't thread-safe */
class epoll_event_loop : public object
{
public:
  enum class event_type { EPOLL_IN = EPOLLIN, EPOLL_OUT = EPOLLOUT, EPOLL_RDHUP = EPOLLRDHUP };

  /* int - is a fd an event happened at (it's a duplication of a real fd,
   *       which will be closed automatically);
   * void* - is a pointer to the user-supplied data passed to an add_event_source member function  */
  using event_cb = std::function<void(int, void*)>;

  epoll_event_loop();
  ~epoll_event_loop();

  epoll_event_loop( const epoll_event_loop& that ) = delete;
  epoll_event_loop& operator=( const epoll_event_loop& that ) = delete;

  epoll_event_loop( epoll_event_loop&& that ) = default;
  epoll_event_loop& operator=( epoll_event_loop&& that );

  friend void swap( epoll_event_loop& lhs, epoll_event_loop& rhs ) noexcept;

  /* add a source of event (start being notified about the event);
   * callable object 'func' with dup('fd') and 'data' as arguments will be called
   * if an event of type 'type' happens on a fd dup('fd');
   * a fd 'fd' will be dupped;
   * return an event_source's id which can be used to remove this source of event from the event loop */
  uint64_t add_event_source( int fd, event_type type, event_cb func, void* data, const std::string& debug_str = "" );

  /* remove a source of event (stop being notified about the event);
   * 'ev_src_id' should be a valid event_source's id */
  void remove_event_source( uint64_t ev_src_id );

  /* return fd which can be built-in app's main event loop;
   * if fd returned by this function gets triggered 'hadle_events' functions should be called
   * to handle events;
   * returned fd is a dup of real underlying fd */
  int get_fd() const;
  void handle_events();

  /* to map epoll_event_loop::event_type constants to strings */
  friend std::ostream& operator <<( std::ostream& out, epoll_event_loop::event_type ev_type );

private:
  using base = object;

  /* to map epoll.h constants to strings */
  static const std::string& get_event_name( uint32_t ev );

private:
  int epoll_fd;
  std::map<uint64_t, std::shared_ptr<event_source>> id_to_ev_src_map; /* manages the event_sources' memory */

  static const std::map<uint32_t, std::string> event_type_to_string_map; /* to map epoll.h constants to strings */
};

/* not-public class representing an event_source, pointers to instances of this class
 * are passed to the kernel via epoll_ctl(EPOLL_CTL_ADD) */
class event_source : public object
{
public:
  /* no copy semantic as an event_source is supposed to be unique */
  event_source( const event_source& that ) = delete;
  event_source& operator=( const event_source& that ) = delete;

  event_source( event_source&& that ) = default;
  event_source& operator=( event_source&& that ) = default;

private:
  /* only epoll_event_loop's instances should be able to create
   * instances of an 'event_source' object */
  friend class epoll_event_loop;

  event_source();

private:
  std::function<void(int, void*)> func;
  void* data;

  epoll_event_loop::event_type type;

  int fd;
  bool armed; /* is fd armed (added to the epoll instance) or not */

  std::string debug_str;
};

#endif /* EPOLL_EVENT_LOOP_H */
