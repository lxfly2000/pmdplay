//	$Id: file.cpp,v 1.6 1999/12/28 11:14:05 cisc Exp $

#include "headers.h"
#include "file.h"
#include "portability.h"


#ifdef _WIN32

// ---------------------------------------------------------------------------
//	構築
// ---------------------------------------------------------------------------

FilePath::FilePath() : EmptyChar('\0')
{
}

// ---------------------------------------------------------------------------
//	ファイル名で示されたファイルのサイズを取得する
// ---------------------------------------------------------------------------

int64 FilePath::GetFileSize(const TCHAR* filename)
{
	HANDLE	handle;
	WIN32_FIND_DATA	FindFileData;

	if((handle = FindFirstFile(filename, &FindFileData)) == INVALID_HANDLE_VALUE) {
		return -1;		// 取得不可
	} else {
		FindClose(handle);
		return (((_int64)FindFileData.nFileSizeHigh) << 32) + FindFileData.nFileSizeLow;
 	}
}

// ---------------------------------------------------------------------------
//	ファイル名をクリア
// ---------------------------------------------------------------------------

TCHAR* FilePath::Clear(TCHAR* dest, size_t size)
{
	return (TCHAR*)memset(dest, 0, size * sizeof(TCHAR));
}

// ---------------------------------------------------------------------------
//	ファイル名が空文字列か確認
// ---------------------------------------------------------------------------

bool FilePath::IsEmpty(const TCHAR* src)
{
	return (*src == EmptyChar);
}

// ---------------------------------------------------------------------------
//	ファイル名がクリアされているか確認
// ---------------------------------------------------------------------------

const TCHAR* FilePath::GetEmptyStr(void)
{
	return &EmptyChar;
}

// ---------------------------------------------------------------------------
//	文字列の長さを取得
// ---------------------------------------------------------------------------

size_t FilePath::Strlen(const TCHAR *str)
{
#ifdef _MBCS
	return strlen(str);
#endif
	
#ifdef _UNICODE
	return wcslen(str);
#endif
}

// ---------------------------------------------------------------------------
//	文字列を比較
// ---------------------------------------------------------------------------

int FilePath::Strcmp(const TCHAR *str1, const TCHAR *str2)
{
	return _tcscmp(str1, str2);
}

// ---------------------------------------------------------------------------
//	サイズを指定して文字列を比較
// ---------------------------------------------------------------------------

int FilePath::Strncmp(const TCHAR *str1, const TCHAR *str2, size_t size)
{
#ifdef _MBCS
	return strncmp(str1, str2, size);
#endif
	
#ifdef _UNICODE
	return wcsncmp(str1, str2, size);
#endif
}

// ---------------------------------------------------------------------------
//	大文字、小文字を同一視して文字列を比較
// ---------------------------------------------------------------------------

int FilePath::Stricmp(const TCHAR *str1, const TCHAR *str2)
{
	return _tcsicmp(str1, str2);
}

// ---------------------------------------------------------------------------
//	大文字、小文字を同一視、サイズを指定して文字列を比較
// ---------------------------------------------------------------------------

int FilePath::Strnicmp(const TCHAR *str1, const TCHAR *str2, size_t size)
{
#ifdef _MBCS
	return strnicmp(str1, str2, size);
#endif
	
#ifdef _UNICODE
	return _wcsnicmp(str1, str2, size);
#endif
}

// ---------------------------------------------------------------------------
//	文字列をコピー
// ---------------------------------------------------------------------------

TCHAR* FilePath::Strcpy(TCHAR* dest, const TCHAR* src)
{
	return _tcscpy(dest, src);
}

// ---------------------------------------------------------------------------
//	長さを指定して文字列をコピー
// ---------------------------------------------------------------------------

TCHAR* FilePath::Strncpy(TCHAR* dest, const TCHAR* src, size_t size)
{
#ifdef _MBCS
	return strncpy(dest, src, size);
#endif
	
#ifdef _UNICODE
	return wcsncpy(dest, src, size);
#endif
}

