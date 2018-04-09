#ifndef PMDWIN_COMPAT_H
#define PMDWIN_COMPAT_H

#include <stdio.h>

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;
#define _int64 __int64//typedef long long _int64;

#ifndef _WINDOWS
# define strnicmp strncasecmp
#endif

#ifndef HAVE__ISMBBLEAD
# define _ismbblead(c) \
   (((c) >= 0x80) && ((c) <= 0x9F) || \
    ((c) >= 0xE0) && ((c) <= 0xFC))
#endif

#ifndef HAVE__MBSRCHR
# define _mbsrchr(s,c) \
   strrchr((char *)(s), (c))
#endif

#ifndef MAX_PATH
# define MAX_PATH FILENAME_MAX
#endif

#ifndef _MAX_PATH
# define _MAX_PATH MAX_PATH
#endif

#ifndef _MAX_DRIVE
# define _MAX_DRIVE FILENAME_MAX
#endif

#ifndef _MAX_DIR
# define _MAX_DIR FILENAME_MAX
#endif

#ifndef _MAX_FNAME
# define _MAX_FNAME FILENAME_MAX
#endif

#ifndef _MAX_EXT
# define _MAX_EXT FILENAME_MAX
#endif

#ifdef WIN32
# define PATH_DELIMIT_CHR '\\'
# define PATH_DELIMIT_STR "\\"
#else
# define PATH_DELIMIT_CHR '/'
# define PATH_DELIMIT_STR "/"
#endif

#ifndef WIN32
# define __declspec(x)
# define WINAPI
#endif

#ifdef __GNUC__
# define __PACKED__ __attribute__((packed))
#else
#define __PACKED__
#endif

#endif /* PMDWIN_COMPAT_H */
