//=============================================================================
//			Utility Functions
//				Programmed by C60
//=============================================================================

#ifdef _WINDOWS
#include	<windows.h>
#include	<mbstring.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include	"util.h"


//=============================================================================
//	ファイル名で示されたファイルのサイズを取得する
//=============================================================================
_int64 GetFileSize_s(char *filename)
{
#ifdef _WINDOWS
	HANDLE	handle;
	WIN32_FIND_DATAA	FindFileData;

	if((handle = FindFirstFileA(filename, &FindFileData)) == INVALID_HANDLE_VALUE) {
		return -1;		// 取得不可
	} else {
		FindClose(handle);
		return (_int64)((__int64)FindFileData.nFileSizeHigh << 32) + FindFileData.nFileSizeLow; 
 	}
#else
	FILE *fp;
	int size;

	fp = fopen (filename, "rb");
	if (!fp)
	  {
	    int i;
	    
	    for (i = 0; i < strlen (filename); i ++)
	      filename[i] = tolower (filename[i]);

	    fp = fopen (filename, "rb");
	    if (!fp)
		return -1;
	  }

	fseek (fp, 0, SEEK_END);
	size = (int)ftell (fp);
	fclose (fp);

	return size;
#endif
}


//=============================================================================
//	TAB を SPACE に変換(tabcolumn カラム毎 / ESCシーケンス不可)
//=============================================================================
char *tab2spc(char *dest, const char *src, int tabcolumn)
{
	int		column = 0;
	char	*dest2;

	dest2 = dest;
	while(*src != '\0') {
		if(*src == '\t') {							// TAB
			src++;
			do {
				*dest++ = ' ';
			}while(++column % tabcolumn);
		} else if(*src == 0x0d || *src == 0x0a) {	// 改行
			column = 0;
			*dest++ = *src++;
		} else {
			*dest++ = *src++;
			column++;
		}
	}
	*dest = '\0';
	return dest2;
}


//=============================================================================
//	エスケープシーケンスの除去
//	カーソル移動系エスケープシーケンスには非対応
//=============================================================================
char *delesc(char *dest, const char *src)
{
	char	*dest2;
	uchar	*src2;
	uchar	*src3;
	size_t	i;

	if((src2 = src3 = (uchar *)malloc(strlen(src) + 32)) == NULL) {
		return NULL;
	};

	strcpy((char *)src2, src);

	// 31バイト連続 '\0' にする(8 以上なら OK なはずだけど，念のため）
	for(i = strlen((char *)src2); i < strlen((char *)src2) + 31; i++) {
		src2[i] = '\0';
	}

	dest2 = dest;

	do {
		if(*src2 == 0x1b) {		// エスケープシーケンス
			if(*(src2 + 1) == '[') {
				src2 += 2;
				while(*src2 && (toupper(*src2) < 'A' || toupper(*src2) > 'Z')) {
					if(_ismbblead(*src2)) {
						src2 += 2;
						continue;
					}
					src2++;
				}
				src2++;
			} else if(*(src2 + 1) == '=') {
				src2 += 4;
			} else if(*(src2 + 1) == ')' || *(src2 + 1) == '!') {
				src2 += 3;
			} else {
				src2 += 2;
			}
		} else {
			*dest++ = *src2++;		
		}
	}while(*src2 != '\0');

	free(src3);
	*dest = '\0';
	return dest2;
}


