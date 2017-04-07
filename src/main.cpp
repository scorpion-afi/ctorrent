#include <iostream>
#include <fstream>

#include <memory>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include <boost/serialization/base_object.hpp>

const std::string archive_name( "archive.txt" );


class base
{
public:
	base( int i, double d, char c ) : i(i), d(d), c(c) {}
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
		std::cout << "serialization for base object...\n";

		/* load order is equal to store order */
		ar & i;
		ar & d;
		ar & c;
	}

	int i;
	double d;
	char c;
};

std::ostream& operator<<( std::ostream& cout, const base& bs )
{
	bs.show();
	return std::cout;
}

class derived : public base
{
public:
	explicit derived( int num ) : base(3, 2.5, 'b'), hi(num) {}
	derived() = default;

	void show( void ) const override
	{
		std::cout << "derived object:\n ";
		base::show();
		std::cout << " hi: " << hi << std::endl;
	}

private:

	friend class boost::serialization::access;

	template< class Archive >
	void serialize( Archive& ar, const unsigned int version )
	{
		/* an insane way to serialize base part of derived object */
		ar & boost::serialization::base_object<base>( *this );

		std::cout << "serialization for derived object...\n";
		ar & hi;
	}

	int hi;
};


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
	}

	std::ifstream ifs( archive_name );
	boost::archive::text_iarchive text_input_archive( ifs );

	text_input_archive.register_type( static_cast<derived*>(nullptr) );

	derived* received_obj = nullptr;

	std::cout << "try to receive...\n";
	text_input_archive >> received_obj;

	std::cout << "received object:\n\n";
	std::cout << *received_obj;
}
