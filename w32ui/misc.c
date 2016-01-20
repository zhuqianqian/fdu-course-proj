
#include "misc.h"
#include "../resources/resource.h"

static TCHAR _szFilter[] = TEXT("Snf204 Package Files (*.spf)\0*.SPF\0")\
						  TEXT("All Files (*.*)\0*.*\0\0");
static TCHAR _szDefExt[] = TEXT(".spf");

void 
InitOFN(OPENFILENAME *pofn, HWND hwnd)
{
	pofn->lStructSize		  = sizeof(OPENFILENAME);
	pofn->hwndOwner		  = hwnd;
	pofn->hInstance		  = NULL;
	pofn->lpstrFilter		  = _szFilter;
	pofn->lpstrCustomFilter = NULL;
	pofn->nMaxCustFilter	  = 0;
	pofn->nFilterIndex	  = 0;
	pofn->lpstrFile		  = NULL;
	pofn->nMaxFile		  = MAX_PATH;
	pofn->lpstrFileTitle	  = NULL;
	pofn->nMaxFileTitle	  = MAX_PATH;
	pofn->lpstrInitialDir	  = NULL;
	pofn->lpstrTitle		  = NULL;
	pofn->Flags			  = 0;
	pofn->nFileOffset		  = 0;
	pofn->nFileExtension	  = 0;
	pofn->lpstrDefExt       = _szDefExt;
	pofn->lCustData		  = 0L;
	pofn->lpfnHook		  = NULL;
	pofn->lpTemplateName	  = NULL;
}

BOOL ShowOpenDlg(OPENFILENAME *pofn, LPWSTR szFileName, LPWSTR szTitleName)
{
	pofn->lpstrFile	   = szFileName;
	pofn->lpstrFileTitle = szTitleName;
	pofn->Flags		   = OFN_HIDEREADONLY | OFN_CREATEPROMPT;
	
	return GetOpenFileName(pofn);
}

BOOL ShowSaveDlg(OPENFILENAME *pofn, LPWSTR szFileName, LPWSTR szTitleName)
{
	pofn->lpstrFile	   = szFileName;
	pofn->lpstrFileTitle = szTitleName;
	pofn->Flags			= OFN_OVERWRITEPROMPT;

	return GetSaveFileName(pofn);
}

BOOL
TempFileTrans(LPWSTR file1, LPWSTR file2)
{
	HANDLE hFile1, hFile2;
	DWORD dwLow, dwHigh;
	char *buf;
	hFile1 = CreateFile(file1, GENERIC_READ, FILE_SHARE_READ, 0, 
		OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if(hFile1 == INVALID_HANDLE_VALUE)
	{
		return GetLastError();
	}
	hFile2 = CreateFile(file2, GENERIC_WRITE, FILE_SHARE_READ, 0,
		CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile2 == INVALID_HANDLE_VALUE)
	{
		CloseHandle(hFile1);
		return GetLastError();
	}
	dwLow = GetFileSize(hFile1, &dwHigh);
	buf = (char *)malloc(dwLow);
	ReadFile(hFile1, (LPVOID)buf, dwLow, &dwHigh, 0);
	WriteFile(hFile2, (LPVOID)buf, dwLow, &dwHigh, 0);
	CloseHandle(hFile1);
	CloseHandle(hFile2);
	free(buf);
	return 0;
}

BOOL CALLBACK 
AboutDlgProc (HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND hwnd;
	RECT rcParent, rcThis;
	POINT pos;

	switch(msg)
	{
	case WM_INITDIALOG:
		hwnd = GetParent(hDlg);
		GetWindowRect(hwnd, &rcParent);
		GetWindowRect(hDlg, &rcThis);
		pos.y = (rcParent.bottom - rcParent.top - rcThis.bottom + rcThis.top)>>1;
		pos.x = (rcParent.right - rcParent.left - rcThis.right + rcThis.left)>>1;
		SetWindowPos(hDlg, NULL, 
			pos.x+rcParent.left, pos.y+rcParent.top, 0, 0,
			SWP_NOZORDER | SWP_NOSIZE);
		return TRUE;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
		case IDCANCEL:
			EndDialog(hDlg, 0);
			break;
		}
		break;

	case WM_CTLCOLORDLG:
	case WM_CTLCOLORSTATIC:
		return (INT_PTR)GetStockObject(WHITE_BRUSH);
	}
	return FALSE;
}

void
SetTempFile(LPWSTR szPath)
{
	LPWSTR ptr;
	ptr = szPath;
	while(*ptr)
		++ptr;
	while(*ptr != '\\')
		--ptr;
	++ptr;
	*ptr = 0;
	lstrcat(szPath, TEXT("~temp"));
}

BOOL
PopupAboutDlg(HINSTANCE hInstance, HWND hwnd)
{
	BOOL retval;
	retval = DialogBox(hInstance, MAKEINTRESOURCE(IDD_ABOUT), hwnd, AboutDlgProc);
	return retval;
}