// ---------------------------------------------------------------------------
//	文字列を追加
// ---------------------------------------------------------------------------

TCHAR* FilePath::Strcat(TCHAR* dest, const TCHAR* src)
{
	return _tcscat(dest, src);
}

// ---------------------------------------------------------------------------
//	文字数を指定して文字列を追加
// ---------------------------------------------------------------------------

TCHAR* FilePath::Strncat(TCHAR* dest, const TCHAR* src, size_t count)
{
#ifdef _MBCS
	return strncat(dest, src, count);
#endif
	
#ifdef _UNICODE
	return wcsncat(dest, src, count);
#endif
}

// ---------------------------------------------------------------------------
//	指定された文字の最初の出現箇所を検索
// ---------------------------------------------------------------------------

TCHAR* FilePath::Strchr(const TCHAR *str, TCHAR c)
{
#ifdef _MBCS
	return (TCHAR*)_mbschr((const unsigned char*)str, c);
#endif
	
#ifdef _UNICODE
	return (TCHAR*)wcschr((TCHAR*)str, c);
#endif
}

// ---------------------------------------------------------------------------
//	指定された文字の最後の出現箇所を検索
// ---------------------------------------------------------------------------

TCHAR* FilePath::Strrchr(const TCHAR *str, TCHAR c)
{
#ifdef _MBCS
	return (TCHAR*)_mbsrchr((const unsigned char*)str, c);
#endif
	
#ifdef _UNICODE
	return (TCHAR*)wcsrchr((TCHAR*)str, c);
#endif
}

// ---------------------------------------------------------------------------
//	文字列の末尾に「\」を付与
// ---------------------------------------------------------------------------
TCHAR* FilePath::AddDelimiter(TCHAR* str)
{
	if(Strrchr(str, _T('\\')) != &str[Strlen(str)-1]) {
		Strcat(str, _T("\\"));
	}
	return str;
}

// ---------------------------------------------------------------------------
//	ファイル名を分割
// ---------------------------------------------------------------------------

void FilePath::Splitpath(const TCHAR *path, TCHAR *drive, TCHAR *dir, TCHAR *fname, TCHAR *ext)
{
	_tsplitpath(path, drive, dir, fname, ext);
}

// ---------------------------------------------------------------------------
//	ファイル名を合成
// ---------------------------------------------------------------------------

void FilePath::Makepath(TCHAR *path, const TCHAR *drive, const TCHAR *dir, const TCHAR *fname, const TCHAR *ext)
{
	_tmakepath(path, drive, dir, fname, ext);
}

// ---------------------------------------------------------------------------
//	ファイル名を合成(ディレクトリ＋ファイル名)
// ---------------------------------------------------------------------------

void FilePath::Makepath_dir_filename(TCHAR* path, const TCHAR* dir, const TCHAR* filename)
{
	Strcpy(path, dir);
	if(Strrchr(dir, _T('\\')) != &dir[Strlen(dir)-1]) {
		Strcat(path, _T("\\"));
	}
	Strcat(path, filename);
}

// ---------------------------------------------------------------------------
//	ファイルパスから指定された要素を抽出
// ---------------------------------------------------------------------------

void FilePath::Extractpath(TCHAR *dest, const TCHAR *src, uint flg)
{
	TCHAR	drive[_MAX_PATH];
	TCHAR	dir[_MAX_PATH];
	TCHAR	filename[_MAX_PATH];
	TCHAR	ext[_MAX_PATH];
	TCHAR*	pdrive;
	TCHAR*	pdir;
	TCHAR*	pfilename;
	TCHAR*	pext;
	
	*dest = EmptyChar;
	if(flg & extractpath_drive) {
		pdrive = drive;	
	} else {
		pdrive = NULL;
	}
	
	if(flg & extractpath_dir) {
		pdir = dir;	
	} else {
		pdir = NULL;
	}
	
	if(flg & extractpath_filename) {
		pfilename = filename;
	} else {
		pfilename = NULL;
	}
	
	if(flg & extractpath_ext) {
		pext = ext;
	} else {
		pext = NULL;
	}
	
	Splitpath(src, pdrive, pdir, pfilename, pext);
	Makepath(dest, pdrive, pdir, pfilename, pext);
}

