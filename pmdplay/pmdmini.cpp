#include <string.h>
#include <stdio.h>

#include "pmdwin/pmdwin.h"
#include "pmdmini.h"

int pmd_length = 0;
int pmd_loop = 0;

char pmd_title[1024];
char pmd_compo[1024];
TCHAR pmd_file[2048];

OPEN_WORK *pmdwork = NULL;

//
// path splitter
//

static int pmd_split_dir( const TCHAR *file , TCHAR *dir )
{
	const TCHAR*p;
	int len = 0;
	
	p = _tcsrchr( file , '/' );

	if ( p )
	{
		len = (int)( p - file );
		lstrcpyn ( dir , file , len );
	}
	dir[ len ] = 0;
	
	return len;
}

//
// 初期化
//

void pmd_init(void)
{
	TCHAR *current_dir = (TCHAR *)_T("./");
	
	pmdwininit( current_dir );
	setpcmrate( SOUND_55K );
	
	pmdwork = NULL;
	
	pmd_length = 0;
	pmd_loop = 0;
}


//
//　周波数設定
//

void pmd_setrate( int freq )
{
	setpcmrate( freq );
}

//
// ファイルチェック
//

int pmd_is_pmd( const wchar_t*file )
{
	int  size;
	unsigned char header[3];

	FILE *fp;

	fp = _tfopen(file,_T("rb"));
	
	if (!fp)
		return 0;
	
	size = (int)fread(header,1,3,fp);
	
	fclose(fp);

	
	if (size != 3)
		return 0;
	
	if (header[0] > 0x0f)
		return 0;

	if (header[1] != 0x18 && header[1] !=0x1a )
		return 0;

	if (header[2] && header[2] != 0xe6)
		return 0;
	
	return 1;
}

//
// エラーであれば0以外を返す
//

int pmd_play ( const wchar_t*file , wchar_t*pcmdir )
{
	TCHAR dir[2048];

	TCHAR *path[4];
	TCHAR *current_dir = (TCHAR *)_T("./");
	
	if ( ! pmd_is_pmd ( file ) )
		return 1;

	lstrcpy ( pmd_file , file );

	if ( pmd_split_dir( file , dir ) > 0 )
	{
		path[0] = dir;
		path[1] = pcmdir;
		path[2] = current_dir;
		path[3] = NULL;
	}
	else
	{
		path[0] = current_dir;
		path[1] = pcmdir;
		path[2] = NULL;
	}
	
	setpcmdir( path );
	
	// get song length in sec
	if (!getlength( pmd_file , &pmd_length , &pmd_loop ))
	{
		pmd_length = 0;
		pmd_loop = 0;
	}
	
	pmd_title[0] = 0;
	pmd_compo[0] = 0;
	
	fgetmemo3( pmd_title , pmd_file , 1 );
	fgetmemo3( pmd_compo , pmd_file , 2 );
	
	
	music_load( pmd_file );
	music_start();
	
	pmdwork = getopenwork();
	
	return 0;
}

//
// トラック数
//
int pmd_get_tracks( void )
{
	return NumOfAllPart;
}


//
// 現在のノート
//
void pmd_get_current_notes ( int *notes , int len )
{
	int i = 0;
	
	for ( i = 0; i < len; i++ ) notes[i] = -1;
	
	if ( ! pmdwork )
		return;
		
	for ( i = 0; i < len ; i++ )
	{
		int data = pmdwork->MusPart[i]->onkai;
		
		if (data == 0xff)
			notes[i] = -1;
		else
			notes[i] = data;		
	}
}


int pmd_length_sec ( void )
{
	return pmd_length / 1000;
} 

int pmd_loop_sec ( void )
{
	return pmd_loop / 1000;
}

void pmd_renderer ( short *buf , int len )
{
	getpcmdata ( buf , len );
}

void pmd_stop ( void )
{	
	music_stop();
	pmdwork = NULL;

}

void pmd_get_title( char *dest )
{
	strcpy( dest , pmd_title );
}

void pmd_get_compo( char *dest )
{
	strcpy( dest , pmd_compo );
}

