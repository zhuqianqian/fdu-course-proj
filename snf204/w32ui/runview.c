#if defined (_X86_)
#define LITTLE_ENDIAN
#elif defined (POWERPC) || defined (MOTOROLA)
#define BIG_ENDIAN
#else
#define LITTLE_ENDIAN
#endif /* _X86_ */

#include "runview.h"
#include "protohlp.h"
#include <commctrl.h>
#include "../include/proto.h"

HWND _hwnd[3] = {0};
HFONT	_hFont;
unsigned int _count = 0;
POINT	_ptmin, _ptmax;
POINT	_minval, _maxval;

#define COLUMNS		(6)
#define MAX_INDEX	(2)

HFONT GetFont(LPWSTR szFaceName)
{
	static LOGFONT lf;
	GetObject(GetStockObject(ANSI_FIXED_FONT), sizeof(lf), &lf);
	lf.lfWeight = FW_NORMAL;
	lf.lfHeight = 16;
	lf.lfItalic = 0;
//	lf.lfWeight = 10;
	lstrcpy(lf.lfFaceName, szFaceName);
	return CreateFont(lf.lfHeight, lf.lfWidth, lf.lfEscapement, lf.lfOrientation,
		lf.lfWeight, lf.lfItalic, lf.lfUnderline, lf.lfStrikeOut, lf.lfCharSet,
		lf.lfOutPrecision, lf.lfClipPrecision, lf.lfQuality, FIXED_PITCH, lf.lfFaceName);
}

BOOL
DoInitialize (HINSTANCE hInstance, HWND hWndParent,
			  UINT cxClient, UINT cyClient, UINT cxWidth, UINT cyHeight)
{
	LVCOLUMN lvcol;
	TCHAR szHeader[][20] = {TEXT("#"), TEXT("Source"), TEXT("Desitination"), 
		TEXT("Protocol"), TEXT("Size"), TEXT("Information")};
	int cxArray[] = {60, 240, 240, 80, 60, 360};
	int i;

	if(_hwnd[0] != NULL)
		return FALSE;
	lvcol.fmt = LVCFMT_LEFT;
	lvcol.mask = LVCF_WIDTH | LVCF_FMT | LVCF_TEXT;
	_ptmin.y = cyHeight;
	_ptmin.x = cxWidth-2;
	_ptmax.y = cyHeight+4;
	_ptmax.x = cxWidth+2;	
	_hwnd[0] = CreateWindowEx(0, WC_LISTVIEW,TEXT(""), 
		WS_CHILD | WS_VISIBLE | LVS_REPORT | WS_BORDER | LVS_SINGLESEL,
		0, 0, cxClient, cyHeight,
		hWndParent, (HMENU)LV_CTRLID, hInstance, NULL);
	if(_hwnd[0] == NULL)
		return FALSE;
	SendMessage(_hwnd[0], LVM_SETEXTENDEDLISTVIEWSTYLE, 
		LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);
	_hwnd[1] = CreateWindowEx(0, WC_TREEVIEW, TEXT(""), 
		WS_CHILD | WS_VISIBLE | WS_BORDER,
		0, _ptmax.y, _ptmin.x, cyClient-cyHeight,
		hWndParent, 0, hInstance, 0);
	if(_hwnd[1] == 0)
	{
		DestroyWindow(_hwnd[0]);
		return FALSE;
	}
	_hFont = GetFont(TEXT("Consolas"));
	_hwnd[2] = CreateWindowEx(0, TEXT("EDIT"), TEXT(""),
		WS_BORDER | WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL,
		_ptmax.x, _ptmax.y, cxClient-cxWidth-2, cyClient-cyHeight,
		hWndParent, 0, hInstance, 0);
	if(_hwnd[2] == 0)
	{
		DestroyWindow(_hwnd[1]);
		DestroyWindow(_hwnd[0]);
		return FALSE;
	}
	SendMessage(_hwnd[2], WM_SETFONT, (WPARAM)_hFont, TRUE);
	for(i = 0; i < COLUMNS;++i)
	{
		lvcol.pszText = szHeader[i];
		lvcol.cx = cxArray[i];
		lvcol.iSubItem = i;
		SendMessage(_hwnd[0], LVM_INSERTCOLUMN, i, (LPARAM)&lvcol);
	}

	return TRUE;
}

VOID
SetMinMaxValue(UINT cxMin, UINT cyMin,
			   UINT cxMax, UINT cyMax)
{
	_minval.x = cxMin;
	_minval.y = cyMin;
	_maxval.x = cxMax;
	_maxval.y = cyMax;
}

VOID
Clear()
{
	SendMessage(_hwnd[0], LVM_DELETEALLITEMS, 0, 0);
	_count = 0;
}

void 
Destroy()
{
	DestroyWindow(_hwnd[0]);
	DestroyWindow(_hwnd[1]);
	DestroyWindow(_hwnd[2]);
	DeleteObject(_hFont);
	_hFont = NULL;
	_hwnd[0] = 0;
}

HWND 
GetHwnd(int type)
{
	if(type < 0 || type > MAX_INDEX)
		return NULL;
	return _hwnd[type];
}

