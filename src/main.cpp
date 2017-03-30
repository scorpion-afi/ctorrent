#include <iostream>
#include <fstream>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>


class base
{
public:
	base( int i, double d, char c ) : i(i), d(d), c(c) {}
	base() : i(0), d(0.0), c('a') {}

	void show( void ) const
	{
		std::cout << "i: " << i << ", d: " << d << ", c: " << c << std::endl;
	}

private:

	/* an access class should have an access to our private method 'serialize' */
	friend class boost::serialization::access;

	template< class Archive >
	void serialize( Archive& ar, const unsigned int version )
	{
		ar & i;
		ar & d;
		ar & c;
	}

	int i;
	double d;
	char c;
};


int main( void )
{
	base bs( 25, 2.5, 'S' );

	std::ofstream ofs( "archive.txt" );

	{
		/* text_oarchive dtor destroys an ofstream object !!! */
		boost::archive::text_oarchive text_output_archive( ofs );

		text_output_archive << bs;
		text_output_archive & 10;
		text_output_archive << std::string( "hi cruel world" );
	}

	base restored_bs;

	{
		std::ifstream ifs( "archive.txt" );
		boost::archive::text_iarchive text_input_archive( ifs );

		int a; std::string str;

		text_input_archive >> restored_bs;
		text_input_archive & a;
		text_input_archive >> str;

		std::cout << "a: " << a << ", str: " << str << std::endl;
	}

	restored_bs.show();

	return 0;
}