// ---------------------------------------------------------------------------
//	ファイルパスから指定された要素を比較
// ---------------------------------------------------------------------------

int FilePath::Comparepath(TCHAR *filename1, const TCHAR *filename2, uint flg)
{
	TCHAR	extfilename1[_MAX_PATH];
	TCHAR	extfilename2[_MAX_PATH];
	
	Extractpath(extfilename1, filename1, flg);
	Extractpath(extfilename2, filename2, flg);
	return Stricmp(extfilename1, extfilename2);
}

// ---------------------------------------------------------------------------
//	拡張子を変更
// ---------------------------------------------------------------------------

TCHAR* FilePath::ExchangeExt(TCHAR *dest, TCHAR *src, const TCHAR *ext)
{
	TCHAR	drive2[_MAX_PATH];
	TCHAR	dir2[_MAX_PATH];
	TCHAR	filename2[_MAX_PATH];
	TCHAR	ext2[_MAX_PATH];
	
	Splitpath(src, drive2, dir2, filename2, ext2);
	Makepath(dest, drive2, dir2, filename2, ext);
	
	return dest;
}

// ---------------------------------------------------------------------------
//	char配列→TCHAR配列に変換
// ---------------------------------------------------------------------------

TCHAR* FilePath::CharToTCHAR(TCHAR *dest, const char *src)
{
#ifdef _MBCS
	return strcpy(dest, src);
#endif
	
#ifdef _UNICODE
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, src, -1, dest, strlen(src));
	return dest;
#endif
}

// ---------------------------------------------------------------------------
//	char配列→TCHAR配列に変換(文字数指定)
// ---------------------------------------------------------------------------

TCHAR* FilePath::CharToTCHARn(TCHAR *dest, const char *src, size_t count)
{
#ifdef _MBCS
	Clear(dest, count);
	return Strncpy(dest, src, count);
#endif
	
#ifdef _UNICODE
	Clear(dest, count);
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, src, -1, dest, count);
	return dest;
#endif
	
}


// ---------------------------------------------------------------------------
//	構築/消滅
// ---------------------------------------------------------------------------

FileIO::FileIO()
{
	flags = 0;
	lorigin = 0;
	error = success;
	path = new TCHAR[_MAX_PATH];
	
	hfile = 0;
}

FileIO::FileIO(const TCHAR* filename, uint flg)
{
	flags = 0;
	path = new TCHAR[_MAX_PATH];
	Open(filename, flg);
}

FileIO::~FileIO()
{
	Close();
	delete[] path;
}

// ---------------------------------------------------------------------------
//	ファイルを開く
// ---------------------------------------------------------------------------

bool FileIO::Open(const TCHAR* filename, uint flg)
{
	FilePath	filepath;
	
	Close();
	
	filepath.Strncpy(path, filename, _MAX_PATH);
	
	DWORD access = (flg & readonly ? 0 : GENERIC_WRITE) | GENERIC_READ;
	DWORD share = (flg & readonly) ? FILE_SHARE_READ : 0;
	DWORD creation = flg & create ? CREATE_ALWAYS : OPEN_EXISTING;
	
	hfile = CreateFile(filename, access, share, 0, creation, 0, 0);
	
	flags = (flg & readonly) | (hfile == INVALID_HANDLE_VALUE ? 0 : open);
	if (!(flags & open))
	{
		switch (GetLastError())
		{
		case ERROR_FILE_NOT_FOUND:		error = file_not_found; break;
		case ERROR_SHARING_VIOLATION:	error = sharing_violation; break;
		default: error = unknown; break;
		}
	}
	SetLogicalOrigin(0);
	
	return !!(flags & open);
}

