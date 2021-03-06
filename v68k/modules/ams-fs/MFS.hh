/*
	MFS.hh
	------
*/

#ifndef MOUNT_HH
#define MOUNT_HH

// Standard
#include <stdint.h>


struct VCB;


namespace mfs
{
	
#pragma options align=packed
	
	struct f_info
	{
		uint16_t words[ 8 ];
	};
	
	struct file_directory_entry
	{
		int8_t    flAttrib;
		uint8_t   flVersNum;
		f_info    flFndrInfo;
		uint32_t  flNum;
		uint16_t  flStBlk;
		uint32_t  flLgLen;
		uint32_t  flPyLen;
		uint16_t  flRStBlk;
		uint32_t  flRLgLen;
		uint32_t  flRPyLen;
		uint32_t  flCrDat;
		uint32_t  flMdDat;
		uint8_t   flNam[ 256 ];
	};
	
#pragma options align=reset
	
}

const mfs::file_directory_entry* MFS_lookup( VCB* vcb, const uint8_t* name );

void MFS_load( VCB* vcb, uint16_t stBlk, char* buffer, int16_t n );

#endif
