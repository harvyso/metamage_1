/*
	Genie/FS/sys/mac/desktop.cc
	---------------------------
*/

#include "Genie/FS/sys/mac/desktop.hh"

// plus
#include "plus/var_string.hh"

// Nitrogen
#include "Nitrogen/Quickdraw.hh"

// vfs
#include "vfs/property.hh"
#include "vfs/node/types/property_file.hh"

// Genie
#include "Genie/FS/serialize_qd.hh"


namespace Genie
{
	
	namespace N = Nitrogen;
	
	
	struct GetScreenBounds : serialize_Rect
	{
		typedef const Rect& Result;
		
		static Result Get( const BitMap& screenBits )
		{
			return screenBits.bounds;
		}
	};
	
	struct GetScreenSize : serialize_Point
	{
		typedef Point Result;
		
		static Result Get( const BitMap& screenBits )
		{
			const Rect& bounds = screenBits.bounds;
			
			const Point size = { bounds.bottom - bounds.top,
			                     bounds.right - bounds.left };
			
			return size;
		}
	};
	
	template < class Accessor >
	struct sys_mac_desktop_Property : vfs::readonly_property
	{
		static const int fixed_size = Accessor::fixed_size;
		
		static void get( plus::var_string& result, const vfs::node* that, bool binary )
		{
			const BitMap& screenBits = N::GetQDGlobalsScreenBits();
			
			const typename Accessor::Result data = Accessor::Get( screenBits );
			
			Accessor::deconstruct::apply( result, data, binary );
		}
	};
	
	#define PROPERTY( prop )  &vfs::new_property, &vfs::property_params_factory< sys_mac_desktop_Property< prop > >::value
	
	const vfs::fixed_mapping sys_mac_desktop_Mappings[] =
	{
		{ "bounds", PROPERTY( GetScreenBounds ) },
		{ "size",   PROPERTY( GetScreenSize   ) },
		
		{ NULL, NULL }
	};
	
}