// ---------------------------------------------------------------------------
//	ファイルがない場合は作成
// ---------------------------------------------------------------------------

bool FileIO::CreateNew(const TCHAR* filename)
{
	FilePath	filepath;
	Close();
	
	filepath.Strncpy(path, filename, _MAX_PATH);
	
	DWORD access = GENERIC_WRITE | GENERIC_READ;
	DWORD share = 0;
	DWORD creation = CREATE_NEW;
	
	hfile = CreateFile(filename, access, share, 0, creation, 0, 0);
	
	flags = (hfile == INVALID_HANDLE_VALUE ? 0 : open);
	SetLogicalOrigin(0);
	
	return !!(flags & open);
}

// ---------------------------------------------------------------------------
//	ファイルを作り直す
// ---------------------------------------------------------------------------

bool FileIO::Reopen(uint flg)
{
	if (!(flags & open)) return false;
	if ((flags & readonly) && (flg & create)) return false;
	
	if (flags & readonly) flg |= readonly;
	
	Close();
	
	DWORD access = (flg & readonly ? 0 : GENERIC_WRITE) | GENERIC_READ;
	DWORD share = flg & readonly ? FILE_SHARE_READ : 0;
	DWORD creation = flg & create ? CREATE_ALWAYS : OPEN_EXISTING;
	
	hfile = CreateFile(path, access, share, 0, creation, 0, 0);
	
	flags = (flg & readonly) | (hfile == INVALID_HANDLE_VALUE ? 0 : open);
	SetLogicalOrigin(0);
	
	return !!(flags & open);
}

// ---------------------------------------------------------------------------
//	ファイルを閉じる
// ---------------------------------------------------------------------------

void FileIO::Close()
{
	if (GetFlags() & open)
	{
		CloseHandle(hfile);
		hfile = 0;
		flags = 0;
	}
}

// ---------------------------------------------------------------------------
//	ファイルからの読み出し
// ---------------------------------------------------------------------------

int32 FileIO::Read(void* dest, int32 size)
{
	if (!(GetFlags() & open))
		return -1;
	
	DWORD readsize;
	if (!ReadFile(hfile, dest, size, &readsize, 0))
		return -1;
	return readsize;
}

// ---------------------------------------------------------------------------
//	ファイルへの書き出し
// ---------------------------------------------------------------------------

int32 FileIO::Write(const void* dest, int32 size)
{
	if (!(GetFlags() & open) || (GetFlags() & readonly))
		return -1;
	
	DWORD writtensize;
	if (!WriteFile(hfile, dest, size, &writtensize, 0))
		return -1;
	return writtensize;
}

// ---------------------------------------------------------------------------
//	ファイルをシーク
// ---------------------------------------------------------------------------

bool FileIO::Seek(int32 pos, SeekMethod method)
{
	if (!(GetFlags() & open))
		return false;
	
	DWORD wmethod;
	switch (method)
	{
	case begin:
		wmethod = FILE_BEGIN; pos += lorigin;
		break;
	case current:
		wmethod = FILE_CURRENT;
		break;
	case end:
		wmethod = FILE_END;
		break;
	default:
		return false;
	}
	
	return 0xffffffff != SetFilePointer(hfile, pos, 0, wmethod);
}

// ---------------------------------------------------------------------------
//	ファイルの位置を得る
// ---------------------------------------------------------------------------

int32 FileIO::Tellp()
{
	if (!(GetFlags() & open))
		return 0;
	
	return SetFilePointer(hfile, 0, 0, FILE_CURRENT) - lorigin;
}

// ---------------------------------------------------------------------------
//	現在の位置をファイルの終端とする
// ---------------------------------------------------------------------------

