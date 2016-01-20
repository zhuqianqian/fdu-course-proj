#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "comdlg32.lib")


#include "snfwgui.h"
#include "runview.h"
#include "option.h"
#include "misc.h"

#include "../include/snfcore.h"
#include "../include/fileutil.h"
#include "../resources/resource.h"

#define BUFFERSIZE	(32<<20)
#define INDEXCOUNT	(64<<10)

#define S_NOTREADY	(-1)
#define S_READY		(3)
#define S_STOPPED	(0)
#define S_RUNNING	(1)
#define S_PAUSED	(2)

unsigned long count = 0;
unsigned long total = 0;
char *buffer;
unsigned int *index;
unsigned int *offset;
unsigned int ic = 0;
unsigned int bs = 0;

HINSTANCE g_hInstance;
HCURSOR	g_hNS, g_hWE, g_hArrow;
OPENFILENAME g_ofn;
TCHAR g_szFileName[MAX_PATH];
TCHAR g_szTitleName[MAX_PATH];
TCHAR g_szTempW[MAX_PATH];
char  g_szTempA[MAX_PATH];

const TCHAR szAppName[] = TEXT("snf204");
const TCHAR szWndClass[] = TEXT("SnfFrame");
const TCHAR szHelpPath[] = TEXT("http://code.google.com/p/zqqcode/wiki/Help");
const TCHAR szIssuePath[] = TEXT("http://code.google.com/p/zqqcode/issues/list");

int track(HWND hwnd);
void load(HWND hwnd, char *filename);

int APIENTRY 
WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
		 LPSTR lpCmdLine, int nShowCmd)
{
	MSG msg;

	g_hInstance = hInstance;
	if(!RegWndClass(hInstance))
	{
		MessageBox( NULL, TEXT("Cannot Register Window Class"), 
					szAppName, MB_OK | MB_ICONERROR);
		return 1;
	}
	if(InitWindow(hInstance, nShowCmd) == NULL)
	{
		MessageBox( NULL, TEXT("Cannot create window"),
					szAppName, MB_OK | MB_ICONERROR);
		return 1;
	}
	g_hNS = LoadCursor(NULL, MAKEINTRESOURCE(IDC_SIZENS));
	g_hWE = LoadCursor(NULL, MAKEINTRESOURCE(IDC_SIZEWE));
	while(GetMessage(&msg, 0, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return 0;
}

BOOL 
RegWndClass (HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = (WNDPROC)WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = (HICON)LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SNF));
    wcex.hCursor = (HCURSOR)LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW));
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName = MAKEINTRESOURCE(IDR_MAINMENU);
    wcex.lpszClassName = szWndClass;
	wcex.hIconSm = wcex.hIcon;
	g_hArrow = wcex.hCursor;
	return RegisterClassEx(&wcex);
}

HWND
InitWindow (HINSTANCE hInstance, int nShowCmd)
{
	HWND hwnd = CreateWindowEx (WS_EX_APPWINDOW | WS_EX_WINDOWEDGE | WS_EX_LEFT, szWndClass,
							szAppName, WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
							CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 
							CW_USEDEFAULT, NULL, NULL, hInstance, NULL);
	if(!hwnd)
		return NULL;
	ShowWindow(hwnd, SW_SHOWMAXIMIZED);
	UpdateWindow(hwnd);
	return hwnd;
}


int status;
sockaddr_in addr;
HWND hStatusBar;

