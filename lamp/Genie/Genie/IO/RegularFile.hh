/*	==============
 *	RegularFile.hh
 *	==============
 */

#ifndef GENIE_IO_REGULARFILE_HH
#define GENIE_IO_REGULARFILE_HH

 // Genie
 #include "Genie/IO/Stream.hh"


namespace Genie
{
	
	class RegularFileHandle : public StreamHandle
	{
		private:
			off_t itsMark;
		
		public:
			RegularFileHandle( int                                flags,
			                   const vfs::filehandle_method_set*  methods = NULL );
			
			RegularFileHandle( const vfs::node_ptr&               file,
			                   int                                flags,
			                   const vfs::filehandle_method_set*  methods = NULL );
			
			virtual ~RegularFileHandle();
			
			off_t get_mark() const  { return itsMark; }
			
			off_t set_mark( off_t mark )  { return itsMark = mark; }
			
			ssize_t advance_mark( ssize_t delta )  { itsMark += delta;  return delta; }
			
			bool IsRegularFile() const  { return true; }
			
			virtual IOPtr Clone() = 0;
			
			virtual unsigned int SysPoll()  { return kPollRead | kPollWrite; }
			
			virtual ssize_t Append( const char* buffer, size_t n_bytes );
			
			ssize_t SysRead( char* buffer, size_t n_bytes );
			
			ssize_t SysWrite( const char* buffer, size_t n_bytes );
			
			off_t Seek( off_t offset, int whence );
			
			ssize_t Write( const char* buffer, std::size_t byteCount );
			
			memory_mapping_ptr Map( size_t length, int prot, int flags, off_t offset );
	};
	
	template <> struct IOHandle_Downcast_Traits< RegularFileHandle >
	{
		static IOHandle::Test GetTest()  { return &IOHandle::IsRegularFile; }
		
		static int GetError( IOHandle& handle )
		{
			return IOHandle_Downcast_Traits< StreamHandle >::GetError( handle );
		}
	};
	
}

#endif

