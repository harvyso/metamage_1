#if defined(__STDPP__directive) && defined(__STDPP__hide)
__STDPP__directive pragma pp:hide getpagesize
#else
#define getpagesize	______getpagesize
#endif

#include	"sfhdr.h"

#if defined(__STDPP__directive) && defined(__STDPP__hide)
__STDPP__directive pragma pp:nohide getpagesize
#else
#undef	getpagesize
#endif

#if _lib_getpagesize
_BEGIN_EXTERNS_
extern int	getpagesize _ARG_((void));
_END_EXTERNS_
#endif

/*	Set a (new) buffer for a stream.
**	If size < 0, it is assigned a suitable value depending on the
**	kind of stream. The actual buffer size allocated is dependent
**	on how much memory is available.
**
**	Written by Kiem-Phong Vo (06/27/90)
*/

#if !_sys_stat
struct stat
{	int	st_mode;
	int	st_size;
};
#define fstat(fd,st)	(-1)
#endif /*_sys_stat*/

#if __STD_C
Void_t* sfsetbuf(reg Sfio_t* f, reg Void_t* buf, reg size_t size)
#else
Void_t* sfsetbuf(f,buf,size)
reg Sfio_t*	f;	/* stream to be buffered */
reg Void_t*	buf;	/* new buffer */
reg size_t	size;	/* buffer size, -1 for default size */
#endif
{
	reg int		sf_malloc;
	reg uchar*	obuf;
	reg Sfdisc_t*	disc;
	reg ssize_t	osize, blksize;
	reg int		oflags, justopen, init, okmmap, local;
	Stat_t		st;

	GETLOCAL(f,local);

	if(size == 0 && buf)
	{	/* special case to get buffer info */
		_Sfi = f->val = (f->bits&SF_MMAP) ? (f->endb-f->data) : f->size;
		return (Void_t*)f->data;
	}

	/* cleanup actions already done, don't allow write buffering any more */
	if(_Sfexiting && !(f->flags&SF_STRING) && (f->mode&SF_WRITE))
		return NIL(Void_t*);

	if((init = f->mode&SF_INIT) )
	{	if(!f->pool && _sfsetpool(f) < 0)
			return NIL(Void_t*);
	}
	else if((f->mode&SF_RDWR) != SFMODE(f,local) && _sfmode(f,0,local) < 0)
		return NIL(Void_t*);

	justopen = f->mode&SF_OPEN; f->mode &= ~SF_OPEN;
	if(init)
		f->mode = (f->mode&SF_RDWR)|SF_LOCK;
	else
	{	int	rv;

		/* make sure there is no hidden read data */
		if((f->bits&SF_PROCESS) && (f->flags&SF_READ) && (f->mode&SF_WRITE) &&
		   _sfmode(f,SF_READ,local) < 0)
			return NIL(Void_t*);

		/* synchronize first */
		SFLOCK(f,local); rv = SFSYNC(f); SFOPEN(f,local);
		if(rv < 0)
			return NIL(Void_t*);

		/* turn off the SF_SYNCED bit because buffer is changing */
		f->mode &= ~SF_SYNCED;
	}

	SFLOCK(f,local);

	blksize = 0;
	oflags = f->flags;

	/* see if memory mapping is possible (see sfwrite for SF_BOTH) */
	okmmap = (buf || (f->flags&SF_STRING) || (f->flags&SF_RDWR) == SF_RDWR) ? 0 : 1;

	/* save old buffer info */
#ifdef MAP_TYPE
	if(f->bits&SF_MMAP)
	{	if(f->data)
		{	SFMUNMAP(f,f->data,f->endb-f->data);
			f->data = NIL(uchar*);
		}
	} else
#endif
	if(f->data == f->tiny)
	{	f->data = NIL(uchar*);
		f->size = 0;
	}
	obuf  = f->data;
	osize = f->size;

	f->flags &= ~SF_MALLOC;
	f->bits  &= ~SF_MMAP;

	/* pure read/string streams must have a valid string */
	if((f->flags&(SF_RDWR|SF_STRING)) == SF_RDSTR &&
	   (size == (size_t)SF_UNBOUND || !buf))
		size = 0;

	/* set disc to the first discipline with a seekf */
	for(disc = f->disc; disc; disc = disc->disc)
		if(disc->seekf)
			break;

	if((init || local) && !(f->flags&SF_STRING))
	{	/* ASSERT(f->file >= 0) */
		st.st_mode = 0;

		/* if has discipline, set size by discipline if possible */
		if(!_sys_stat || disc)
		{	if((f->here = SFSK(f,(Sfoff_t)0,1,disc)) < 0)
				goto unseekable;
			else
			{	Sfoff_t	e;
				if((e = SFSK(f,(Sfoff_t)0,2,disc)) >= 0)
					f->extent = e > f->here ? e : f->here;
				(void)SFSK(f,f->here,0,disc);
				goto setbuf;
			}
		}

		/* get file descriptor status */
		if(fstat((int)f->file,&st) < 0)
			f->here = -1;
		else
		{
#if _sys_stat && _stat_blksize	/* preferred io block size */
			if((blksize = (ssize_t)st.st_blksize) > 0)
				while((blksize + (ssize_t)st.st_blksize) <= SF_PAGE)
					blksize += (ssize_t)st.st_blksize;
#endif
			if(S_ISDIR(st.st_mode) || (int)st.st_size < SF_GRAIN)
				okmmap = 0;
			if(S_ISREG(st.st_mode) || S_ISDIR(st.st_mode))
				f->here = justopen ? 0 : SFSK(f,(Sfoff_t)0,1,f->disc);
			else	f->here = -1;
		}

		if(f->here >= 0)
		{	f->extent = (Sfoff_t)st.st_size;

			/* seekable std-devices are share-public by default */
			if(f == sfstdin || f == sfstdout || f == sfstderr)
				f->flags |= SF_SHARE|SF_PUBLIC;
		}
		else
		{
		unseekable:
			f->extent = -1;
			f->here = 0;

			if(init)
			{	if(S_ISCHR(st.st_mode) )
				{	blksize = SF_GRAIN;

					/* set line mode for terminals */
					if(!(f->flags&SF_LINE) && isatty(f->file))
						f->flags |= SF_LINE;
#if _sys_stat
					else	/* special case /dev/null */
					{	reg int	dev, ino;
						dev = (int)st.st_dev;	
						ino = (int)st.st_ino;	
						if(stat(DEVNULL,&st) >= 0 &&
						   dev == (int)st.st_dev &&
						   ino == (int)st.st_ino)
							SFSETNULL(f);
					}
#endif
				}

				/* initialize save input buffer for r+w streams */
				if(!(f->bits&SF_PROCESS) && (f->bits&SF_BOTH) )
					(void)_sfpopen(f,-1,-1);
			}
		}

		/* set page size, this is also the desired default buffer size */
#if _lib_getpagesize
		if(_Sfpage <= 0)
			_Sfpage = (size_t)getpagesize();
#endif
		if(_Sfpage <= 0)
			_Sfpage = SF_PAGE;
	}

#ifdef MAP_TYPE
	if(okmmap && size && (f->mode&SF_READ) && f->extent >= 0 )
	{	/* see if we can try memory mapping */
		if(!disc)
			for(disc = f->disc; disc; disc = disc->disc)
				if(disc->readf)
					break;
		if(!disc)
		{	f->bits |= SF_MMAP;
			if(size == (size_t)SF_UNBOUND)
				size = _Sfpage * SF_NMAP;
		}
	}
#endif

	/* get buffer space */
setbuf:
	if(size == (size_t)SF_UNBOUND)
	{	/* define a default size suitable for block transfer */
		if((init || local) && osize > 0)
			size = osize;
		else if(f == sfstderr && (f->mode&SF_WRITE))
			size = 0;
		else if(f->flags&SF_STRING )
			size = SF_GRAIN;
		else if((f->flags&SF_READ) && !(f->bits&SF_BOTH) &&
			f->extent > 0 && f->extent < (Sfoff_t)_Sfpage )
			size = (((size_t)f->extent + SF_GRAIN-1)/SF_GRAIN)*SF_GRAIN;
		else if((size = _Sfpage) < blksize)
			size = blksize;

		buf = NIL(Void_t*);
	}

	sf_malloc = 0;
	if(size > 0 && !buf && !(f->bits&SF_MMAP))
	{	/* try to allocate a buffer */
		if(obuf && size == (size_t)osize && local)
		{	buf = (Void_t*)obuf;
			obuf = NIL(uchar*);
			sf_malloc = (oflags&SF_MALLOC);
		}
		if(!buf)
		{	/* do allocation */
#if _lib_memalign
			if(!(f->flags&SF_STRING) && _Sfpage && (size % _Sfpage) == 0)
				buf = (Void_t*)memalign(_Sfpage,size);
#endif
			while(!buf && size > 0)
			{	if((buf = (Void_t*)malloc(size)) )
					break;
				else	size /= 2;
			}
			if(size > 0)
				sf_malloc = SF_MALLOC;
		}
	}

	if(size == 0 && !(f->flags&SF_STRING) && !(f->bits&SF_MMAP) && (f->mode&SF_READ))
	{	/* use the internal buffer */
		size = sizeof(f->tiny);
		buf = (Void_t*)f->tiny;
	}

	/* set up new buffer */
	f->size = size;
	f->next = f->data = f->endr = f->endw = (uchar*)buf;
	f->endb = (f->mode&SF_READ) ? f->data : f->data+size;
	if(f->flags&SF_STRING)
	{	/* these fields are used to test actual size - see sfseek() */
		f->extent = (!sf_malloc &&
			     ((f->flags&SF_READ) || (f->bits&SF_BOTH)) ) ? size : 0;
		f->here = 0;

		/* read+string stream should have all data available */
		if((f->mode&SF_READ) && !sf_malloc)
			f->endb = f->data+size;
	}

	f->flags = (f->flags & ~SF_MALLOC)|sf_malloc;

	if(obuf && obuf != f->data && osize > 0 && (oflags&SF_MALLOC))
	{	free((Void_t*)obuf);
		obuf = NIL(uchar*);
	}

	_Sfi = f->val = obuf ? osize : 0;

	SFOPEN(f,local);

	return (Void_t*)obuf;
}
