/*	================
 *	ProjectCommon.cc
 *	================
 */

#include "A-line/ProjectCommon.hh"

// C++
#include <algorithm>
#include <map>
#include <set>
#include <vector>

// POSIX
#include "unistd.h"

// Nucleus
#include "Nucleus/NAssert.h"
#include "Nucleus/Shared.h"

// Nitrogen
#include "Nitrogen/OSStatus.h"

// GetPathname
#include "GetPathname.hh"

// A-line
#include "A-line/BuildCommon.hh"
#include "A-line/ExtractIncludes.hh"


namespace ALine
{
	
	namespace N = Nitrogen;
	namespace NN = Nucleus;
	
	using namespace io::path_descent_operators;
	
	
	typedef std::map< ProjName, NN::Shared< Project*, NN::DisposeWithDelete > > ProjectMap;
	typedef std::set< ProjName > ProjectSet;
	typedef std::map< FileName, FSSpec > FileMap;
	typedef std::map< IncludePath, std::string > IncludeMap;
	typedef std::map< IncludePath, time_t > DateMap;
	
	static ProjectMap gProjects;
	static ProjectSet gProjectsWithIncludeDirs;
	static FileMap gRezzes;
	static IncludeMap gIncludes;
	static DateMap gDates;
	
	//static bool gNeedsCwdSourceOption = false;
	//static N::FSDirSpec gCurrentSourceDir;
	
	
	Project& GetProject( const ProjName& projName )
	{
		ProjectMap::iterator it = gProjects.find( projName );
		
		if ( it != gProjects.end() )
		{
			// We already have it
			return *it->second.Get();
		}
		
		// Load it
		NN::Owned< Project*, NN::DisposeWithDelete > newProject = NN::Owned< Project*, NN::DisposeWithDelete >::Seize( new Project( projName ) );
		Project& project = *( gProjects[ projName ] = newProject ).Get();
		
		project.Study();
		
		return project;
	}
	
	void AddIncludeDir( const ProjName& projName )
	{
		gProjectsWithIncludeDirs.insert( projName );
	}
	
	
	FSSpec RezLocation( const FileName& filename )
	{
		return gRezzes[ filename ];
	}
	
	void AddRezFile( const FileName& filename, const FSSpec& file )
	{
		gRezzes[ filename ] = file;
	}
	
	/*
	void SetCurrentSourceDir( const N::FSDirSpec& dir )
	{
		gNeedsCwdSourceOption = true;
		gCurrentSourceDir = dir;
	}
	
	void SetCurrentSourceDir( int zero )
	{
		ASSERT( zero == 0 );
		
		gNeedsCwdSourceOption = false;
	}
	*/
	
	std::string IncludeLocation( const IncludePath& includePath )
	{
		return gIncludes[ includePath ];
	}
	
	void AddInclude( const IncludePath& includePath, const std::string& file )
	{
		// Store the found location of the include specified by the pathname.
		gIncludes[ includePath ] = file;
	}
	
	bool FindInclude( const IncludePath& includePath )
	{
		// This is a nasty hack.
		// It would be nice if we didn't have to support this, but OpenSSL requires it.
		
		/*
		if ( gNeedsCwdSourceOption )
		{
			FSSpec file = gCurrentSourceDir / includePath;
			
			if ( io::item_exists( file ) )
			{
				AddInclude( includePath, file );
				return true;
			}
		}
		*/
		
		ProjectSet::const_iterator it, end = gProjectsWithIncludeDirs.end();
		
		// For each project with an include folder,
		for ( it = gProjectsWithIncludeDirs.begin();  it != end;  ++it )
		{
			const ProjName& proj = *it;
			
			// Check to see if it has the include file.
			if ( GetProject( proj ).FindInclude( includePath ) )
			{
				// If we get here, the file exists and its location is stored.
				return true;
			}
		}
		
		return false;
	}
	
	time_t RecursivelyLatestDate( const IncludePath& includePath )
	{
		if ( gIncludes.find( includePath ) != gIncludes.end() )
		{
			// Already stored
			return gDates[ includePath ];
		}
		else
		{
			// Not stored yet
			bool found = FindInclude( includePath );
			
			if ( !found )
			{
				// We can't display this warning any more because it may be a
				// case where a system include file was included as a user include.
				// Furthermore, we were getting false positives on nonexistent
				// headers that were conditionalized for other platforms.
				// Anyway, the compiler is the ultimate judge of whether an
				// error has occurred, and it will say so.
				
				//Io::Err << "  Missing include " << q( includePath ) << "\n";
				
				return 0;  // FIXME
			}
			
			return RecursivelyLatestDate( includePath, gIncludes[ includePath ] );
		}
	}
	
	/*
	static void FixNullFileType( const FSSpec& file )
	{
	#if !TARGET_API_MAC_CARBON  // FIXME:  This is a hack -- check Gestalt
		
		CInfoPBRec pb;
		
		N::FSpGetCatInfo( file, pb );
		
		FInfo& fInfo = pb.hFileInfo.ioFlFndrInfo;
		
		if ( fInfo.fdType == ::OSType( 0 ) )
		{
			fInfo.fdCreator = 'R*ch';  // FIXME:  Should be configurable
			fInfo.fdType    = 'TEXT';
			
			pb.hFileInfo.ioDirID   = file.parID;
			pb.hFileInfo.ioNamePtr = const_cast< FSSpec& >( file ).name;
			
			//N::FSpSetFileInfo( pb );
			N::ThrowOSStatus( ::PBSetCatInfoSync( &pb ) );
		}
		
	#endif
	}
	*/
	
	time_t RecursivelyLatestDate( const IncludePath& includePath, const std::string& pathname )
	{
		std::vector< IncludePath > includes = ExtractIncludes( pathname );
		
		time_t modDate = ModifiedDate( pathname );
		
		// For each included file
		std::vector< std::string >::const_iterator it, end = includes.end();
		
		for ( it = includes.begin();  it != end;  ++it )
		{
			time_t incDate = RecursivelyLatestDate( *it );
			modDate = std::max( modDate, incDate );
		}
		
		gDates[ includePath ] = modDate;
		
		return modDate;
	}
	
}

