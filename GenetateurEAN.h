#pragma once
#define _WIN32_IE 0x501
#define IDC_IMAGE        200
#define IDC_DECODE       201
#define IDC_EDIT         202
#define IDC_EDITRES      203
#define IDC_BINARY       204
#define IDC_CHECKDIGIT   205
#define IDC_PRINT        206
#define IDC_COPY         207
#define IDC_HZOOMUP      208
#define IDC_HZOOMDOWN    209
#define IDC_VZOOMUP      210
#define IDC_VZOOMDOWN    211
#define IDC_FILECLOSE    212
#define IDC_FILEHELP     213
#define IDC_ABOUTHELP    214
#define IDI_APPMAIN    100
#define IDD_PRINTSETUP 101
#define IDC_EDIT_LEFT   1000
#define IDC_EDIT_TOP    1001
#define IDC_EDIT_WIDTH  1002
#define IDC_EDIT_HEIGHT 1003
#define IDC_UD_LEFT     1004
#define IDC_UD_TOP      1005
#define IDC_UD_WIDTH    1006
#define IDC_UD_HEIGHT   1007
#define  TEXT_BTN1   "Binaire"
#define  TEXT_BTN2   "Caractère"
#define IDM_TIMER1 0x01000
#define GWL_WNDPROC         (-4)
#define GWL_HINSTANCE       (-6)
#define GWL_HWNDPARENT      (-8)
#define GWL_STYLE           (-16)
#define GWL_EXSTYLE         (-20)
#define GWL_USERDATA        (-21)
#define GWL_ID              (-12)

#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <wingdi.h>
#include "resource.h"
#pragma comment (lib,"comctl32")

void SetResultCode(HWND hwnd);
BOOL PreparePrint(HDC hdc, POINTFLOAT ptf, LPRECT lprc);
int GetAppLastError();
const char* ErrorString(int error);
const char* GetCodeString();
void SetBits(char **lppData, int * lpPos, WORD wCode, int len);
int Decode(const char * pszCode);
int GetApproximateStep(int width);
int GetMinimalHeight(HDC hdc, int step);
int CheckDigit(const char * pszCode, BOOL bComplet);
void Draw(HDC hdc, POINT pt, int height, int step);
int SetIndex(char car);
int GetApproximateWidth(int step);
int   Decode(const char*);
const char* GetCodeString();
int GetAppLastError();
const char* ErrorString(int);
int MsgBox(HINSTANCE hInst,HWND hDlg,char* lpszText,char* lpszCaption, DWORD dwStyle,int lpszIcon);
PBITMAPINFO CreateBitmapInfoStruct(HWND hwnd, HBITMAP hBmp);
void CreateBMPFile(HWND hwnd, LPTSTR pszFile, PBITMAPINFO pbi,HBITMAP hBMP, HDC hDC);
void errhandler(LPCTSTR Texte,HWND hwnd);
void SetBits(char**, int*, WORD, int);
LRESULT CALLBACK WindowProcedure (HWND, UINT, WPARAM, LPARAM);
void SetResultCode(HWND hwnd);
BOOL PreparePrint(HDC hdc, POINTFLOAT ptf, LPRECT lprc);
int ModeTelecommande(HWND hwnd);