bool FileIO::SetEndOfFile()
{
	if (!(GetFlags() & open))
		return false;
	return ::SetEndOfFile(hfile) != 0;
}


#else

// ---------------------------------------------------------------------------
//	構築
// ---------------------------------------------------------------------------

FilePath::FilePath() : EmptyChar('\0')
{
}

// ---------------------------------------------------------------------------
//	ファイル名で示されたファイルのサイズを取得する
// ---------------------------------------------------------------------------

int64 FilePath::GetFileSize(const TCHAR* filename)
{
	int fd;
	struct stat buf;
	
	if((fd = open(filename, O_RDONLY)) == -1) {
		return -1;		// 取得不可
		
	} else if(fstat(fd, &buf) != 0) {
		return -1;		// 取得不可
	}
	
	return buf.st_size;
}

// ---------------------------------------------------------------------------
//	ファイル名をクリア
// ---------------------------------------------------------------------------

TCHAR* FilePath::Clear(TCHAR* dest, size_t size)
{
	return (TCHAR*)memset(dest, 0, size * sizeof(TCHAR));
}

// ---------------------------------------------------------------------------
//	ファイル名が空文字列か確認
// ---------------------------------------------------------------------------

bool FilePath::IsEmpty(const TCHAR* src)
{
	return (*src == EmptyChar);
}

// ---------------------------------------------------------------------------
//	ファイル名がクリアされているか確認
// ---------------------------------------------------------------------------

const TCHAR* FilePath::GetEmptyStr(void)
{
	return &EmptyChar;
}

// ---------------------------------------------------------------------------
//	文字列の長さを取得
// ---------------------------------------------------------------------------

size_t FilePath::Strlen(const TCHAR *str)
{
	return strlen(str);
}

// ---------------------------------------------------------------------------
//	文字列を比較
// ---------------------------------------------------------------------------

int FilePath::Strcmp(const TCHAR *str1, const TCHAR *str2)
{
	return strcmp(str1, str2);
}

// ---------------------------------------------------------------------------
//	サイズを指定して文字列を比較
// ---------------------------------------------------------------------------

int FilePath::Strncmp(const TCHAR *str1, const TCHAR *str2, size_t size)
{
	return strncmp(str1, str2, size);
}

// ---------------------------------------------------------------------------
//	大文字、小文字を同一視して文字列を比較
// ---------------------------------------------------------------------------

int FilePath::Stricmp(const TCHAR *str1, const TCHAR *str2)
{
	return strcasecmp(str1, str2);
}

// ---------------------------------------------------------------------------
//	大文字、小文字を同一視、サイズを指定して文字列を比較
// ---------------------------------------------------------------------------

int FilePath::Strnicmp(const TCHAR *str1, const TCHAR *str2, size_t size)
{
	return strncasecmp(str1, str2, size);
}

// ---------------------------------------------------------------------------
//	文字列をコピー
// ---------------------------------------------------------------------------

TCHAR* FilePath::Strcpy(TCHAR* dest, const TCHAR* src)
{
	return strcpy(dest, src);
}

// ---------------------------------------------------------------------------
//	長さを指定して文字列をコピー
// ---------------------------------------------------------------------------

TCHAR* FilePath::Strncpy(TCHAR* dest, const TCHAR* src, size_t size)
{
	return strncpy(dest, src, size);
}

// ---------------------------------------------------------------------------
//	文字列を追加
// ---------------------------------------------------------------------------

TCHAR* FilePath::Strcat(TCHAR* dest, const TCHAR* src)
{
	return strcat(dest, src);
}

// ---------------------------------------------------------------------------
//	文字数を指定して文字列を追加
// ---------------------------------------------------------------------------

TCHAR* FilePath::Strncat(TCHAR* dest, const TCHAR* src, size_t count)
{
	return strncat(dest, src, count);
}

// ---------------------------------------------------------------------------
//	指定された文字の最初の出現箇所を検索
// ---------------------------------------------------------------------------

