#include <iostream>
#include <streambuf>

#include <list>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

#include <boost/serialization/list.hpp>

#include <boost/serialization/base_object.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/split_member.hpp>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

class base
{
public:
	base( int i, double d, char c ) : i(i), d(d), c(c)
    {
		int_list.push_back( 25 ); /* int_list is default-initialized */
    }

	base() : i(0), d(0.0), c('a') {} /* int_list is default-initialized */

	base( const base& ref ) = default;
	base( base&& ref ) = delete;

	base& operator=( const base& ref ) = default;
	base& operator=( base&& ref ) = delete;

	virtual ~base() {}

	virtual void show( std::ostream& cout = std::cout ) const
	{
		cout << " base object: " << "i: " << i << ", d: " << d << ", c: " << c << std::endl;
	}

private:

	friend std::ostream& operator<<( std::ostream& cout, const base& bs );

	/* an 'access' class should have an access to our private method 'serialize' */
	friend class boost::serialization::access;

	template< class Archive >
	void serialize( Archive& ar, const unsigned int version )
	{
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

class derived : public base
{
public:
	explicit derived( int num ) : base(3, 2.5, 'b'), hi(num), dd(8)	{}

	derived() = default;

	derived( const derived& ref ) = default;
	derived( derived&& ref ) = delete;

	derived& operator=( const derived& ref ) = default;
	derived& operator=( derived&& ref ) = delete;

	void show( std::ostream& cout = std::cout) const override
	{
		cout << " derived object:\n ";
		base::show( cout );
		cout << "  hi: " << hi << ", dd: " << dd << std::endl;
	}

private:

	friend class boost::serialization::access;

	template< class Archive >
	void save( Archive& ar, const unsigned int version ) const
	{
		/* an insane way to serialize base part of derived object */
		ar & boost::serialization::base_object<base>(*this);

		ar & hi;

		/* we got to write to archive anyway */
		ar & dd;
	}

	template< class Archive >
	void load( Archive& ar, const unsigned int version )
	{
		ar & boost::serialization::base_object<base>( *this );

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
	pid_t child_pid;
	int sockets[2];
	int ret;

	/* create stream-based connected sockets pair in PF_LOCAL domain */
	ret = socketpair( AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, sockets );
	if( ret < 0 )
	{
		std::cout << "an attempt to create socketpair failed.\n";
		return 1;
	}

	child_pid = fork();
	if( child_pid < 0 )
	{
		std::cout << "an attempt to fork() failed.\n";
		for( int fd : sockets )
			close( fd );

		return 1;
	}

	/* after fork child process has sockets[2] fds which refer to the same socket objects,
	 * as parent refers to */

	/* parent process */
	if( child_pid )
	{
		raw_memory_stream_buff raw_stream_buf;
		boost::archive::binary_oarchive raw_ar( raw_stream_buf );

		std::shared_ptr<base> bs = std::make_shared<derived>( 26 );
		std::cout << "object to transfer:\n" << *bs;

		std::cout << "--------serialization start---------\n";

		/* we got to register derived class to be able to save polymorphic pointers */
		/* TODO: we have another way to do this, but now I don't wanna spent time to figure out
		 *       how to do that */
		raw_ar.register_type( static_cast<derived*>(nullptr) );
		raw_ar << bs.get();

		std::cout << "-------serialization finish---------\n";

		const std::vector<char>& sered_obj = raw_stream_buf.upload_ref();

		ret = send( sockets[0], &sered_obj.front(), sered_obj.size(), 0 );
		if( ret < 0 )
		{
			std::cout << "an attempt to send() failed.\n";
			return 1;
		}

		usleep( 20000 );
	}
	else
	{
		char data[4096];
		int size;

		size = recv( sockets[1], data, sizeof(data), 0 );
		if( size < 0 )
		{
			std::cout << "an attempt to recv() failed.\n";
			return 1;
		}

		raw_memory_stream_buff raw_stream_buf( std::vector<char>( &data[0], &data[size] ) );
		boost::archive::binary_iarchive raw_ar( raw_stream_buf );

		derived* received_obj = nullptr;

		std::cout << "\n--------deserialization start-----------\n";

		raw_ar >> received_obj;

		std::cout << "---------deserialization finish---------\n";

		std::cout << "received object:\n";
		std::cout << *received_obj;
	}
}
