#include <iostream>
#include <fstream>
#include <list>

#include <memory>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include <boost/serialization/list.hpp>

#include <boost/serialization/base_object.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/split_member.hpp>

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
	void save( Archive& ar, const unsigned int version ) const
	{
		/* an insane way to serialize base part of derived object */
		ar & boost::serialization::base_object<base>(*this);

		std::cout << "store for derived object, version: " << version << "...\n";

		ar & hi;

		/* we got to write to archive anyway */
		ar & dd;
	}

	template< class Archive >
	void load( Archive& ar, const unsigned int version )
	{
		ar & boost::serialization::base_object<base>( *this );

		std::cout << "load for derived object, version: " << version << "...\n";
		ar & hi;

		/* we got to check as we're loading object from archive (to support old archives) */
		if( version == 1 )
			ar & dd;
	}

	BOOST_SERIALIZATION_SPLIT_MEMBER()

	/* added for 0-th version */
	int hi;

	/* added for 1-st version */
	double dd;
};

BOOST_CLASS_VERSION( derived, 1 );

int main( void )
{
	{
		std::ofstream ofs( archive_name );
		boost::archive::text_oarchive text_output_archive( ofs );

		/* we got to register derived class to be able to save polymorphic pointers */
		text_output_archive.register_type( static_cast<derived*>(nullptr) );

		std::shared_ptr<base> bs = std::make_shared<derived>( 26 );
		std::cout << *bs;

		text_output_archive << bs.get();

		/* I don't understand what's the purpose of save_binary function */
		//derived der( 4 );
		//text_output_archive.save_binary( &der, sizeof(der) );

		//int a = 25;
		//text_output_archive.save_binary( &a, 4 );

		//text_output_archive.save_binary( "hi cruel world.", sizeof("hi cruel world.") );
	}

	std::ifstream ifs( archive_name );
	boost::archive::text_iarchive text_input_archive( ifs );

	derived* received_obj = nullptr;

	std::cout << "try to receive...\n";
	text_input_archive >> received_obj;

	std::cout << "received object:\n\n";
	std::cout << *received_obj;
}
