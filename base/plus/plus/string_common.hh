/*
	string_common.hh
	----------------
*/

#ifndef PLUS_STRINGCOMMON_HH
#define PLUS_STRINGCOMMON_HH


namespace plus
{
	
	struct datum_storage;
	
	void string_check_size( long size );
	
	void string_reserve( datum_storage& datum, long capacity );
	
	void string_set_length( datum_storage& datum, long length, char* data );
	
}

#endif
