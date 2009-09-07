/*	=========
 *	select.cc
 *	=========
 */

// Standard C++
#include <algorithm>
#include <map>
#include <string>
#include <vector>

// Standard C/C++
#include <cstdio>

// POSIX
#include <sys/select.h>

// Iota
#include "iota/strings.hh"

// Nucleus
#include "Nucleus/NAssert.h"

// POSeven
#include "POSeven/functions/open.hh"
#include "POSeven/functions/perror.hh"
#include "POSeven/types/exit_t.hh"

// Orion
#include "Orion/get_options.hh"
#include "Orion/Main.hh"


namespace tool
{
	
	namespace p7 = poseven;
	namespace o = orion;
	
	
	static int Select( const std::vector< std::string >& read_files, bool only_one )
	{
		int maxFD = -1;
		
		fd_set read_fds;
		
		FD_ZERO( &read_fds );
		
		std::map< int, const char* > name_of;
		
		typedef std::vector< std::string >::const_iterator const_iterator;
		
		for ( const_iterator it = read_files.begin();  it != read_files.end();  ++it )
		{
			const char* name = it->c_str();
			
			p7::fd_t fd = p7::open( name, p7::o_rdonly ).release();
			
			FD_SET( fd, &read_fds );
			
			maxFD = std::max< int >( maxFD, fd );
			
			name_of[ fd ] = name;
		}
		
		// This blocks and yields to other threads
		int selected = select( maxFD + 1, &read_fds, NULL, NULL, NULL );
		
		if ( selected == -1 )
		{
			p7::perror( "select: select()" );
			
			return 3;  // error from select()
		}
		
		if ( selected == 0 )
		{
			return 1;  // timeout
		}
		
		if ( selected > 1  &&  only_one )
		{
			return 2;  // multiple, maybe a window closed
		}
		
		for ( int reader = 0;  reader <= maxFD;  ++reader )
		{
			if ( !FD_ISSET( reader, &read_fds ) )  continue;
			
			const char* name = name_of[ reader ];
			
			std::printf( "%s\n", name );
		}
		
		return 0;
	}
	
	
	int Main( int argc, iota::argv_t argv )
	{
		bool only_one = false;
		
		std::vector< std::string > readers;
		
		o::bind_option_to_variable( "-1", only_one );
		o::bind_option_to_variable( "-r", readers  );
		
		o::get_options( argc, argv );
		
		//char const *const *free_args = o::free_arguments();
		
		//const std::size_t n_args = o::free_argument_count();
		
		return Select( readers, only_one );
	}
	
}

