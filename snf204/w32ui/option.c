#include "../resources/resource.h"
#include "option.h"
#include "../include/proto.h"
#include <iphlpapi.h>


ipaddr _ip[32];
unsigned long _selection;

void 
FillInterfaceList(HWND hCombo)
{
	PIP_ADAPTER_INFO pAdapterInfo;
	PIP_ADAPTER_INFO pAdapter;
	u_long ip_ret;
	u_long ulOutBufLen;
	u_long i = 0;

	pAdapterInfo = (IP_ADAPTER_INFO *) malloc( sizeof(IP_ADAPTER_INFO) );
	ulOutBufLen = sizeof(IP_ADAPTER_INFO);

	if (GetAdaptersInfo( pAdapterInfo, &ulOutBufLen) != ERROR_SUCCESS) {
		free (pAdapterInfo);
		pAdapterInfo = (IP_ADAPTER_INFO *) malloc (ulOutBufLen);
	}
	if ((ip_ret = GetAdaptersInfo( pAdapterInfo, &ulOutBufLen)) == NO_ERROR) {
		pAdapter = pAdapterInfo;
		while (pAdapter) {
			SendMessageA(hCombo, CB_ADDSTRING, 0, (LPARAM)(pAdapter->Description));
			_ip[i].un.ip_l = inet_addr(pAdapter->IpAddressList.IpAddress.String);
			pAdapter = pAdapter->Next;
			++i;
		}
	}
	free(pAdapterInfo);
	SendMessage(hCombo, CB_SETCURSEL, (WPARAM)_selection, 0);
}

BOOL CALLBACK
OptionDlgProc (HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND hwnd;
	TCHAR szIpShow[20];
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
		FillInterfaceList(GetDlgItem(hDlg, IDC_INTERFACE));
		wsprintf(szIpShow, TEXT("%d.%d.%d.%d"), _ip[_selection].un.ip_b.b1, _ip[_selection].un.ip_b.b2,
			_ip[_selection].un.ip_b.b3, _ip[_selection].un.ip_b.b4);
		SetDlgItemText(hDlg, IDC_IPADDR, szIpShow);
		return TRUE;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_INTERFACE:
			if(HIWORD(wParam) == CBN_SELCHANGE)
			{
				_selection = (ULONG)SendMessage(GetDlgItem(hDlg, IDC_INTERFACE),
					CB_GETCURSEL, 0, 0);
				wsprintf(szIpShow, TEXT("%d.%d.%d.%d"), _ip[_selection].un.ip_b.b1, _ip[_selection].un.ip_b.b2,
					_ip[_selection].un.ip_b.b3, _ip[_selection].un.ip_b.b4);
				SetDlgItemText(hDlg, IDC_IPADDR, szIpShow);
			}
			break;

		case IDOK:
			EndDialog(hDlg, TRUE);
			break;

		case IDCANCEL:
			EndDialog(hDlg, FALSE);
			break;
		}
		return FALSE;
	}
	return FALSE;
}

ULONG 
GetIpAddr ()
{
	return _ip[_selection].un.ip_l;
}

BOOL 
PopupOptionDlg(HINSTANCE hInstance, HWND hParentWnd)
{
	BOOL val;
	val = DialogBox(hInstance, MAKEINTRESOURCE(IDD_OPTION), hParentWnd, OptionDlgProc);
	return val;
}