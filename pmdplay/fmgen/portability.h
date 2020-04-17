#ifndef PORTABILITY_H
#define PORTABILITY_H

#include "headers.h"

#ifndef _WIN32

//	呼出規約の無効化

#define	__stdcall
#define	WINAPI


#define	_MAX_PATH	((PATH_MAX) > ((FILENAME_MAX)*4) ? (PATH_MAX) : ((FILENAME_MAX)*4))
#define	_MAX_DIR	FILENAME_MAX
#define	_MAX_FNAME	FILENAME_MAX
#define	_MAX_EXT	FILENAME_MAX


#endif


#endif	// PORTABILITY_H
