#include "viewmem.h"
#include"pmdwin/pmdwinimport.h"
#include<DxLib.h>
#include"DxKeyTrigger.h"
static int look = 0;
static bool enable = false;
static TCHAR des[24][8] = {
	L"FM1",L"FM2",L"FM3",L"FM4",L"FM5",L"FM6",
	L"SSG1",L"SSG2",L"SSG3",
	L"ADPCM",
	L"OPNAR",
	L"Ext1",L"Ext2",L"Ext3",
	L"Rhy",
	L"Eff",
	L"PPZ",L"PPZ",L"PPZ",L"PPZ",L"PPZ",L"PPZ",L"PPZ",L"PPZ"
};

DxKeyTrigger key_prev, key_next;

void ViewMemInit()
{
	key_prev.SetKey(KEY_INPUT_COMMA);
	key_next.SetKey(KEY_INPUT_PERIOD);
}

void ViewMemDraw()
{
	if (!enable)
		return;
	if (key_next.Pressed()&&look<23)
		look++;
	if (key_prev.Pressed()&&look>-1)
		look--;
	clsDx();
	if (look == -1)
	{
		printfDx(TEXT("Open Work"));
		for (int i = 0; i < 524; i++)
		{
			if (i % 16 == 0)printfDx(TEXT("\n"));
			printfDx(TEXT("%02x "), ((char*)getopenwork())[i] & 0xFF);
		}
	}
	else
	{
		printfDx(TEXT("Part Work %d (%s)"), look, des[look]);
		for (int i = 0; i < sizeof QQ; i++)
		{
			if (i % 16 == 0)printfDx(TEXT("\n"));
			printfDx(TEXT("%02x "), ((char*)getpartwork(look))[i] & 0xFF);
		}
	}
}

void ViewMemEnable(bool e)
{
	enable = e;
	if (!enable)
		clsDx();
}

bool ViewMemGetEnabled()
{
	return enable;
}