LRESULT CALLBACK
WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static int cxClient, cyClient, cyStatusBar;
	static int flag;
	static HMENU hMenu;
	static POINT ptOld;
	POINT	ptNew;
	int aWidths[3], okay;
	static UINT uDir;
	RECT rc;
	WSADATA wd;
	TCHAR szError[128];
	LPNMHDR lpnmh;
	LPNMLISTVIEW lplv;
	LVITEM lvitem;
	switch(msg)
	{
	case WM_CREATE:
		InitCommonControls();
		hStatusBar = CreateWindowEx(0, STATUSCLASSNAME, TEXT(""), WS_CHILD | WS_VISIBLE,
									0, 0, 0, 0,
									hwnd, NULL, g_hInstance, 0);
		aWidths[0] = 360;
		aWidths[1] = 480;
		aWidths[2] = -1;
		SendMessage(hStatusBar, SB_SETPARTS, 3, (LPARAM)aWidths);
		SendMessage(hStatusBar, SB_SETTEXT, 0, (LPARAM)TEXT("Ready"));
		GetWindowRect(hStatusBar, &rc);
		cyStatusBar = rc.bottom - rc.top;
		flag = 0;
		status = S_NOTREADY;
		WSAStartup(0x202, &wd);
		addr.sin_family = AF_INET;
		addr.sin_port = htons(204);
		hMenu = GetMenu(hwnd);
		EnableMenuItem(hMenu, ID_CAPTURE_START, MF_GRAYED | MF_BYCOMMAND);
		EnableMenuItem(hMenu, ID_CAPTURE_STOP, MF_GRAYED | MF_BYCOMMAND);
		index = (unsigned int *)malloc(BUFFERSIZE+(sizeof(unsigned int)*INDEXCOUNT*2));
		if(index == NULL)
		{
			lstrcpy(szError, TEXT("Cannot allocate enough memory!"));
			MessageBox(hwnd, szError, szAppName, MB_OK | MB_ICONERROR);
			return -1;
		}
		offset = index + INDEXCOUNT;
		*offset = 0;
		buffer = (char *)(index + INDEXCOUNT*2);
		InitOFN(&g_ofn, hwnd);
		GetModuleFileName(NULL, g_szTempW, MAX_PATH);
		SetTempFile(g_szTempW);
		WideCharToMultiByte(CP_ACP, 0, g_szTempW, -1, g_szTempA, MAX_PATH,0, 0);
		return 0;

	case WM_SIZE:
		SendMessage(hStatusBar, WM_SIZE, wParam,  lParam);
		cxClient = LOWORD(lParam);
		cyClient = HIWORD(lParam)-cyStatusBar-3;
		AdjustWnd(cxClient, cyClient);
		return 0;

	case WM_GETMINMAXINFO:
		{
			MINMAXINFO *p = (MINMAXINFO *)lParam;
			p->ptMinTrackSize.x = 720;
			p->ptMinTrackSize.y = 450;
		}
		return 0;

	case WM_MOUSEMOVE:
		if(!flag)
		{
			uDir = HitMouse(lParam);
			if(uDir == MD_NS)
				SetCursor(g_hNS);
			else if(uDir == MD_WE)
				SetCursor(g_hWE);
			else
				SetCursor(g_hArrow);
		}
		else
		{
			GetCursorPos(&ptNew);
			ResizeWnd(cxClient, cyClient, ptNew.x - ptOld.x,  
				ptNew.y - ptOld.y, uDir);
			ptOld.x = ptNew.x;
			ptOld.y = ptNew.y;
		}
		return 0;

	case WM_LBUTTONDOWN:
		flag = 1;
		GetCursorPos(&ptOld);
		if(uDir == MD_NS)
			SetCursor(g_hNS);
		else if(uDir == MD_WE)
			SetCursor(g_hWE);
		else SetCursor(g_hArrow);
		SetCapture(hwnd);
		return 0;

	case WM_LBUTTONUP:
		flag = 0;
		ReleaseCapture();
		uDir = MD_NONE;
		SetCursor(g_hArrow);
		return 0;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDM_FILE_OPEN:
			okay = 1;
			if(status == S_RUNNING || status == S_PAUSED)
			{
				if(IDYES == MessageBox(hwnd,
					TEXT("A running monitor is not stopped.\r\nDo you want to stop it first?"),
					szAppName, MB_YESNO | MB_ICONQUESTION))
				{
					SendMessage(hwnd, WM_COMMAND, ID_CAPTURE_STOP, 0);
				}
				else okay = 0;
			}
			if(okay && ShowOpenDlg(&g_ofn, g_szFileName, g_szTitleName))
			{
				TempFileTrans(g_szFileName, g_szTempW);
				SendMessage(hStatusBar, SB_SETTEXT, 0, (LPARAM)TEXT("Loading file..."));
				SetMinMaxValue(200, 150, 200, 100);
				if(!DoInitialize(g_hInstance, hwnd, cxClient, cyClient, cxClient>>1, (UINT)(cyClient * 0.6)))
					Clear();
				load(hwnd, g_szTempA);
				SendMessage(hStatusBar, SB_SETTEXT, 0, (LPARAM)TEXT("File loaded."));
			}
			break;

		case IDM_FILE_SAVE:
			okay = 1;
			if(status == S_RUNNING || status == S_PAUSED)
			{
				if(IDYES == MessageBox(hwnd, 
					TEXT("A running monitor is not stopped.\r\nDo you want to stop it first?"),
					szAppName, MB_YESNO | MB_ICONQUESTION))
				{
					SendMessage(hwnd, WM_COMMAND, ID_CAPTURE_STOP, 0);
				}
				else okay = 0;
			}
			if(okay && ShowSaveDlg(&g_ofn, g_szFileName, g_szTitleName))
			{
				TempFileTrans(g_szTempW, g_szFileName);
				GetLastError();
				SendMessage(hStatusBar, SB_SETTEXT, 0, (LPARAM)TEXT("File saved."));
			}
			break;

		case IDM_FILE_EXIT:
			SendMessage(hwnd, WM_CLOSE, 0, 0);
			break;

		case ID_CAPTURE_OPTIONS:
			status = S_READY;
			if(TRUE == PopupOptionDlg(g_hInstance, hwnd))
				SendMessage(hwnd, WM_COMMAND, ID_CAPTURE_START, 0);
			if(status != S_NOTREADY)
			{
				EnableMenuItem(hMenu, ID_CAPTURE_START, MF_ENABLED | MF_BYCOMMAND);
				EnableMenuItem(hMenu, ID_CAPTURE_STOP, MF_ENABLED | MF_BYCOMMAND);
			}
			break;

		case ID_CAPTURE_CLEARDATA:
			Clear();
			break;

		case ID_CAPTURE_START:
			if(status == S_READY)
			{
				SetMinMaxValue(200, 150, 200, 100);
				if(!DoInitialize(g_hInstance, hwnd, cxClient, cyClient, cxClient>>1, (UINT)(cyClient * 0.6)))
					Clear();
				if(track(hwnd))
				{	
					status = S_NOTREADY;
					SendMessage(hStatusBar, SB_SETTEXT, 0, (LPARAM)TEXT("Errors exist."));
				}
				else
				{
					status = S_RUNNING;
					SendMessage(hStatusBar, SB_SETTEXT, 0, (LPARAM)TEXT("Tracking"));
				}
			}
			else if(status == S_RUNNING)
			{
				pausemonitor();
				status = S_PAUSED;
				SendMessage(hStatusBar, SB_SETTEXT, 0, (LPARAM)TEXT("Paused"));
			}
			else if(status == S_PAUSED)
			{
				resumemonitor();
				status = S_RUNNING;
				SendMessage(hStatusBar, SB_SETTEXT, 0, (LPARAM)TEXT("Tracking"));
			}
			else if(status == S_STOPPED)
			{
				ic = bs = 0;
				status = S_READY;
				SendMessage(hwnd, WM_COMMAND, ID_CAPTURE_START, 0);
			}
			if(status != S_NOTREADY)
				EnableMenuItem(hMenu, ID_CAPTURE_OPTIONS, MF_GRAYED | MF_BYCOMMAND);
			else
			{
				EnableMenuItem(hMenu, ID_CAPTURE_OPTIONS, MF_ENABLED | MF_BYCOMMAND);
				EnableMenuItem(hMenu, ID_CAPTURE_START, MF_GRAYED | MF_BYCOMMAND);
				EnableMenuItem(hMenu, ID_CAPTURE_STOP, MF_GRAYED | MF_BYCOMMAND);
			}
			break;

		case ID_CAPTURE_STOP:
			if(status == S_RUNNING || status == S_PAUSED)
			{
				status = S_STOPPED;
				stopmonitor();
				SendMessage(hStatusBar, SB_SETTEXT, 0, (LPARAM)TEXT("Finished"));
				writeblock(buffer, bs, index, ic);
				finish();
				EnableMenuItem(hMenu, ID_CAPTURE_OPTIONS, MF_ENABLED | MF_BYCOMMAND);
			}
			break;

		case ID_HELP_SNF204HELP:
			StartBrowser(szHelpPath);
			break;

		case ID_HELP_REPORTBUG:
			StartBrowser(szIssuePath);
			return 0;

		case ID_HELP_ABOUT:
			PopupAboutDlg(g_hInstance, hwnd);
			break;
		}
		return 0;

	case WM_NOTIFY:
		{
			lpnmh = (LPNMHDR)lParam;
			if(lpnmh->code == LVN_ITEMCHANGED && lpnmh->idFrom == LV_CTRLID)
			{
				lplv = (LPNMLISTVIEW)lParam;
				if(lplv->iItem >= 0)
				{
					lvitem.mask = LVIF_STATE;
					lvitem.iItem = lplv->iItem;
					lvitem.stateMask = LVIS_SELECTED;
					ListView_GetItem(GetHwnd(HWNDLV), &lvitem);
					if(lvitem.state == LVIS_SELECTED)
					{
						SetSelect(buffer+(*(offset+lvitem.iItem)), *(index+lvitem.iItem));
					}
				}
			}
		}
		return 0;

	case WM_CLOSE:
		DestroyWindow(hwnd);
		return 0;

	case WM_DESTROY:
		SendMessage(hwnd, WM_COMMAND, ID_CAPTURE_STOP, 0);
		while(!stopdone())
			Sleep(100);
		free(index);
		Destroy();
		WSACleanup();
		DeleteFile(g_szTempW);
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

