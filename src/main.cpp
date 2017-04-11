#include <iostream>
#include <fstream>
#include <list>

#include <memory>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include <boost/serialization/list.hpp>

#include <boost/serialization/base_object.hpp>
#include <boost/serialization/version.hpp>

const std::string archive_name( "archive.txt" );


class base
{
public:
	base( int i, double d, char c ) : i(i), d(d), c(c) { int_list.push_back( 25 ); }
	base() : i(0), d(0.0), c('a') {}
	virtual ~base() {}

	virtual void show( void ) const
	{
		std::cout << "base object: " << "i: " << i << ", d: " << d << ", c: " << c << std::endl;
	}

private:

	friend std::ostream& operator<<( std::ostream& cout, const base& bs );

	/* an access class should have an access to our private method 'serialize' */
	friend class boost::serialization::access;

	template< class Archive >
	void serialize( Archive& ar, const unsigned int version )
	{
		std::cout << "serialization for base object, version: " << version << "...\n";

		/* load order is equal to store order */
		ar & i;
		ar & d;
		ar & c;
		ar & int_list;
	}

	int i;
	double d;
	char c;

	std::list<int> int_list;
};

std::ostream& operator<<( std::ostream& cout, const base& bs )
{
	bs.show();
	return std::cout;
}

class derived : public base
{
public:
	explicit derived( int num ) : base(3, 2.5, 'b'), hi(num), dd(8) {}
	derived() = default;

	void show( void ) const override
	{
		std::cout << "derived object:\n ";
		base::show();
		std::cout << " hi: " << hi << ", dd: " << dd << std::endl;
	}

private:

	friend class boost::serialization::access;

	template< class Archive >
	void serialize( Archive& ar, const unsigned int version )
	{
		/* an insane way to serialize base part of derived object */
		ar & boost::serialization::base_object<base>( *this );

		std::cout << "serialization for derived object, version: " << version << "...\n";
		ar & hi;

		if( version == 1 )
			ar & dd;
	}

	/* added for 0-th version */
	int hi;

	/* added for 1-st version */
	double dd;
};

BOOST_CLASS_VERSION( derived, 1 );


int main( void )
{
	std::ifstream ifs( archive_name );
	boost::archive::text_iarchive text_input_archive( ifs );

	derived* received_obj = nullptr;

	std::cout << "try to receive...\n";
	text_input_archive >> received_obj;

	std::cout << "received object:\n\n";
	std::cout << *received_obj;
}