//=============================================================================
//	２バイト半角文字を半角に変換
//=============================================================================
char *zen2tohan(char *dest, const char *src)
{
	char *src2;
	char *src3;
	char *dest2;
	int	len;
	static const char *codetable[] = {
		"!",		/*	8540 	*/
		"\"",		/*	8541 	*/
		"#",		/*	8542 	*/
		"$",		/*	8543 	*/
		"%",		/*	8544 	*/
		"&",		/*	8545 	*/
		"'",		/*	8546 	*/
		"(",		/*	8547 	*/
		")",		/*	8548 	*/
		"*",		/*	8549 	*/
		"+",		/*	854a 	*/
		",",		/*	854b 	*/
		"-",		/*	854c 	*/
		".",		/*	854d 	*/
		"/",		/*	854e 	*/
		"0",		/*	854f 	*/
		"1",		/*	8550 	*/
		"2",		/*	8551 	*/
		"3",		/*	8552 	*/
		"4",		/*	8553 	*/
		"5",		/*	8554 	*/
		"6",		/*	8555 	*/
		"7",		/*	8556 	*/
		"8",		/*	8557 	*/
		"9",		/*	8558 	*/
		":",		/*	8559 	*/
		";",		/*	855a 	*/
		"<",		/*	855b 	*/
		"=",		/*	855c 	*/
		">",		/*	855d 	*/
		"?",		/*	855e 	*/
		"@",		/*	855f 	*/
		"A",		/*	8560 	*/
		"B",		/*	8561 	*/
		"C",		/*	8562 	*/
		"D",		/*	8563 	*/
		"E",		/*	8564 	*/
		"F",		/*	8565 	*/
		"G",		/*	8566 	*/
		"H",		/*	8567 	*/
		"I",		/*	8568 	*/
		"J",		/*	8569 	*/
		"K",		/*	856a 	*/
		"L",		/*	856b 	*/
		"M",		/*	856c 	*/
		"N",		/*	856d 	*/
		"O",		/*	856e 	*/
		"P",		/*	856f 	*/
		"Q",		/*	8570 	*/
		"R",		/*	8571 	*/
		"S",		/*	8572 	*/
		"T",		/*	8573 	*/
		"U",		/*	8574 	*/
		"V",		/*	8575 	*/
		"W",		/*	8576 	*/
		"X",		/*	8577 	*/
		"Y",		/*	8578 	*/
		"Z",		/*	8579 	*/
		"[",		/*	857a 	*/
		"\\",		/*	857b 	*/
		"]",		/*	857c 	*/
		"^",		/*	857d 	*/
		"_",		/*	857e 	*/
		"",			/*	857f 	*/
		"`",		/*	8580 	*/
		"a",		/*	8581 	*/
		"b",		/*	8582 	*/
		"c",		/*	8583 	*/
		"d",		/*	8584 	*/
		"e",		/*	8585 	*/
		"f",		/*	8586 	*/
		"g",		/*	8587 	*/
		"h",		/*	8588 	*/
		"i",		/*	8589 	*/
		"j",		/*	858a 	*/
		"k",		/*	858b 	*/
		"l",		/*	858c 	*/
		"m",		/*	858d 	*/
		"n",		/*	858e 	*/
		"o",		/*	858f 	*/
		"p",		/*	8590 	*/
		"q",		/*	8591 	*/
		"r",		/*	8592 	*/
		"s",		/*	8593 	*/
		"t",		/*	8594 	*/
		"u",		/*	8595 	*/
		"v",		/*	8596 	*/
		"w",		/*	8597 	*/
		"x",		/*	8598 	*/
		"y",		/*	8599 	*/
		"z",		/*	859a 	*/
		"{",		/*	859b 	*/
		"|",		/*	859c 	*/
		"}",		/*	859d 	*/
		"\x81\x45",		/*	859e 	*/
		"\xa1",		/*	859f 	*/
		"\xa2",		/*	85a0 	*/
		"\xa3",		/*	85a1 	*/
		"\xa4",		/*	85a2 	*/
		"\xa5",		/*	85a3 	*/
		"\xa6",		/*	85a4 	*/
		"\xa7",		/*	85a5 	*/
		"\xa8",		/*	85a6 	*/
		"\xa9",		/*	85a7 	*/
		"\xaa",		/*	85a8 	*/
		"\xab",		/*	85a9 	*/
		"\xac",		/*	85aa 	*/
		"\xad",		/*	85ab 	*/
		"\xae",		/*	85ac 	*/
		"\xaf",		/*	85ad 	*/
		"\xb0",		/*	85ae 	*/
		"\xb1",		/*	85af 	*/
		"\xb2",		/*	85b0 	*/
		"\xb3",		/*	85b1 	*/
		"\xb4",		/*	85b2 	*/
		"\xb5",		/*	85b3 	*/
		"\xb6",		/*	85b4 	*/
		"\xb7",		/*	85b5 	*/
		"\xb8",		/*	85b6 	*/
		"\xb9",		/*	85b7 	*/
		"\xba",		/*	85b8 	*/
		"\xbb",		/*	85b9 	*/
		"\xbc",		/*	85ba 	*/
		"\xbd",		/*	85bb 	*/
		"\xbe",		/*	85bc 	*/
		"\xbf",		/*	85bd 	*/
		"\xc0",		/*	85be 	*/
		"\xc1",		/*	85bf 	*/
		"\xc2",		/*	85c0 	*/
		"\xc3",		/*	85c1 	*/
		"\xc4",		/*	85c2 	*/
		"\xc5",		/*	85c3 	*/
		"\xc6",		/*	85c4 	*/
		"\xc7",		/*	85c5 	*/
		"\xc8",		/*	85c6 	*/
		"\xc9",		/*	85c7 	*/
		"\xca",		/*	85c8 	*/
		"\xcb",		/*	85c9 	*/
		"\xcc",		/*	85ca 	*/
		"\xcd",		/*	85cb 	*/
		"\xce",		/*	85cc 	*/
		"\xcf",		/*	85cd 	*/
		"\xd0",		/*	85ce 	*/
		"\xd1",		/*	85cf 	*/
		"\xd2",		/*	85d0 	*/
		"\xd3",		/*	85d1 	*/
		"\xd4",		/*	85d2 	*/
		"\xd5",		/*	85d3 	*/
		"\xd6",		/*	85d4 	*/
		"\xd7",		/*	85d5 	*/
		"\xd8",		/*	85d6 	*/
		"\xd9",		/*	85d7 	*/
		"\xda",		/*	85d8 	*/
		"\xdb",		/*	85d9 	*/
		"\xdc",		/*	85da 	*/
		"\xdd",		/*	85db 	*/
		"\xde",		/*	85dc 	*/
		"\xdf",		/*	85dd 	*/
		"\x81\x45",		/*	85de 	*/
		"\x81\x45",		/*	85df 	*/
		"\xdc",		/*	85e0 	*/
		"\xb6",		/*	85e1 	*/
		"\xb9",		/*	85e2 	*/
		"\xb3\xde",		/*	85e3 	*/
		"\xb6\xde",		/*	85e4 	*/
		"\xb7\xde",		/*	85e5 	*/
		"\xb8\xde",		/*	85e6 	*/
		"\xb9\xde",		/*	85e7 	*/
		"\xba\xde",		/*	85e8 	*/
		"\xbb\xde",		/*	85e9 	*/
		"\xbc\xde",		/*	85ea 	*/
		"\xbd\xde",		/*	85eb 	*/
		"\xbe\xde",		/*	85ec 	*/
		"\xbf\xde",		/*	85ed 	*/
		"\xc0\xde",		/*	85ee 	*/
		"\xc1\xde",		/*	85ef 	*/
		"\xc2\xde",		/*	85f0 	*/
		"\xc3\xde",		/*	85f1 	*/
		"\xc4\xde",		/*	85f2 	*/
		"\xca\xde",		/*	85f3 	*/
		"\xca\xdf",		/*	85f4 	*/
		"\xcb\xde",		/*	85f5 	*/
		"\xcb\xdf",		/*	85f6 	*/
		"\xcc\xde",		/*	85f7 	*/
		"\xcc\xdf",		/*	85f8 	*/
		"\xcd\xde",		/*	85f9 	*/
		"\xcd\xdf",		/*	85fa 	*/
		"\xce\xde",		/*	85fb 	*/
		"\xce\xdf"		/*	85fc 	*/
	};

	if((src2 = src3 = (char *)malloc(strlen(src) + 2)) == NULL) {
		return NULL;
	};

	strcpy(src2, src);
	src2[strlen(src2)+1] = '\0';		// 2バイト連続 '\0' にする

	dest2 = dest;
	do {
		if(_ismbblead(*(uchar *)src2)) {		// 漢字１バイト目
			if(*(uchar *)src2 == 0x85 &&
					*(uchar *)(src2+1) >= 0x40 && *(uchar *)(src2+1) <= 0xfc) {	// 2バイト半角
				len = (int)strlen(codetable[*(uchar *)(src2+1) - 0x40]);
				strncpy(dest, codetable[*(uchar *)(src2+1) - 0x40], len);
				src2 += 2;
				dest += len;
			} else {
				*dest++ = *src2++;
				*dest++ = *src2++;
			}
		} else {
			*dest++ = *src2++;
		}
	}while(*src2 != '\0');

	free(src3);
	if(strlen(dest2) > 0) {
		if(*(dest - 1) != '\0') {
			*dest = '\0';
		}
	}
	return dest2;
}