UINT 
AddRecord(char *data, int size)
{
	ULONG ret;
	iphdr_ptr ip;
	LVITEM lvi;
	TCHAR szText[128];
	TCHAR szTemp[8];

	ip = (iphdr_ptr)data;
	lvi.mask = LVIF_TEXT;
	lvi.pszText = szText;
	lvi.iItem = _count;

	lvi.iSubItem = 0;
	wsprintf(szText, TEXT("%d"), _count+1); 
	ret = SendMessage(_hwnd[0], LVM_INSERTITEM, 0, (LPARAM)&lvi);	
	lvi.iItem = ret;
	lvi.iSubItem = 1;
	wsprintf(szText, TEXT("%d.%d.%d.%d"), ip->srcip.un.ip_b.b1,
		ip->srcip.un.ip_b.b2, ip->srcip.un.ip_b.b3, ip->srcip.un.ip_b.b4); 
	SendMessage(_hwnd[0], LVM_SETITEM, 0, (LPARAM)&lvi);	
	ret = GetLastError();
	lvi.iSubItem = 2;
	wsprintf(szText, TEXT("%d.%d.%d.%d"), ip->destip.un.ip_b.b1,
		ip->destip.un.ip_b.b2, ip->destip.un.ip_b.b3, ip->destip.un.ip_b.b4); 
	SendMessage(_hwnd[0], LVM_SETITEM, 0, (LPARAM)&lvi);	
	switch(ip->protocal)
	{
	case PF_ICMP:
		wsprintf(szTemp, TEXT("%s"), TEXT("ICMP"));
		geticmpinfo((data+((ip->hdrlen)<<2)), szText);
		lvi.iSubItem = 5;
		SendMessage(_hwnd[0], LVM_SETITEM, 0, (LPARAM)&lvi);	
		break;

	case PF_TCP:
		wsprintf(szTemp, TEXT("%s"), TEXT("TCP"));
		gettcpinfo((data+((ip->hdrlen)<<2)), szText);
		lvi.iSubItem = 5;
		SendMessage(_hwnd[0], LVM_SETITEM, 0, (LPARAM)&lvi);	
		break;

	case PF_UDP:
		wsprintf(szTemp, TEXT("%s"), TEXT("UDP"));
		getudpinfo((data+((ip->hdrlen)<<2)), szText);
		lvi.iSubItem = 5;
		SendMessage(_hwnd[0], LVM_SETITEM, 0, (LPARAM)&lvi);	
		break;
	default:
		wsprintf(szTemp, TEXT("%s"), TEXT("unknown"));
		break;
	}
	lstrcpy(szText, szTemp);
	lvi.iSubItem = 3;
	SendMessage(_hwnd[0], LVM_SETITEM, 0, (LPARAM)&lvi);	
	lvi.iSubItem = 4;
	wsprintf(szText, TEXT("%d"), size); 
	SendMessage(_hwnd[0], LVM_SETITEM, 0, (LPARAM)&lvi);

	++_count;
	return _count;
}

VOID
AdjustWnd (UINT cxClient, UINT cyClient)
{
	if(_ptmin.y + _minval.y > cyClient)
	{
		_ptmin.y = cyClient - _minval.y;
		_ptmax.y = _ptmin.y + 4;
	}
	if(_ptmin.y < _maxval.y)
	{
		_ptmin.y = _maxval.y;
		_ptmax.y = _ptmin.y + 4;
	}
	if(_ptmin.x + _minval.x > cxClient)
	{
		_ptmin.x = cxClient - _minval.x + 2;
		_ptmax.x = _ptmin.x + 4;
	}
	if(_ptmin.x < _maxval.x)
	{
		_ptmin.x = _maxval.x;
		_ptmax.x = _ptmin.x + 4;
	}
	SetWindowPos(_hwnd[0], NULL,
		0, 0, cxClient, _ptmin.y, SWP_NOZORDER);
	SetWindowPos(_hwnd[1], NULL,
		0, _ptmax.y, _ptmin.x, cyClient-_ptmin.y, SWP_NOZORDER);
	SetWindowPos(_hwnd[2], NULL, 
		_ptmax.x, _ptmax.y, cxClient-_ptmin.x, cyClient-_ptmin.y,
		SWP_NOZORDER);
}

UINT 
HitMouse (LPARAM lParam)
{
	UINT cx, cy;
	cx = LOWORD(lParam);
	cy = HIWORD(lParam);

	if(cy >= _ptmin.y && cy <= _ptmax.y)
	{
		return MD_NS;
	}
	else if(cy > _ptmax.y && cx >= _ptmin.x && cx <= _ptmax.x)
	{
		return MD_WE;
	}
	return MD_NONE;
}

VOID
ResizeWnd(UINT cxClient, UINT cyClient, int cxDelta, int cyDelta, UINT uDir)
{
	if(uDir == MD_NS)
	{
		_ptmin.y = _ptmin.y + cyDelta;
		_ptmax.y = _ptmin.y + 4;
	}
	else if(uDir == MD_WE)
	{
		_ptmin.x = _ptmin.x + cxDelta;
		_ptmax.x = _ptmin.x + 4;
	}
	AdjustWnd(cxClient, cyClient);
}


UINT
SetSelect(char *data, int size)
{
	TCHAR szBinary[2048];
	TCHAR szTemp[32];
	unsigned int size8;
	unsigned int i;
	unsigned char *p;
	p = (unsigned char *)data;
	i = 0;
	size8 = ((unsigned)size & 0xfffffff8);
	szBinary[0] = 0;
	while (i<size8)
	{
		wsprintf(szTemp, TEXT("%02x %02x %02x %02x    %02x %02x %02x %02x\r\n"),
			*(p+i), *(p+i+1), *(p+i+2), *(p+i+3),
			*(p+i+4), *(p+i+5), *(p+i+6), *(p+i+7));
		lstrcat(szBinary, szTemp);
		i += 8;
	}
	for(; i < size; ++i)
	{
		wsprintf(szTemp, TEXT("%02x "), *(p+i));
		lstrcat(szBinary, szTemp);
		if((i & 0x00000003)==3)
			lstrcat(szBinary, TEXT("   "));
	}
	SetWindowText(_hwnd[2], szBinary);
	return 0;
}