TCHAR* FilePath::Strchr(const TCHAR *str, TCHAR c)
{
	return (TCHAR*)strchr((char*)str, c);
}

// ---------------------------------------------------------------------------
//	指定された文字の最後の出現箇所を検索
// ---------------------------------------------------------------------------

TCHAR* FilePath::Strrchr(const TCHAR *str, TCHAR c)
{
	return (TCHAR*)strrchr((char*)str, c);
}

// ---------------------------------------------------------------------------
//	文字列の末尾に「/」を付与
// ---------------------------------------------------------------------------
TCHAR* FilePath::AddDelimiter(TCHAR* str)
{
	if(Strrchr(str, _T('/')) != &str[Strlen(str)-1]) {
		Strcat(str, _T("/"));
	}
	return str;
}

// ---------------------------------------------------------------------------
//	ファイル名を分割
// ---------------------------------------------------------------------------

void FilePath::Splitpath(const TCHAR *path, TCHAR *drive, TCHAR *dir, TCHAR *fname, TCHAR *ext)
{
	if(path == NULL) {
		if(drive != NULL) {
			*drive = EmptyChar;
		}
		
		if(dir != NULL) {
			*dir = EmptyChar;
		}
		
		if(fname != NULL) {
			*fname = EmptyChar;
		}
		
		if(ext != NULL) {
			*ext = EmptyChar;
		}
		return;
	}
	if(*path == EmptyChar) {
		if(drive != NULL) {
			*drive = EmptyChar;
		}
		
		if(dir != NULL) {
			*dir = EmptyChar;
		}
		
		if(fname != NULL) {
			*fname = EmptyChar;
		}
		
		if(ext != NULL) {
			*ext = EmptyChar;
		}
		return;
	}
	
	TCHAR*	p1 = Strrchr(path, '/');
	TCHAR*	p2 = Strrchr(path, '.');
	
	if(p1 != NULL && p2 != NULL && p1 > p2) {
		p2 = const_cast<TCHAR*>(path) + Strlen(path);
	}
	
	if(p1 == NULL) {
		p1 = const_cast<TCHAR*>(path) - 1;
	}
	
	if(p2 == NULL) {
		p2 = const_cast<TCHAR*>(path) + Strlen(path);
	}
	
	if(drive != NULL) {
		*drive = EmptyChar;
	}
	
	if(dir != NULL) {
		Strncpy(dir, path, p1 - path + 1);
		dir[p1 - path + 1] = EmptyChar;
	}
	
	if(fname != NULL) {
		Strncpy(fname, p1 + 1, p2 - p1 - 1);
		fname[p2 - p1 - 1] = EmptyChar;
	}
	
	if(ext != NULL) {
		Strcpy(ext, p2);
	}
}

// ---------------------------------------------------------------------------
//	ファイル名を合成
// ---------------------------------------------------------------------------

void FilePath::Makepath(TCHAR *path, const TCHAR *drive, const TCHAR *dir, const TCHAR *fname, const TCHAR *ext)
{
	// driveは無視
	
	if(dir == NULL) {
		*path = EmptyChar;
	} else {
		strcpy(path, dir);
		if(strlen(path) >= 1) {
			if(path[strlen(path)-1] != '/') {
				strcat(path, "/");
			}
		}
	}
	if(fname != NULL) {
		strcat(path, fname);
	}
	
	if(ext != NULL) {
		strcat(path, ext);
	}
}

// ---------------------------------------------------------------------------
//	ファイル名を合成(ディレクトリ＋ファイル名)
// ---------------------------------------------------------------------------

void FilePath::Makepath_dir_filename(TCHAR* path, const TCHAR* dir, const TCHAR* filename)
{
	Strcpy(path, dir);
	if(Strrchr(dir, _T('/')) != &dir[Strlen(dir)-1]) {
		Strcat(path, _T("/"));
	}
	Strcat(path, filename);
}

