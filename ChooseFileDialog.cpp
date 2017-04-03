#include"ChooseFileDialog.h"
#include<ShObjIdl.h>//新对话框使用
#include<ShlObj.h>//旧对话框使用

BOOL ChooseFile(HWND hWndParent, TCHAR *filepath, TCHAR *filename, PCTSTR pcszFiletype, PCTSTR pcszTitle)
{
	OPENFILENAME ofn = { sizeof OPENFILENAME };
	ofn.lStructSize = sizeof OPENFILENAME;
	ofn.hwndOwner = hWndParent;
	ofn.hInstance = nullptr;
	ofn.lpstrFilter = pcszFiletype;
	ofn.lpstrFile = filepath;
	ofn.lpstrTitle = pcszTitle;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFileTitle = filename;
	ofn.nMaxFileTitle = MAX_PATH;
	ofn.Flags = OFN_HIDEREADONLY;
	return GetOpenFileName(&ofn);
}

//http://blog.csdn.net/xhhjin/article/details/6550467
//释放使用 CoTaskMemFree
LPITEMIDLIST GetItemIDListFromFilePath(LPCTSTR strFilePath)
{
	if (!strFilePath)
	{
		return NULL;
	}
	// 得到桌面的目录
	LPSHELLFOLDER pDesktopFolder = NULL;
	HRESULT hr = SHGetDesktopFolder(&pDesktopFolder);
	if (FAILED(hr))
	{
		return NULL;
	}
	// 得到文件路径对应的ItemIDList
	LPITEMIDLIST pItemIDList = NULL;
	TCHAR strbuf[MAX_PATH];
	lstrcpy(strbuf, strFilePath);
	hr = pDesktopFolder->ParseDisplayName(NULL, NULL, strbuf, NULL, &pItemIDList, NULL);
	pDesktopFolder->Release();
	if (FAILED(hr))
	{
		return NULL;
	}
	return pItemIDList;
}

BOOL ChooseDirectory(HWND hWndParent, TCHAR *fullPath, PCTSTR pcszDefaultPath, PCTSTR pcszTitle)
{
	IFileDialog *pf;
	CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pf));
	FILEOPENDIALOGOPTIONS odo;
	pf->GetOptions(&odo);
	pf->SetOptions(FOS_PICKFOLDERS | odo);
	pf->SetTitle(pcszTitle);
	IShellItem *psi;
	if (pcszDefaultPath)
	{
		LPITEMIDLIST pi = GetItemIDListFromFilePath(pcszDefaultPath);
		SHCreateShellItem(NULL, NULL, pi, &psi);
		CoTaskMemFree(pi);
		pf->SetFolder(psi);
		psi->Release();
	}
	if (pf->Show(hWndParent) == HRESULT_FROM_WIN32(ERROR_CANCELLED))
	{
		pf->Release();
		return FALSE;
	}
	TCHAR *compath;
	pf->GetResult(&psi);
	psi->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, &compath);
	lstrcpy(fullPath, compath);
	CoTaskMemFree(compath);
	psi->Release();
	pf->Release();
	return TRUE;
}

//https://msdn.microsoft.com/zh-cn/library/windows/desktop/bb762115(v=vs.85).aspx
BOOL ChooseDirectoryClassic(HWND hWndParent, TCHAR *fullPath, PCTSTR pcszDefaultPath, PCTSTR pcszInstruction)
{
	BROWSEINFO bi;
	bi.hwndOwner = hWndParent;
	bi.pidlRoot = NULL;
	bi.pszDisplayName = fullPath;
	bi.lpszTitle = pcszInstruction;
	bi.ulFlags = BIF_USENEWUI;
	bi.lParam = (LPARAM)pcszDefaultPath;
	//https://www.arclab.com/en/kb/cppmfc/select-folder-shbrowseforfolder.html
	bi.lpfn = [](HWND hwnd, UINT msg, LPARAM lParam, LPARAM lpData)
	{
		if (msg == BFFM_INITIALIZED)
			SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
		return 0;
	};
	bi.iImage = NULL;
	LPITEMIDLIST pi = SHBrowseForFolder(&bi);

	if (pi)
	{
		if (SHGetPathFromIDList(pi, fullPath))
			return TRUE;
	}
	return FALSE;
}