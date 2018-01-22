/*
 * serializer_deserializer.h
 *
 *  Created on: Mar 11, 2018
 *      Author: sergs
 */

#ifndef SERIALIZER_DESERIALIZER_H
#define SERIALIZER_DESERIALIZER_H

#include <cstdint>
#include <iosfwd>

#include <boost/log/trivial.hpp>
#include <boost/serialization/base_object.hpp>
//#include <boost/serialization/tracking.hpp>

#include "object.h"

/* the base class for serialization/deserialization operation;
 * has to be inherited by classes intended to be serialized/deserialized */
class base_serialize
{
public:
  virtual ~base_serialize() = default;  /* this type should be polymorphic to
                                           allow polymorphic serialization/deserialization */

private:
  /* an 'access' class should have an access to our private method 'serialize' */
  friend class boost::serialization::access;

  template<class Archive>
  void serialize( Archive& ar, const unsigned int version )
  {
    /* KEEP NO-OPERATION here, look at comments bellow */
    BOOST_LOG_TRIVIAL( debug ) << "base_serialize::serialize";
  }
};

/* for some reason the boost.serialize library tracks base_serialize parts of objects derived from
 * a base_serialize class (despite that we have the tracking disabled for type of that objects),
 * thus causing problems which can be fixed by disabling such tracking, but if we disable the tracking
 * we get some warnings when we compile for Android, so to avoid such warnings we don't disable the tracking
 * and just promise to avoid ANY REAL serialization for the base_serialize class */
//BOOST_CLASS_TRACKING( base_serialize, track_never );

class serializer_deserializer : public object
{
protected:
  /* has to be a POD-like type without any padding;
   * all members have to be platform-independent */
  struct control_object
  {
    uint32_t pkg_size;     /* a size of the client package */
    uint32_t objs_amount;  /* a total number of objects within the client package */
  }  __attribute__((packed, aligned(1)));

  friend std::ostream& operator<<( std::ostream& o_stream, const control_object& control_obj );
  friend std::istream& operator>>( std::istream& i_stream, control_object& control_obj );

  static const std::size_t client_package_size = 65536; /* control object + serialized data (max size) */
  static const std::size_t control_object_size = sizeof(control_object);

  static_assert( std::is_pod<control_object>::value, "control_object has to be a POD type" );
};

#endif /* SERIALIZER_DESERIALIZER_H */