// ---------------------------------------------------------------------------
//	ファイルパスから指定された要素を抽出
// ---------------------------------------------------------------------------

void FilePath::Extractpath(TCHAR *dest, const TCHAR *src, uint flg)
{
	TCHAR	drive[_MAX_PATH];
	TCHAR	dir[_MAX_PATH];
	TCHAR	filename[_MAX_PATH];
	TCHAR	ext[_MAX_PATH];
	TCHAR*	pdrive;
	TCHAR*	pdir;
	TCHAR*	pfilename;
	TCHAR*	pext;
	
	*dest = EmptyChar;
	if(flg & extractpath_drive) {
		pdrive = drive;
	} else {
		pdrive = NULL;
	}
	
	if(flg & extractpath_dir) {
		pdir = dir;
	} else {
		pdir = NULL;
	}
	
	if(flg & extractpath_filename) {
		pfilename = filename;
	} else {
		pfilename = NULL;
	}
	
	if(flg & extractpath_ext) {
		pext = ext;
	} else {
		pext = NULL;
	}
	
	Splitpath(src, pdrive, pdir, pfilename, pext);
	Makepath(dest, pdrive, pdir, pfilename, pext);
}

// ---------------------------------------------------------------------------
//	ファイルパスから指定された要素を比較
// ---------------------------------------------------------------------------

int FilePath::Comparepath(TCHAR *filename1, const TCHAR *filename2, uint flg)
{
	TCHAR	extfilename1[_MAX_PATH];
	TCHAR	extfilename2[_MAX_PATH];
	
	Extractpath(extfilename1, filename1, flg);
	Extractpath(extfilename2, filename2, flg);
	return Stricmp(extfilename1, extfilename2);
}

// ---------------------------------------------------------------------------
//	拡張子を変更
// ---------------------------------------------------------------------------

TCHAR* FilePath::ExchangeExt(TCHAR *dest, TCHAR *src, const TCHAR *ext)
{
	TCHAR	drive2[_MAX_PATH];
	TCHAR	dir2[_MAX_PATH];
	TCHAR	filename2[_MAX_PATH];
	TCHAR	ext2[_MAX_PATH];
	
	Splitpath(src, drive2, dir2, filename2, ext2);
	Makepath(dest, drive2, dir2, filename2, ext);
	
	return dest;
}

// ---------------------------------------------------------------------------
//	char配列→TCHAR配列に変換
// ---------------------------------------------------------------------------

TCHAR* FilePath::CharToTCHAR(TCHAR *dest, const char *src)
{
	//@ 要:MBCS→UTF-8変換
	return strcpy(dest, src);
}

// ---------------------------------------------------------------------------
//	char配列→TCHAR配列に変換(文字数指定)
// ---------------------------------------------------------------------------

TCHAR* FilePath::CharToTCHARn(TCHAR *dest, const char *src, size_t count)
{
	Clear(dest, count);
	
	//@ 要:MBCS→UTF-8変換
	return Strncpy(dest, src, count);
}


// ---------------------------------------------------------------------------
//	構築/消滅
// ---------------------------------------------------------------------------

FileIO::FileIO()
{
	flags = 0;
	lorigin = 0;
	error = success;
	path = new TCHAR[_MAX_PATH];
	
	fp = NULL;
}

FileIO::FileIO(const TCHAR* filename, uint flg)
{
	flags = 0;
	path = new TCHAR[_MAX_PATH];
	Open(filename, flg);
}

FileIO::~FileIO()
{
	Close();
	delete[] path;
}

// ---------------------------------------------------------------------------
//	ファイルを開く
// ---------------------------------------------------------------------------

