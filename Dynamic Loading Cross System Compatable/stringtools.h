/** ------------------------------------------------------------
 ** Tools to perform some handy string operations
 ** ------------------------------------------------------------
 ** $Id: stringtools.h 81 2010-07-12 19:38:05Z andrew $
 **/

#ifndef         STRINGTOOLS_HEADER__
#define         STRINGTOOLS_HEADER__


/**
 * This is how we use conditional compilation to wrap regular C code
 * for use in a C++ program while making it also available for use
 * in C itself.
 */
# if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
# endif

char *strconcat(const char *leadptr, ...);

int split(
			char **fill_array,
			const char *delims,
			const char *source,
			int max
		);

# if defined(__cplusplus) || defined(c_plusplus)
}
# endif

#endif  /* STRINGTOOLS_HEADER__ */

