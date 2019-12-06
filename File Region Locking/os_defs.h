#ifndef __FORMATTER_DEFS_HEADER__
#define __FORMATTER_DEFS_HEADER__

/* System include files. */
#ifndef MAKEDEPEND
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#endif


	/*
	 * You will have to add more code here -- use the -E or /E flag of
	 * your compiler to see what it defines by default, in order to
	 * be able to figure out what operating system/compiler combo you are
	 * on.  You should be able to define one of the following macros to
	 * identify each of the indicated systems:
	 *	OS_BSD
	 *		defined only if the environment is some form of BSD
	 *		(this one is done for you)
	 *	OS_WINDOWS
	 *		defined only if the environment is some Windows variant
	 *		(Windows 10, etc.)
	 *	OS_LINUX
	 *		defined only if the environment is some version of Linux
	 *	OS_DARWIN
	 *		defined only if the environment is some version of Darwin/MacOSX
	 */

/*
 * Compilation Environment Identification
 */
#if defined( __NetBSD__ ) || defined( __OpenBSD__) || defined( __FreeBSD__ )

    /** example code to figure out if we are on a BSD unix (but not MacOSX) */

#   define OS_BSD


    /** figure out the endianness */
#   if defined( __x86_64__ )
#     define OS_LITTLE_ENDIAN
#   elif defined( _BIG_ENDIAN )
#     define OS_BIG_ENDIAN
#   elif defined( _LITTLE_ENDIAN )
#     define OS_LITTLE_ENDIAN
#   else
#     error Unknown endian on BSD
#   endif


#elif defined(__linux__)

#define OS_LINUX

#elif defined(_WIN32)

#define OS_WINDOWS

#elif defined(__APPLE__)

#define OS_DARWIN

#else
#   warning Unknown operating system -- need more defines in os_defs.h!
#endif


/*
 * if we are using a dynamic loaded library, windows wants to know
 * in what way we are using it
 */
#if defined( OS_WINDOWS )
# if defined( OS_USE_DYNAMIC )
	/**
	 **	if OS_USE_DYNAMIC is declared before importing this header, then
	 **	we want OS_EXPORT declarations in other header files to turn into
	 **	_import_ declarations
	 **/
#  define OS_EXPORT __declspec(dllimport)
#  define OS_C_DECL __cdecl

# elif defined( OS_STATIC )
	/**
	 **	if we are statically linking, these should be nothing, as on other
	 ** OSs
	 **/
#  define OS_EXPORT
#  define OS_C_DECL


# else
	/**
	 **	default to dynamic linking, and declaring the objects as such in
	 ** headers and in code
	 **/
#  define OS_EXPORT __declspec(dllexport)
#  define OS_C_DECL __cdecl
# endif
#else

	/**
	 **	If not on Windows, none of this applies, so simply deflate the
	 **	macro to nothing
	 **/
#  define OS_EXPORT
#  define OS_C_DECL
#endif



/**
 * Win32 hacks...
 */
#ifdef  OS_WINDOWS

	/*
	 * Define a character and string for use when trying to put
	 * together directory paths for our various operating systems.
	 * Everyone but Microsoft uses /, but MS uses \
	 */
# define	OS_PATH_DELIM			'\\'
# define	OS_PATH_DELIM_STRING		"\\"

/* For some reason, various versions of Windows don't
 * define these common values.  When these are defined, M_PI is
 * defined, so we use this fact below to figure out when to try and
 * add these definitions.
 */

# include	<math.h>
# ifndef	M_PI
	/**
	 * these values are taken from the GCC header file /usr/include/math.h
	 * and are reproduced here as these values are not available under Windows
	 */
#  define M_E		2.7182818284590452354	/* e */
#  define M_LOG2E	1.4426950408889634074	/* log_2 e */
#  define M_LOG10E	0.43429448190325182765	/* log_10 e */
#  define M_LN2		0.69314718055994530942	/* log_e 2 */
#  define M_LN10	2.30258509299404568402	/* log_e 10 */
#  define M_PI		3.14159265358979323846	/* pi */
#  define M_PI_2	1.57079632679489661923	/* pi/2 */
#  define M_PI_4	0.78539816339744830962	/* pi/4 */
#  define M_1_PI	0.31830988618379067154	/* 1/pi */
#  define M_2_PI	0.63661977236758134308	/* 2/pi */
#  define M_2_SQRTPI	1.12837916709551257390	/* 2/sqrt(pi) */
#  define M_SQRT2	1.41421356237309504880	/* sqrt(2) */
#  define M_SQRT1_2	0.70710678118654752440	/* 1/sqrt(2) */
# endif

#else

	/** define the path delimiter for all non-Windows machines */
# define	OS_PATH_DELIM			'/'
# define	OS_PATH_DELIM_STRING		"/"

    /**
     * Windows uses O_BINARY to define opening a file in binary
     * mode via the open(2) interface.  Other operating systems
     * don't define "binary mode", so we define O_BINARY to a
     * safe value to OR with in case we are using open(2)
     * or creat(2) somewhere.
     */
# ifndef	O_BINARY
#  define	O_BINARY	0
# endif

#endif

#endif /* __FORMATTER_DEFS_HEADER__ */
