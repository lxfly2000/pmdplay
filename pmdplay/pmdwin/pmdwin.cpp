//=============================================================================
//	Professional Music Driver [P.M.D.] version 4.8
//			Programmed By M.Kajihara
//			Windows Converted by C60
//=============================================================================

#include "pmdwincore.h"


PMDWIN* pmdwin;
PMDWIN* pmdwin2;

//=============================================================================
//	DLL Export Functions
//=============================================================================

BOOL WINAPI DllMain(HINSTANCE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			DisableThreadLibraryCalls(hModule);
			pmdwin = pmdwin2 = NULL;
			break;
			
		case DLL_THREAD_ATTACH:
			break;
			
		case DLL_THREAD_DETACH:
			break;
			
		case DLL_PROCESS_DETACH:
			delete pmdwin;
			delete pmdwin2;
			break;
    }
    return TRUE;
}


#ifdef __cplusplus
extern "C" {
#endif


//=============================================================================
//	バーション取得
//=============================================================================
__declspec(dllexport) int WINAPI getversion(void)
{
	return	DLLVersion;
}


//=============================================================================
//	インターフェイスのバーション取得
//=============================================================================
__declspec(dllexport) int WINAPI getinterfaceversion(void)
{
	return	InterfaceVersion;
}


//=============================================================================
//	COM 風インターフェイス(PMDWIN インスタンスの取得)
//=============================================================================
__declspec(dllexport) HRESULT WINAPI pmd_CoCreateInstance(
  REFCLSID rclsid,     //Class identifier (CLSID) of the object
  LPUNKNOWN pUnkOuter, //Pointer to whether object is or isn't part 
                       // of an aggregate
  DWORD dwClsContext,  //Context for running executable code
  REFIID riid,         //Reference to the identifier of the interface
  LPVOID * ppv         //Address of output variable that receives 
                       // the interface pointer requested in riid
)
{
	IUnknown* pUnknown;
	
	if(rclsid != CLSID_PMDWIN) {
		return REGDB_E_CLASSNOTREG;		// CLSID が違う
	} else if(riid != IID_IPCMMUSICDRIVER && riid != IID_IFMPMD && riid != IID_IPMDWIN) {
		return REGDB_E_CLASSNOTREG;		// IID が違う
	} else {
		pUnknown = (IUnknown*)new(PMDWIN);
		*ppv = (void **)pUnknown;
		pUnknown->AddRef();
		return S_OK;
	}
}


//=============================================================================
//	初期化
//=============================================================================
__declspec(dllexport) bool WINAPI pmdwininit(TCHAR *path)
{
	bool result;
	
	if(pmdwin == NULL) {
		pmdwin = new PMDWIN;
		pmdwin2 = new PMDWIN;
	}
	
	pmdwin2->init(path);
	result = pmdwin->init(path);
/*
	pmdwin2->setfmwait(0);			// 曲長計算高速化のため
	pmdwin2->setssgwait(0);			// 曲長計算高速化のため
	pmdwin2->setrhythmwait(0);		// 曲長計算高速化のため
	pmdwin2->setadpcmwait(0);		// 曲長計算高速化のため
*/
	return result;
}


//=============================================================================
//	リズム音の再読み込み
//=============================================================================
__declspec(dllexport) bool WINAPI loadrhythmsample(TCHAR *path)
{
	return pmdwin->loadrhythmsample(path);
}


//=============================================================================
//	（补充）从内存加载节奏声音
//=============================================================================
__declspec(dllexport) bool WINAPI loadrhythmsample_mem(char* bd, char* sd, char* top, char* hh, char* tom, char* rim)
{
	return pmdwin->loadrhythmsample_mem(bd, sd, top, hh, tom, rim);
}


//=============================================================================
//	PCM 検索ディレクトリの設定
//=============================================================================
__declspec(dllexport) bool WINAPI setpcmdir(TCHAR **path)
{
	return pmdwin->setpcmdir(path);
}


//=============================================================================
//	合成周波数の設定
//=============================================================================
__declspec(dllexport) void WINAPI setpcmrate(int rate)
{
	pmdwin->setpcmrate(rate);
}


//=============================================================================
//	PPZ 合成周波数の設定
//=============================================================================
__declspec(dllexport) void WINAPI setppzrate(int rate)
{
	pmdwin->setppzrate(rate);
}


//=============================================================================
//	PPS を鳴らすか？
//=============================================================================
__declspec(dllexport) void WINAPI setppsuse(bool value)
{
	pmdwin->setppsuse(value);
}


//=============================================================================
//	SSG 効果音で OPNA Rhythm も鳴らすか？
//=============================================================================
__declspec(dllexport) void WINAPI setrhythmwithssgeffect(bool value)
{
	pmdwin->setrhythmwithssgeffect(value);
}


//=============================================================================
//	PMD86 の PCM を PMDB2 互換にするか？
//=============================================================================
__declspec(dllexport) void WINAPI setpmd86pcmmode(bool value)
{
	pmdwin->setpmd86pcmmode(value);
}


//=============================================================================
//	PMD86 の PCM が PMDB2 互換かどうかを取得する
//=============================================================================
__declspec(dllexport) bool WINAPI getpmd86pcmmode(void)
{
	return pmdwin->getpmd86pcmmode();
}


//=============================================================================
//	曲の読み込みその１（ファイルから）
//=============================================================================
__declspec(dllexport) int WINAPI music_load(TCHAR *filename)
{
	return pmdwin->music_load(filename);
}


//=============================================================================
//	曲の読み込みその２（メモリから）
//=============================================================================
__declspec(dllexport) int WINAPI music_load2(uchar *musdata, int size)
{
	return pmdwin->music_load2(musdata, size);
}


//=============================================================================
//	演奏開始
//=============================================================================
__declspec(dllexport) void WINAPI music_start(void)
{
	pmdwin->music_start();
}


//=============================================================================
//	演奏停止
//=============================================================================
__declspec(dllexport) void WINAPI music_stop(void)
{
	pmdwin->music_stop();
}


//=============================================================================
//	フェードアウト(PMD互換)
//=============================================================================
__declspec(dllexport) void WINAPI fadeout(int speed)
{
	pmdwin->fadeout(speed);
}


//=============================================================================
//	フェードアウト(高音質)
//=============================================================================
__declspec(dllexport) void WINAPI fadeout2(int speed)
{
	pmdwin->fadeout2(speed);
}


//=============================================================================
//	PCM データ（wave データ）の取得
//=============================================================================
__declspec(dllexport) void WINAPI getpcmdata(short *buf, int nsamples)
{
	pmdwin->getpcmdata(buf, nsamples);
}


//=============================================================================
//	FM で 55kHz合成、一次補完するかどうかの設定
//=============================================================================
__declspec(dllexport) void WINAPI setfmcalc55k(bool flag)
{
	pmdwin->setfmcalc55k(flag);
}


//=============================================================================
//	PPS で一次補完するかどうかの設定
//=============================================================================
__declspec(dllexport) void WINAPI setppsinterpolation(bool ip)
{
	pmdwin->setppsinterpolation(ip);
}


//=============================================================================
//	P86 で一次補完するかどうかの設定
//=============================================================================
__declspec(dllexport) void WINAPI setp86interpolation(bool ip)
{
	pmdwin->setp86interpolation(ip);
}


//=============================================================================
//	PPZ8 で一次補完するかどうかの設定
//=============================================================================
__declspec(dllexport) void WINAPI setppzinterpolation(bool ip)
{
	pmdwin->setppzinterpolation(ip);
}


//=============================================================================
//	メモの取得
//=============================================================================
__declspec(dllexport) char * WINAPI getmemo(char *dest, uchar *musdata, int size, int al)
{
	return pmdwin->getmemo(dest, musdata, size, al);
}


//=============================================================================
//	メモの取得（２バイト半角→半角文字に変換）
//=============================================================================
__declspec(dllexport) char * WINAPI getmemo2(char *dest, uchar *musdata, int size, int al)
{
	return pmdwin->getmemo2(dest, musdata, size, al);
}


//=============================================================================
//	メモの取得（２バイト半角→半角文字に変換＋ESCシーケンスの除去）
//=============================================================================
__declspec(dllexport) char * WINAPI getmemo3(char *dest, uchar *musdata, int size, int al)
{
	return pmdwin->getmemo3(dest, musdata, size, al);
}


//=============================================================================
//	メモの取得（ファイル名から）
//=============================================================================
__declspec(dllexport) int WINAPI fgetmemo(char *dest, TCHAR *filename, int al)
{
	return pmdwin->fgetmemo(dest, filename, al);
}


//=============================================================================
//	メモの取得（ファイル名から／２バイト半角→半角文字に変換）
//=============================================================================
__declspec(dllexport) int WINAPI fgetmemo2(char *dest, TCHAR *filename, int al)
{
	return pmdwin->fgetmemo2(dest, filename, al);
}


//=============================================================================
//	メモの取得（ファイル名から／半角文字に変換＋ESCシーケンスの除去）
//=============================================================================
__declspec(dllexport) int WINAPI fgetmemo3(char *dest, TCHAR *filename, int al)
{
	return pmdwin->fgetmemo3(dest, filename, al);
}


//=============================================================================
//	曲の filename の取得
//=============================================================================
__declspec(dllexport) TCHAR * WINAPI getmusicfilename(TCHAR *dest)
{
	return pmdwin->getmusicfilename(dest);
}


//=============================================================================
//	PPC / P86 filename の取得
//=============================================================================
__declspec(dllexport) TCHAR * WINAPI getpcmfilename(TCHAR *dest)
{
	return pmdwin->getpcmfilename(dest);
}


//=============================================================================
//	PPC filename の取得
//=============================================================================
__declspec(dllexport) TCHAR * WINAPI getppcfilename(TCHAR *dest)
{
	return pmdwin->getppcfilename(dest);
}


//=============================================================================
//	PPS filename の取得
//=============================================================================
__declspec(dllexport) TCHAR * WINAPI getppsfilename(TCHAR *dest)
{
	return pmdwin->getppsfilename(dest);
}


//=============================================================================
//	P86 filename の取得
//=============================================================================
__declspec(dllexport) TCHAR * WINAPI getp86filename(TCHAR *dest)
{
	return pmdwin->getp86filename(dest);
}


//=============================================================================
//	PPZ filename の取得
//=============================================================================
__declspec(dllexport) TCHAR * WINAPI getppzfilename(TCHAR *dest, int bufnum)
{
	return pmdwin->getppzfilename(dest, bufnum);
}


//=============================================================================
//	.PPC の読み込み（ファイルから）
//=============================================================================
__declspec(dllexport) int WINAPI ppc_load(TCHAR *filename)
{
	return pmdwin->ppc_load(filename);
}


//=============================================================================
//	.PPS の読み込み（ファイルから）
//=============================================================================
__declspec(dllexport) int WINAPI pps_load(TCHAR *filename)
{
	return pmdwin->pps_load(filename);
}


//=============================================================================
//	.P86 の読み込み（ファイルから）
//=============================================================================
__declspec(dllexport) int WINAPI p86_load(TCHAR *filename)
{
	return pmdwin->p86_load(filename);
}


//=============================================================================
//	.PZI, .PVI の読み込み（ファイルから）
//=============================================================================
__declspec(dllexport) int WINAPI ppz_load(TCHAR *filename, int bufnum)
{
	return pmdwin->ppz_load(filename, bufnum);
}


//=============================================================================
//	パートのマスク
//=============================================================================
__declspec(dllexport) int WINAPI maskon(int ch)
{
	return pmdwin->maskon(ch);
}


//=============================================================================
//	パートのマスク解除
//=============================================================================
__declspec(dllexport) int WINAPI maskoff(int ch)
{
	return pmdwin->maskoff(ch);
}


//=============================================================================
//	FM Volume Down の設定
//=============================================================================
__declspec(dllexport) void WINAPI setfmvoldown(int voldown)
{
	pmdwin->setfmvoldown(voldown);
}


//=============================================================================
//	SSG Volume Down の設定
//=============================================================================
__declspec(dllexport) void WINAPI setssgvoldown(int voldown)
{
	pmdwin->setssgvoldown(voldown);
}


//=============================================================================
//	Rhythm Volume Down の設定
//=============================================================================
__declspec(dllexport) void WINAPI setrhythmvoldown(int voldown)
{
	pmdwin->setrhythmvoldown(voldown);
}


//=============================================================================
//	ADPCM Volume Down の設定
//=============================================================================
__declspec(dllexport) void WINAPI setadpcmvoldown(int voldown)
{
	pmdwin->setadpcmvoldown(voldown);
}


//=============================================================================
//	PPZ8 Volume Down の設定
//=============================================================================
__declspec(dllexport) void WINAPI setppzvoldown(int voldown)
{
	pmdwin->setppzvoldown(voldown);
}


//=============================================================================
//	FM Volume Down の取得
//=============================================================================
__declspec(dllexport) int WINAPI getfmvoldown(void)
{
	return pmdwin->getfmvoldown();
}


//=============================================================================
//	FM Volume Down の取得（その２）
//=============================================================================
__declspec(dllexport) int WINAPI getfmvoldown2(void)
{
	return pmdwin->getfmvoldown2();
}


//=============================================================================
//	SSG Volume Down の取得
//=============================================================================
__declspec(dllexport) int WINAPI getssgvoldown(void)
{
	return pmdwin->getssgvoldown();
}


//=============================================================================
//	SSG Volume Down の取得（その２）
//=============================================================================
__declspec(dllexport) int WINAPI getssgvoldown2(void)
{
	return pmdwin->getssgvoldown2();
}


//=============================================================================
//	Rhythm Volume Down の取得
//=============================================================================
__declspec(dllexport) int WINAPI getrhythmvoldown(void)
{
	return pmdwin->getrhythmvoldown();
}


//=============================================================================
//	Rhythm Volume Down の取得（その２）
//=============================================================================
__declspec(dllexport) int WINAPI getrhythmvoldown2(void)
{
	return pmdwin->getrhythmvoldown2();
}


//=============================================================================
//	ADPCM Volume Down の取得
//=============================================================================
__declspec(dllexport) int WINAPI getadpcmvoldown(void)
{
	return pmdwin->getadpcmvoldown();
}


//=============================================================================
//	ADPCM Volume Down の取得（その２）
//=============================================================================
__declspec(dllexport) int WINAPI getadpcmvoldown2(void)
{
	return pmdwin->getadpcmvoldown2();
}


//=============================================================================
//	PPZ8 Volume Down の取得
//=============================================================================
__declspec(dllexport) int WINAPI getppzvoldown(void)
{
	return pmdwin->getppzvoldown();
}


//=============================================================================
//	PPZ8 Volume Down の取得（その２）
//=============================================================================
__declspec(dllexport) int WINAPI getppzvoldown2(void)
{
	return pmdwin->getppzvoldown2();
}


//=============================================================================
//	再生位置の移動(pos : ms)
//=============================================================================
__declspec(dllexport) void WINAPI setpos(int pos)
{
	pmdwin->setpos(pos);
}


//=============================================================================
//	再生位置の移動(pos : count 単位)
//=============================================================================
__declspec(dllexport) void WINAPI setpos2(int pos)
{
	pmdwin->setpos2(pos);
}


//=============================================================================
//	再生位置の取得(pos : ms)
//=============================================================================
__declspec(dllexport) int WINAPI getpos(void)
{
	return pmdwin->getpos();
}


//=============================================================================
//	再生位置の取得(pos : count 単位)
//=============================================================================
__declspec(dllexport) int WINAPI getpos2(void)
{
	return pmdwin->getpos2();
}


//=============================================================================
//	曲の長さの取得(pos : ms)
//=============================================================================
__declspec(dllexport) bool WINAPI getlength(TCHAR *filename, int *length, int *loop)
{
	return pmdwin2->getlength(filename, length, loop);
}


//=============================================================================
//	曲の長さの取得(pos : count 単位)
//=============================================================================
__declspec(dllexport) bool WINAPI getlength2(TCHAR *filename, int *length, int *loop)
{
	return pmdwin2->getlength2(filename, length, loop);
}

//=============================================================================
//	ループ回数の取得
//=============================================================================
__declspec(dllexport) int WINAPI getloopcount(void)
{
	return pmdwin->getloopcount();
}


//=============================================================================
//	FM の Register 出力後の wait 設定
//=============================================================================
__declspec(dllexport) void WINAPI setfmwait(int nsec)
{
	pmdwin->setfmwait(nsec);
}


//=============================================================================
//	SSG の Register 出力後の wait 設定
//=============================================================================
__declspec(dllexport) void WINAPI setssgwait(int nsec)
{
	pmdwin->setssgwait(nsec);
}


//=============================================================================
//	rhythm の Register 出力後の wait 設定
//=============================================================================
__declspec(dllexport) void WINAPI setrhythmwait(int nsec)
{
	pmdwin->setrhythmwait(nsec);
}


//=============================================================================
//	ADPCM の Register 出力後の wait 設定
//=============================================================================
__declspec(dllexport) void WINAPI setadpcmwait(int nsec)
{
	pmdwin->setadpcmwait(nsec);
}


//=============================================================================
//	OPEN_WORKのポインタの取得
//=============================================================================
__declspec(dllexport) OPEN_WORK * WINAPI getopenwork(void)
{
	return pmdwin->getopenwork();
}


//=============================================================================
//	パートワークのポインタの取得
//=============================================================================
__declspec(dllexport) QQ * WINAPI getpartwork(int ch)
{
	return pmdwin->getpartwork(ch);
}


#ifdef __cplusplus
}
#endif
