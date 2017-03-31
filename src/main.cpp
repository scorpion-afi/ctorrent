#include <iostream>
#include <fstream>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

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

	void show( void ) const override
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
	base bs( 25, 2.5, 'S' );
	derived der( "buy cruel world" );

	{
		std::ofstream ofs( archive_name );
		boost::archive::text_oarchive text_output_archive( ofs );

		text_output_archive << bs;
		text_output_archive & 10; // for output type of archive & is equal to <<
		text_output_archive << std::string( "hi cruel world" );
		text_output_archive << der;
	}

	{
		std::ifstream ifs( archive_name );
		boost::archive::text_iarchive text_input_archive( ifs );

		base restored_bs;
		int a;
		std::string str;
		derived der;

		text_input_archive >> restored_bs;
		text_input_archive & a;	// for input type of archive & is equal to >>
		text_input_archive >> str;
		text_input_archive >> der;

		restored_bs.show();
		std::cout << "a: " << a << ", str: " << str << std::endl;
		der.show();
	}

	return 0;
}
