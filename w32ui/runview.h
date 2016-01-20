/*
* runview.h: Pack the list view management.
*/

#ifndef RUNVIEW_H
#define RUNVIEW_H

#include <windows.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#define LV_CTRLID	(99)

#define HWNDLV	(0)
#define HWNDTV	(1)
#define HWNDBV	(2)

#define MD_NONE	(0)
#define MD_NS	(1)
#define MD_WE	(2)

BOOL 
DoInitialize (HINSTANCE hInstance, HWND hWndParent,
			  UINT cxClient, UINT cyClient, UINT cxWidth, UINT cyHeight);

VOID
SetMinMaxValue(UINT cxMin, UINT cyMin,
			   UINT cxMax, UINT cyMax);

HWND
GetHwnd (int type);

VOID
Destroy ();

VOID
Clear ();

UINT
AddRecord (char * data, int size);

UINT
SetSelect (char * data, int size);

VOID
AdjustWnd (UINT cxClient, UINT cyClient);

UINT 
HitMouse (LPARAM lParam);

VOID
ResizeWnd (UINT cxClient, UINT cyClient, int cxDelta, int cyDelta, UINT uDir); 

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* RUNVIEW_H */