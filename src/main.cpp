#include <iostream>
#include <fstream>

#include <memory>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>


const std::string archive_name( "archive.txt" );


class base
{
public:
	base( int i, double d, char c ) : i(i), d(d), c(c) {}
	base() : i(0), d(0.0), c('a') {}
	virtual ~base() {}

	void show( void ) const
	{
		std::cout << "base object: " << "i: " << i << ", d: " << d << ", c: " << c << std::endl;
	}

private:

	/* an access class should have an access to our private method 'serialize' */
	friend class boost::serialization::access;

	template< class Archive >
	void serialize( Archive& ar, const unsigned int version )
	{
		/* load order is equal to store order */
		ar & i;
		ar & d;
		ar & c;
	}

	int i;
	double d;
	char c;
};

class derived : public base
{
public:
	explicit derived( std::string&& str ) : base(3, 2.5, 'b'), str(str) {}
	derived() {}

	void show( void ) const
	{
		std::cout << "derived object:\n ";
		base::show();
		std::cout << " str: " << str << std::endl;
	}

private:

	friend class boost::serialization::access;

	template< class Archive >
	void serialize( Archive& ar, const unsigned int version )
	{
		/* an insane way to serialize base part of derived object */
		ar & boost::serialization::base_object<base>( *this );
		ar & str;
	}

	std::string str;
};


int main( void )
{
	{
		std::ofstream ofs( archive_name );
		boost::archive::text_oarchive text_output_archive( ofs );

		std::shared_ptr<base> bs = std::make_shared<base>( 10, 2.5, 's' );
		bs->show();

		text_output_archive << bs.get();
	}

	std::ifstream ifs( archive_name );
	boost::archive::text_iarchive text_input_archive( ifs );

	base* received_bs;
	text_input_archive >> received_bs;

	received_bs->show();
}