BOOL
StartBrowser(const TCHAR *path)
{
	TCHAR szBrowserName[MAX_PATH], szTemp[MAX_PATH];
	HKEY hKey;
	DWORD dwcb;
	PROCESS_INFORMATION pi;
	STARTUPINFO si;
	TCHAR *p;

	if( RegOpenKey(HKEY_CLASSES_ROOT, TEXT("HTTP\\shell\\open\\command"),
					 &hKey) != ERROR_SUCCESS)
	{
		return FALSE;
	}
	dwcb = MAX_PATH;
	RegGetValue(hKey, NULL, NULL, REG_SZ, NULL, (LPVOID)szBrowserName, &dwcb);
	RegCloseKey(hKey);
	p = (TCHAR *)szBrowserName;
	while(*p)
	{
		if(*p == '%' && *(p+1) == '1')
		{
			lstrcpy(szTemp, p+2);
			lstrcpy(p, path);
			p += lstrlen(path);
			lstrcpy(p, szTemp);
			break;
		}
		++p;
	}
	memset(&si, 0, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);
	if(CreateProcess(NULL, szBrowserName, 0, 0, 0, 0, 0, 0, &si, &pi) == 0)
	{
		dwcb = GetLastError();
		return FALSE;
	}
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	return TRUE;
}

