//简单实现的更新功能
#include"Update.h"
#include"resource.h"
#include<Windows.h>
#include<WinInet.h>
#include<sstream>
#include<string>
#pragma comment(lib,"WinInet.lib")
int DownloadFile(const TCHAR *host, const TCHAR *path, std::stringstream &ssf)
{
	HINTERNET hOpen = InternetOpen(TEXT("KaikiUpdate"), INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);//打开连接，获得Internet句柄
	if (!hOpen)return -1;
	HINTERNET hConnect = InternetConnect(hOpen, host, INTERNET_DEFAULT_HTTPS_PORT,
		TEXT(""), TEXT(""), INTERNET_SERVICE_HTTP, 0, 0);//连接，获得连接句柄
	if (!hConnect)return -1;
	HINTERNET hReq = HttpOpenRequest(hConnect, TEXT("GET"), path,
		HTTP_VERSION, TEXT(""), NULL, INTERNET_FLAG_SECURE, 0);//打开请求，获得请求句柄
	if (!hReq)return -1;
	BOOL status = HttpSendRequest(hReq, NULL, 0, NULL, 0);//发送
	if (status == FALSE)return -2;//断网时在这里出错
	char szBuffer[1024] = "";
	DWORD dwByteRead = 0;
	do
	{
		status = InternetReadFile(hReq, szBuffer, ARRAYSIZE(szBuffer), &dwByteRead);
		if (status == FALSE)return -1;
		ssf.write(szBuffer, dwByteRead);
		ZeroMemory(szBuffer, dwByteRead);
	} while (dwByteRead);
	InternetCloseHandle(hReq);
	InternetCloseHandle(hConnect);
	InternetCloseHandle(hOpen);
	return 0;
}

int UpdateGetVersion(int *newVersion, std::stringstream &ssf)
{
	ssf.seekp(0);
	std::string sline, p1, p2;
	int dwver = 0, tmp, left = 4, curver = 0;
	std::stringstream subs;
	while (!ssf.eof())
	{
		std::getline(ssf, sline);
		subs.clear();
		subs.str(sline);
		subs >> p1;
		if (p1 == "#define")
		{
			subs >> p2;
			if (p2 == "APP_VERSION_MAJOR")
			{
				subs >> tmp;
				dwver += tmp << 24;
				left--;
			}
			else if (p2 == "APP_VERSION_MINOR")
			{
				subs >> tmp;
				dwver += tmp << 16;
				left--;
			}
			else if (p2 == "APP_VERSION_REVISION")
			{
				subs >> tmp;
				dwver += tmp << 8;
				left--;
			}
			else if (p2 == "APP_VERSION_BUILD")
			{
				subs >> tmp;
				dwver += tmp;
				left--;
			}
		}
		if (left == 0)break;
	}
	if (left)return -1;
	*newVersion = dwver;
	curver = (APP_VERSION_MAJOR << 24) + (APP_VERSION_MINOR << 16) + (APP_VERSION_REVISION << 8) + APP_VERSION_BUILD;
	if (dwver > curver)return 1;
	return 0;
}

//出错返回负数，无更新返回0，有更新返回1
int CheckForUpdate(const TCHAR *fileURL, int *newVersion)
{
	std::stringstream ssf;
	TCHAR _url[1024] = TEXT(""), *purl = _url;
	lstrcpy(purl, fileURL);
	if (_wcsnicmp(purl, L"https://", 8) == 0)purl += 8;
	wchar_t *ssp = wcschr(purl, '/');
	*ssp = 0;
	ssp++;
	int retcode = DownloadFile(purl, ssp, ssf);
	if (retcode < 0)return retcode;
	return UpdateGetVersion(newVersion, ssf);
}
