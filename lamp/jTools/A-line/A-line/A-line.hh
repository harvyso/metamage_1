/*	=========
 *	A-line.hh
 *	=========
 */

#ifndef ALINE_ALINE_HH
#define ALINE_ALINE_HH

// Standard C++
#include <vector>

// plus
#include "plus/string_fwd.hh"

// A-line
#include "A-line/Task.hh"


namespace tool
{
	
	struct OptionsRecord
	{
		bool all;
		bool verbose;
		bool catalog;
		bool preprocess;
		
		OptionsRecord() : all       ( false ),
		                  verbose   ( false ),
		                  catalog   ( false ),
		                  preprocess( false )
		{
		}
	};
	
	OptionsRecord& Options();
	
	void ExecuteCommand( const TaskPtr&                     task,
	                     const plus::string&                caption,
	                     const std::vector< const char* >&  command,
	                     const char*                        diagnostics_file_path = NULL );
	
	void check_diagnostics( bool succeeded, const char* diagnostics_path );
	
}

#endif
