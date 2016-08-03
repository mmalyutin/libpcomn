/*-*- mode: c++; tab-width: 3; indent-tabs-mode: nil; c-file-style: "ellemtel"; c-file-offsets:((innamespace . 0)(inclass . ++)) -*-*/
#ifndef __PCOMN_SYS_H
#define __PCOMN_SYS_H
/*******************************************************************************
 FILE         :   pcomn_sys.h
 COPYRIGHT    :   Yakov Markovitch, 2008-2016. All rights reserved.
                  See LICENSE for information on usage/redistribution.

 DESCRIPTION  :   System (platform) functions

 PROGRAMMED BY:   Yakov Markovitch
 CREATION DATE:   18 Nov 2008
*******************************************************************************/
/** @file
 System (platform) functions
*******************************************************************************/
#include <pcomn_platform.h>
#include <pcomn_cstrptr.h>
#include <pcomn_except.h>
#include <pcomn_unistd.h>
#include <pcomn_path.h>

#include "platform/dirent.h"

namespace pcomn {
namespace sys {

enum Access {
   ACC_EXISTS,
   ACC_NOEXIST,
   ACC_DENIED,
   ACC_ERROR
} ;

/// Get the memory page size for the platform.
inline size_t pagesize() ;

/// Portable filesize function.
/// @return The file size, or -1 on error.
inline fileoff_t filesize(int fd) ;

/// @overload
inline fileoff_t filesize(const char *name) ;

inline Access fileaccess(const char *name, int mode = 0)
{
   if (::access(name, mode) == 0)
      return ACC_EXISTS ;
   switch (errno)
   {
      case ENOENT:   return ACC_NOEXIST ;
      #ifdef PCOMN_PL_UNIX
      case EROFS:
      #endif
      case EACCES:   return ACC_DENIED ;
   }
   return ACC_ERROR ;
}

/// Get CPU cores count on the system.
/// @return The total count of @em actual cores on all physical CPUs in the system @em
/// not counting hyperthreads.
/// @param[out] phys_sockets  The function places here a count of physical CPUs (sockets).
/// @param[out] ht_count      The function places here a count of cores counting also
/// hyperthreads.
_PCOMNEXP unsigned cpu_core_count(unsigned *phys_sockets = NULL, unsigned *ht_count = NULL) ;

inline unsigned hw_threads_count()
{
   unsigned result = 0 ;
   cpu_core_count(NULL, &result) ;
   return result ;
}

const unsigned ODIR_SKIP_DOT     = 0x0001 ; /**< Skip '.' while writing filenames */
const unsigned ODIR_SKIP_DOTDOT  = 0x0002 ; /**< Skip '..' while writing filenames */
const unsigned ODIR_SKIP_DOTS    = 0x0003 ; /**< Skip both '.' and '..' */
const unsigned ODIR_CLOSE_DIR    = 0x0004 ; /**< Close directory descriptor on return */

/// @cond
namespace detail {
template<typename OutputIterator>
DIR *listdir(const char *dirname, unsigned flags, OutputIterator &filenames, RaiseError raise)
{
   DIR * const d = ::opendir(PCOMN_ENSURE_ARG(dirname)) ;
   if (!d)
   {
      PCOMN_CHECK_POSIX(-!!raise, "Cannot open directory '%s'", dirname) ;
      return NULL ;
   }

   errno = 0 ;
   for (const dirent *entry ; (entry = ::readdir(d)) != NULL ; errno = 0, ++filenames)
      if (!(path::posix::path_dots(entry->d_name) & flags))
         *filenames = entry->d_name ;

   const int err = errno ;
   if (err || (flags & ODIR_CLOSE_DIR))
   {
      ::closedir(d) ;
      errno = err ;
      if (!err)
         return not_a_pointer<DIR>::value ;

      PCOMN_CHECK_POSIX(-!!raise, "Cannot read directory '%s'", dirname) ;
      return NULL ;
   }

   return d ;
}
}
/// @endcond

/// Open and read a directory.
/// @return Pointer to DIR; if @a raise is DONT_RAISE_ERROR
/// and there happens an error while opening/reading a directory, returns NULL.
///
template<typename OutputIterator>
inline DIR *opendir(const cstrptr &dirname, unsigned flags, OutputIterator filenames, RaiseError raise = DONT_RAISE_ERROR)
{
   return detail::listdir(dirname, flags, filenames, raise) ;
}

template<typename OutputIterator>
inline OutputIterator ls(const cstrptr &dirname, unsigned flags, OutputIterator filenames, RaiseError raise = DONT_RAISE_ERROR)
{
   detail::listdir(dirname, flags | ODIR_CLOSE_DIR, filenames, raise) ;
   return filenames ;
}

#ifdef PCOMN_PL_LINUX
/// Open and read a directory.
/// @return A file descriptor of directory @a dirname; if @a raise is DONT_RAISE_ERROR
/// and there happens an error while opening/reading a directory, returns -1.
///
template<typename OutputIterator>
inline int opendirfd(const cstrptr &dirname, unsigned flags, OutputIterator filenames, RaiseError raise = DONT_RAISE_ERROR)
{
   if (DIR * const d = detail::listdir(dirname, flags &~ ODIR_CLOSE_DIR, filenames, raise))
   {
      const int result_fd = (flags & ODIR_CLOSE_DIR) ? 0 : dup(dirfd(d)) ;
      ::closedir(d) ;
      return result_fd ;
   }
   else
      return -1 ;
}
#endif


/*******************************************************************************
 filestat
*******************************************************************************/
#define _PCOMN_SYS_FSTAT(statfn, path, raise)      \
   do {                                            \
      fsstat result ;                              \
      const int r = ::statfn((path), &(result)) ;  \
      if ((raise))                                 \
         PCOMN_ENSURE_POSIX(r, #statfn) ;          \
      else if (r == -1)                            \
         (result).clear() ;                        \
      return result ;                              \
   } while(false)

struct fsstat : stat {
      void clear() { memset(this, 0, sizeof *this) ; }
      explicit operator bool() const { return st_nlink || st_dev || st_ino || st_mode ; }
} ;

inline fsstat filestat(const char *path, RaiseError raise = DONT_RAISE_ERROR)
{
   _PCOMN_SYS_FSTAT(stat, path, raise) ;
}

inline fsstat filestat(int fd, RaiseError raise = DONT_RAISE_ERROR)
{
   _PCOMN_SYS_FSTAT(fstat, fd, raise) ;
}

#ifdef PCOMN_PL_POSIX
inline fsstat linkstat(const char *path, RaiseError raise = DONT_RAISE_ERROR)
{
   _PCOMN_SYS_FSTAT(lstat, path, raise) ;
}
#endif

#undef _PCOMN_SYS_FSTAT

} // end of namespace pcomn::sys
} // end of namespace pcomn

#include PCOMN_PLATFORM_HEADER(pcomn_sys.h)

#endif /* __PCOMN_SYS_H */
