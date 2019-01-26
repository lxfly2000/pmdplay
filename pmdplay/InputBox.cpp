#include "InputBox.h"
#include<windowsx.h>
#include"InputBoxResource.h"

struct InitParam
{
	PTSTR paramOut;
	int paramMaxLen;
	PCTSTR paramText;
	PCTSTR paramTitle;
	PCTSTR paramDefInput;
	PCTSTR paramExButton;
	PCTSTR paramSZOK;
	PCTSTR paramSZCancel;
};

void Finish(HWND hwnd, PTSTR pOut, int maxlen, int retcode)
{
	GetDlgItemText(hwnd, IDC_EDIT_INPUT, pOut, maxlen == -1 ? Edit_GetTextLength(GetDlgItem(hwnd, IDC_EDIT_INPUT)) : maxlen);
	EndDialog(hwnd, retcode);
}

void OnTextChange(HWND hwnd, int maxlen)
{
	TCHAR text[16];
	wsprintf(text, TEXT("%d/%d"), Edit_GetTextLength(GetDlgItem(hwnd, IDC_EDIT_INPUT)), maxlen - 1);
	SetDlgItemText(hwnd, IDC_STATIC_COUNT, text);
}

InitParam *pip;

INT_PTR CALLBACK InputBoxCallback(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
	case WM_INITDIALOG:
	{
		if (pip->paramMaxLen != -1)
		{
			Edit_LimitText(GetDlgItem(hwnd, IDC_EDIT_INPUT), pip->paramMaxLen - 1);
			OnTextChange(hwnd, pip->paramMaxLen);
		}
		else
		{
			DestroyWindow(GetDlgItem(hwnd, IDC_STATIC_COUNT));
			RECT rs, re;
			GetWindowRect(GetDlgItem(hwnd, IDC_STATIC_MESSAGE), &rs);
			GetWindowRect(GetDlgItem(hwnd, IDC_EDIT_INPUT), &re);
			POINT lt = { rs.left,rs.top };
			ScreenToClient(hwnd, &lt);
			MoveWindow(GetDlgItem(hwnd, IDC_STATIC_MESSAGE), lt.x, lt.y, re.right - re.left, rs.bottom - rs.top, FALSE);
		}
		if (pip->paramText)SetDlgItemText(hwnd, IDC_STATIC_MESSAGE, pip->paramText);
		if (pip->paramTitle)SetWindowText(hwnd, pip->paramTitle);
		if (pip->paramDefInput)SetDlgItemText(hwnd, IDC_EDIT_INPUT, pip->paramDefInput);
		if (pip->paramExButton)SetDlgItemText(hwnd, IDC_BUTTON_EXTRA, pip->paramExButton);
		else DestroyWindow(GetDlgItem(hwnd, IDC_BUTTON_EXTRA));
		if (pip->paramSZOK)SetDlgItemText(hwnd, IDOK, pip->paramSZOK);
		if (pip->paramSZCancel)SetDlgItemText(hwnd, IDCANCEL, pip->paramSZCancel);
		SetFocus(hwnd);
	}
		break;
	case WM_COMMAND:
		switch (LOWORD(wp))
		{
		case IDOK:Finish(hwnd, pip->paramOut, pip->paramMaxLen, 1); break;
		case IDCANCEL:Finish(hwnd, pip->paramOut, pip->paramMaxLen, 0); break;
		case IDC_BUTTON_EXTRA:Finish(hwnd, pip->paramOut, pip->paramMaxLen, 2); break;
		case IDC_EDIT_INPUT:
			switch (HIWORD(wp))
			{
			case EN_CHANGE:OnTextChange(hwnd, pip->paramMaxLen); break;
			}
			break;
		}
		break;
	}
	return 0;
}

int InputBox(HWND hwndParent, PTSTR pszOut, int maxlen, PCTSTR msgText, PCTSTR title, PCTSTR defaultInput,
	PCTSTR textExtraButton, PCTSTR textOK, PCTSTR textCancel)
{
	InitParam p = { pszOut,maxlen,msgText,title,defaultInput,textExtraButton,textOK,textCancel };
	pip = &p;
	return (int)DialogBox(NULL, MAKEINTRESOURCE(IDD_DIALOG_INPUTBOX), hwndParent, InputBoxCallback);
}