unsigned long 
OnReceiveData (char *buf, int size, void *arg1, void *arg2)
{
	unsigned long retptr;
	unsigned int tc;
	TCHAR szText[64];

	if(buf!=0)
	{
		tc = AddRecord(buf, size);
		wsprintf(szText,  TEXT("Package: %d"), tc);
		SendMessage(hStatusBar, SB_SETTEXT, 2, (LPARAM)szText);
		*(index+ic) = size;
		bs += size;
		++ic;
		*(offset+ic) = bs;
		if(BUFFERSIZE - bs < 2048 || INDEXCOUNT - ic < 2)
		{
			writeblock(buffer, bs, index, ic);
			retptr = (unsigned long)buffer;
			total += bs;
			ic = 0;
			bs = 0;
		}
		else
			retptr = (unsigned long)(buf+size);
	}
	return retptr;
}

int track(HWND hwnd)
{
	TCHAR szError[128];
	unsigned long err;
	HANDLE hThread;
		
	addr.sin_addr.S_un.S_addr = GetIpAddr();
	hThread = (HANDLE)startmonitor((SOCKADDR *)&addr, sizeof(addr),
		OnReceiveData, 0, 0, &err, buffer);
	if(hThread == NULL)
	{
		wsprintf(szError, TEXT("Error to start, error code: 0x%x"), err);
		MessageBox(hwnd, szError, szAppName, MB_OK | MB_ICONERROR);
		return 1;
	}
	setfile(g_szTempA, FM_WRITE);
	CloseHandle(hThread);
	return 0;
}

void load(HWND hwnd, char *filename)
{
	unsigned long count, i, j, tc;
	char *p;
	TCHAR szText[128];

	setfile(filename, FM_READ);
	count = getblockcount();
	for(i=0; i< count; ++i)
	{
		readblock(buffer, &bs, index, &ic);
		p = buffer;
		for(j = 0; j < ic; ++j)
		{
			tc = AddRecord(p, *(index+j));
			p += *(index+j);
			wsprintf(szText,  TEXT("Package: %d"), tc);
			SendMessage(hStatusBar, SB_SETTEXT, 2, (LPARAM)szText);
		}
	}
	finish();
}