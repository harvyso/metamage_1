/*	=======
 *	vfork.h
 *	=======
 */

#ifndef KEROSENE_VFORK_H
#define KEROSENE_VFORK_H

// Standard C
#include <setjmp.h>

// POSIX
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif
	
	void Kerosene_LongJmp( int result );
	
	int SpawnVFork( void (*LongJmp)( int ) );  // Only returns 0 or -1
	
	jmp_buf* NewJmpBuf();
	
	#define VFORK() ( SpawnVFork( Kerosene_LongJmp ) ? -1 : setjmp( *NewJmpBuf() ) )
	
	#ifdef __cplusplus
		
		inline pid_t vfork()  { return VFORK(); }
		
	#else
		
		#define vfork()  ( (pid_t) Kerosene_vfork() )
		
	#endif
	
#ifdef __cplusplus
}
#endif

#endif

