/*
	get_process_group_in_session.cc
	-------------------------------
*/

#include "relix/api/get_process_group_in_session.hh"

// poseven
#include "poseven/types/errno_t.hh"

// relix-api
#include "relix/api/get_process_group.hh"

// relix-kernel
#include "relix/task/process_group.hh"


namespace relix
{
	
	namespace p7 = poseven;
	
	
	boost::intrusive_ptr< process_group >
	//
	get_process_group_in_session( pid_t pgid, session& s )
	{
		process_group* pgrp = get_process_group( pgid );
		
		if ( pgrp == NULL )
		{
			return new process_group( pgid, s );
		}
		
		if ( &pgrp->get_session() != &s )
		{
			p7::throw_errno( EPERM );
		}
		
		return pgrp;
	}
	
}
