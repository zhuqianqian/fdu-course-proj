/*
* misc.h: Miscellaneous of the project, including
*	File Open/Save dialog, About dialog.
*/

#ifndef MISC_H
#define MISC_H

#include <windows.h>
#include <commctrl.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void 
InitOFN (OPENFILENAME *pofn, HWND hwnd);

BOOL
ShowSaveDlg(OPENFILENAME *pofn, LPWSTR szFileName, LPWSTR szTitleName);

BOOL
ShowOpenDlg(OPENFILENAME *pofn, LPWSTR szFileName, LPWSTR szTitleName);

BOOL
PopupAboutDlg(HINSTANCE hInstance, HWND hwnd);

void
SetTempFile(LPWSTR szPath);

BOOL
TempFileTrans(LPWSTR file1, LPWSTR file2);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MISC_H */