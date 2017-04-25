#include <iostream>
#include <fstream>
#include <streambuf>
#include <sstream>

#include <list>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

#include <boost/serialization/list.hpp>

#include <boost/serialization/base_object.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/split_member.hpp>


const std::string archive_name( "archive.txt" );

class base
{
public:
	base( int i, double d, char c ) : i(i), d(d), c(c)
    {
		int_list.push_back( 25 ); /* int_list is default-initialized */
		std::cout << "this ptr(base ctor): " << static_cast<void*>(this) << std::endl;
    }

	base() : i(0), d(0.0), c('a') {} /* int_list is default-initialized */

	base( const base& ref ) = default;
	base( base&& ref ) = delete;

	base& operator=( const base& ref ) = default;
	base& operator=( base&& ref ) = delete;

	virtual ~base() { std::cout << "I'm base dtor.\n"; }

	virtual void show( std::ostream& cout = std::cout ) const
	{
		cout << "this ptr(base show): " << static_cast<void*>(const_cast<base*>(this)) << std::endl;
		this->kk( cout );
		cout << "base object: " << "i: " << i << ", d: " << d << ", c: " << c << std::endl;
	}

	virtual void kk( std::ostream& cout = std::cout ) const { cout << "I'm base.\n"; }

private:

	friend std::ostream& operator<<( std::ostream& cout, const base& bs );

	/* an 'access' class should have an access to our private method 'serialize' */
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
	bs.show( cout );
	return cout;
}

std::ostream& operator<<( std::ostream& cout, const base *bs )
{
	return cout << *bs;
}

class derived : public base
{
public:
	explicit derived( int num ) : base(3, 2.5, 'b'), hi(num), dd(8)
	{
		std::cout << "this ptr(derived ctor): " << static_cast<void*>(this) << std::endl;
	}

	derived() = default;

	derived( const derived& ref ) = default;
	derived( derived&& ref ) = delete;

	derived& operator=( const derived& ref ) = default;
	derived& operator=( derived&& ref ) = delete;

	~derived() { std::cout << "I'm derived dtor.\n"; }

	void show( std::ostream& cout = std::cout) const override
	{
		cout << "this ptr(derived show): " << static_cast<void*>(const_cast<derived*>(this)) << std::endl;

		cout << "derived object:\n ";
		base::show( cout );
		cout << " hi: " << hi << ", dd: " << dd << std::endl;
	}

	void kk( std::ostream& cout = std::cout ) const override { cout << "I'm derived.\n"; }

private:

	friend class boost::serialization::access;

	template< class Archive >
	void save( Archive& ar, const unsigned int version ) const
	{
		std::cout << "save for derived\n";

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


/* raw memory based streambuffer implementation
 *
 * streambuffer implementation with std::vector<char> as an associated character sequence,
 * controlled character sequences aren't used */
class raw_memory_stream_buff : public std::streambuf
{
public:
	raw_memory_stream_buff() :
		read_offset(0) { buff.reserve(memory_reserv); }

	/* initialize an associated character sequence by copy of data */
	explicit raw_memory_stream_buff( const std::vector<char>& data ) :
		buff(data), read_offset(0) {}

	/* The next functions provide interface to read/write data directly,
	 * not over streambuf interface, it can be useful, e.g. for socket communication */

	/* get */
	std::vector<char> upload( void ) const { return buff; }
	const std::vector<char>& upload_ref( void ) const { return buff; }

	/* set */
	void load( const std::vector<char>& data )
	{
		buff = data;
		read_offset = 0;
	}

private:

	/* don't support copy semantic */
	raw_memory_stream_buff( const raw_memory_stream_buff& ref ) = delete;
	raw_memory_stream_buff& operator=( const raw_memory_stream_buff& ref ) = delete;

	/* these function will be invoked for every symbol to write/read as I've not called
	 * setg(), setp() functions */
	int_type overflow( int_type ch ) override;
	int_type uflow( void ) override;


	const int memory_reserv = 4096;

	/* associated character sequence */
	std::vector<char> buff;

	unsigned int read_offset;
};

raw_memory_stream_buff::int_type raw_memory_stream_buff::uflow( void )
{
	raw_memory_stream_buff::int_type ret;

	/* I don't reset vector after reading has been completed,
	 * 'cause it's not a circle buffer implementation */
	if( buff.begin() + read_offset >= buff.end() )
		return traits_type::eof();

	ret = traits_type::to_int_type( *(buff.begin() + read_offset) );
	read_offset++;

	return ret;
}

raw_memory_stream_buff::int_type raw_memory_stream_buff::overflow( int_type ch )
{
	if( ch == traits_type::eof() )
		return traits_type::eof();

	buff.push_back( ch );

	return ch;
}


int main( void )
{
	std::stringstream string_stream;

	std::basic_stringbuf<char> string_buf( std::ios_base::out | std::ios_base::in );

	raw_memory_stream_buff raw_stream_buf;

	{
		std::shared_ptr<base> bs = std::make_shared<derived>( 26 );
		std::cout << bs;

		std::cout << "--------serialization start---------\n";
		std::cout << "-----------std::ofstream------------\n";

		std::ofstream ofs( archive_name );
		boost::archive::text_oarchive ofs_ar( ofs );

		/* we got to register derived class to be able to save polymorphic pointers */
		/* TODO: we have another way to do this, but now I don't wanna spent time to figure out
		 *       how to do that */
		ofs_ar.register_type( static_cast<derived*>(nullptr) );
		ofs_ar << bs.get();

		std::cout << "---------std::stringstream----------\n";

		boost::archive::binary_oarchive string_stream_ar( string_stream );

		string_stream_ar.register_type( static_cast<derived*>(nullptr) );
		string_stream_ar << bs.get();

		std::cout << "----std::basic_stringbuf<char>------\n";

		boost::archive::binary_oarchive strbuf_ar( string_buf );

		strbuf_ar.register_type( static_cast<derived*>(nullptr) );
		strbuf_ar << bs.get();

		std::cout << "------raw_memory_stream_buff--------\n";

		boost::archive::binary_oarchive raw_ar( raw_stream_buf );

		raw_ar.register_type( static_cast<derived*>(nullptr) );
		raw_ar << bs.get();

		std::cout << "-------serialization finish---------\n";
	}

	derived* received_obj = nullptr;

	std::cout << "\n--------deserialization start-----------\n";
	std::cout << "-----------std::ofstream----------------\n";

	std::ifstream ifs( archive_name );
	boost::archive::text_iarchive ifs_ar( ifs );

	ifs_ar >> received_obj;
	std::cout << "received object:\n\n";
	std::cout << *received_obj;

	received_obj->~base();
	received_obj = nullptr;

	std::cout << "-----------std::stringstream------------\n";

	boost::archive::binary_iarchive string_stream_ar( string_stream );

	string_stream_ar >> received_obj;
	std::cout << "received object:\n\n";
	std::cout << *received_obj;

	received_obj->~base();
	received_obj = nullptr;

	std::cout << "-------std::basic_stringbuf<char>-------\n";

	boost::archive::binary_iarchive strbuf_ar( string_buf );

	strbuf_ar >> received_obj;
	std::cout << "received object:\n\n";
	std::cout << *received_obj;

	received_obj->~base();
	received_obj = nullptr;

	std::cout << "--------raw_memory_stream_buff----------\n";

	boost::archive::binary_iarchive raw_ar( raw_stream_buf );

	raw_ar >> received_obj;
	std::cout << "received object:\n\n";
	std::cout << *received_obj;

	received_obj->~base();
	received_obj = nullptr;

	std::cout << "---------deserialization finish---------\n";
}
