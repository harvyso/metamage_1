/*	========
 *	netdb.cc
 *	========
 */

// Mac OS X
#ifdef __APPLE__
#include <CoreServices/CoreServices.h>
#endif

#ifndef MAC_OS_X_VERSION_10_8

// Mac OS
#ifndef __OPENTRANSPORT__
#include <OpenTransport.h>
#endif

// POSIX
#include "netdb.h"

// mac-app-utils
#include "mac_app/OpenTransport_share.hh"

// relix
#include "relix/syscall/registry.hh"

// ClassicToolbox
#include "ClassicToolbox/OpenTransportProviders.hh"

// Genie
#include "Genie/current_process.hh"
#include "Genie/api/yield.hh"


namespace Genie
{
	
	namespace n = nucleus;
	namespace N = Nitrogen;
	
	using mac::app::OpenTransport_share;
	
	
	struct netdb_provider_data
	{
		OTEventCode  code;
		OTResult     result;
	};
	
	static pascal void netdb_notifier( void*        context,
	                                   OTEventCode  code,
	                                   OTResult     result,
	                                   void*        cookie )
	{
		netdb_provider_data* data = (netdb_provider_data*) context;
		
		if ( data == NULL )
		{
			return;
		}
		
		switch ( code )
		{
			case T_DNRSTRINGTOADDRCOMPLETE:
			case T_DNRMAILEXCHANGECOMPLETE:
				data->code   = code;
				data->result = result;
				break;
			
			default:
				break;
		}
	}
	
	
	static n::owned< InetSvcRef > InternetServices( netdb_provider_data& data )
	{
		static OTNotifyUPP gNotifyUPP = ::NewOTNotifyUPP( netdb_notifier );
		
		n::owned< InetSvcRef > provider = N::OTOpenInternetServices( kDefaultInternetServicesPath );
		
		N::OTInstallNotifier( provider, gNotifyUPP, &data );
		
		N::OTSetAsynchronous( provider );
		
		return provider;
	}
	
	
	static OSStatus _OTInetStringToAddress( char* name, struct InetHostInfo* result )
	{
		try
		{
			OpenTransport_share shared_OpenTransport;
			
			netdb_provider_data data = { 0 };
			
			n::owned< InetSvcRef > services = InternetServices( data );
			
			const OSStatus err = ::OTInetStringToAddress( services,
			                                              name,
			                                              result );
			
			if ( err != noErr )
			{
				return err;
			}
			
			while ( data.code == 0 )
			{
				try_again( false );
			}
			
			return data.result;
		}
		catch ( ... )
		{
			return set_errno_from_exception();
		}
	}
	
	static OSStatus _OTInetMailExchange( char* domain, UInt16* count, InetMailExchange* result )
	{
		try
		{
			OpenTransport_share shared_OpenTransport;
			
			netdb_provider_data data = { 0 };
			
			n::owned< InetSvcRef > services = InternetServices( data );
			
			const OSStatus err = ::OTInetMailExchange( services,
			                                           domain,
			                                           count,
			                                           result );
			
			if ( err != noErr )
			{
				return err;
			}
			
			while ( data.code == 0 )
			{
				try_again( false );
			}
			
			return data.result;
		}
		catch ( ... )
		{
			return set_errno_from_exception();
		}
	}
	
	#pragma force_active on
	
	REGISTER_SYSTEM_CALL( _OTInetStringToAddress );
	
#if !TARGET_API_MAC_CARBON
	
	REGISTER_SYSTEM_CALL( _OTInetMailExchange );
	
#endif
	
	#pragma force_active reset
	
}

#endif  // #ifndef MAC_OS_X_VERSION_10_8
