/*
* snfwgui.h: Declarations and definitions for GUI interface.
*	Platform: Windows
*/

#ifndef SNFWGUI_H
#define SNFWGUI_H

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/*
* InitInstance and RegWndClass just do a pack for normal 
* register window class and create window procedure.
*/
HWND 
InitWindow (HINSTANCE hInstance, int nShowCmd);

BOOL
RegWndClass (HINSTANCE hInstance);

/* Callback procedure declarations */
LRESULT CALLBACK
WndProc(HWND, UINT, WPARAM, LPARAM);

/* Start the system browser to retrive an url */
BOOL 
StartBrowser(const TCHAR *path);

unsigned long 
OnReceiveData(char *buf, int size, void *arg1, void *arg2);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* SNFWGUI_H */