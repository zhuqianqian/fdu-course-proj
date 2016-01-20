/*
* option.h: Pops up the option dialog
*/

#ifndef OPTION_H
#define OPTION_H

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

BOOL 
PopupOptionDlg (HINSTANCE hInstance, HWND hParentWnd);

ULONG
GetIpAddr ();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* OPTION_H */