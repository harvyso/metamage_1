/*	===============
 *	FSRead_Async.hh
 *	===============
 */

#ifndef MACIO_FSREADASYNC_HH
#define MACIO_FSREADASYNC_HH

// Nitrogen
#include "Nitrogen/Files.h"

// MacIO
#include "MacIO/Async.hh"
#include "MacIO/EOF_Policy.hh"
#include "MacIO/Init_IOParam.hh"
#include "MacIO/Routines.hh"
#include "MacIO/ThrowOSStatus.hh"


namespace MacIO
{
	
	using Nitrogen::FSFileRefNum;
	using Nitrogen::FSIOPosMode;
	
	
	template < class Callback >
	inline SInt32 FSRead( EOF_Policy         policy,
	                      FSFileRefNum       file,
	                      FSIOPosMode        positionMode,
	                      SInt32             positionOffset,
	                      SInt32             requestCount,
	                      void *             buffer,
	                      Callback           callback,
	                      ::IOCompletionUPP  completion = NULL )
	{
		ParamBlockRec pb;
		
		Init_PB_For_ReadWrite( pb,
		                       file,
		                       positionMode,
		                       positionOffset,
		                       requestCount,
		                       buffer );
		
		const bool ok = PBAsync< Read_Traits, Return_EOF >( pb,
		                                                    callback,
		                                                    completion );
		
		if ( ok + !!pb.ioParam.ioActCount < policy )
		{
			throw Nitrogen::EOFErr();
		}
		
		return pb.ioParam.ioActCount;
	}
	
	// Default position mode
	template < class Callback >
	inline SInt32 FSRead( EOF_Policy         policy,
	                      FSFileRefNum       file,
	                      SInt32             requestCount,
	                      void *             buffer,
	                      Callback           callback,
	                      ::IOCompletionUPP  completion = NULL )
	{
		return FSRead( policy,
		               file,
		               fsAtMark,
		               0,
		               requestCount,
		               buffer,
		               callback,
		               completion );
	}
	
}

#endif