bool FileIO::Open(const TCHAR* filename, uint flg)
{
	FilePath	filepath;
	
	Close();
	
	filepath.Strncpy(path, filename, _MAX_PATH);
	
	char	mode[4];
	
	if(flg & readonly)
	{
		strcpy(mode, "rb");
	}
	else if(flg & create)
	{
		strcpy(mode, "wb");
	}
	
	fp = fopen(filename, mode);
	
	flags = (flg & readonly) | (fp == NULL ? 0 : open);
	if (!(flags & open))
	{
		error = unknown;		// とりあえずのエラー設定
	}
	SetLogicalOrigin(0);
	
	return !!(flags & open);
}

// ---------------------------------------------------------------------------
//	ファイルがない場合は作成
// ---------------------------------------------------------------------------

bool FileIO::CreateNew(const TCHAR* filename)
{
	FilePath	filepath;
	Close();
	
	filepath.Strncpy(path, filename, _MAX_PATH);
	
	if((fp = fopen(filename,"rb")) != NULL)
	{
		fclose( fp);
		flags = 0;
	}
	else
	{
		fp = fopen(filename, "w+b");
		
		flags = (fp == NULL ? 0 : open);
	}
	SetLogicalOrigin(0);
	
	return !!(flags & open);
}

// ---------------------------------------------------------------------------
//	ファイルを作り直す
// ---------------------------------------------------------------------------

bool FileIO::Reopen(uint flg)
{
	if (!(flags & open)) return false;
	if ((flags & readonly) && (flg & create)) return false;
	
	if (flags & readonly) flg |= readonly;
	
	Close();
	
	char	mode[4];
	
	if(flg & readonly)
	{
		strcpy(mode, "rb");
	}
	else if(flg & create)
	{
		strcpy(mode, "wb");
	}
	
	fp = fopen(path, mode);
	
	flags = (flg & readonly) | (fp == NULL ? 0 : open);
	SetLogicalOrigin(0);
	
	return !!(flags & open);
}

// ---------------------------------------------------------------------------
//	ファイルを閉じる
// ---------------------------------------------------------------------------

void FileIO::Close()
{
	if (GetFlags() & open)
	{
		fclose(fp);
		fp = NULL;
		flags = 0;
	}
}

// ---------------------------------------------------------------------------
//	ファイルからの読み出し
// ---------------------------------------------------------------------------

int32 FileIO::Read(void* dest, int32 size)
{
	if (!(GetFlags() & open))
		return -1;
	
	DWORD readsize;
	readsize = fread(dest, 1, size, fp);
	if (!readsize && size > 0)
		return -1;
	return readsize;
}

// ---------------------------------------------------------------------------
//	ファイルへの書き出し
// ---------------------------------------------------------------------------

int32 FileIO::Write(const void* dest, int32 size)
{
	if (!(GetFlags() & open) || (GetFlags() & readonly))
		return -1;
	
	DWORD writtensize;
	writtensize = fwrite(dest, 1, size, fp);
	if (!writtensize && size > 0)
		return -1;
	return writtensize;
}

// ---------------------------------------------------------------------------
//	ファイルをシーク
// ---------------------------------------------------------------------------

bool FileIO::Seek(int32 pos, SeekMethod method)
{
	if (!(GetFlags() & open))
		return false;
	
	int wmethod;
	switch (method)
	{
	case begin:
		wmethod = SEEK_SET; pos += lorigin;
		break;
	case current:
		wmethod = SEEK_CUR;
		break;
	case end:
		wmethod = SEEK_END;
		break;
	default:
		return false;
	}
	
	return 0 == fseek(fp, pos, wmethod);
}

// ---------------------------------------------------------------------------
//	ファイルの位置を得る
// ---------------------------------------------------------------------------

int32 FileIO::Tellp()
{
	if (!(GetFlags() & open))
		return 0;
	
	return ftell(fp) - lorigin;
}

// ---------------------------------------------------------------------------
//	現在の位置をファイルの終端とする
// ---------------------------------------------------------------------------

bool FileIO::SetEndOfFile()
{
	if (!(GetFlags() & open))
		return false;
	ftruncate(fileno(fp), Tellp());
}


#endif
