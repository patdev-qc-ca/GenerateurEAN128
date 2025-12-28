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

#include <windows.h>
#include <commctrl.h>
#include <wingdi.h>
#pragma comment (lib,"comctl32")

class  CDlgPrintSetup{
public:
    CDlgPrintSetup(LPRECT);
    ~CDlgPrintSetup();    
    int Create(HWND);
    
private:
    static BOOL CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);
    BOOL MsgInitDialog(HWND, WPARAM, LPARAM);
    BOOL MsgCommand(HWND, WPARAM, LPARAM);
    BOOL MsgVScroll(HWND, WPARAM, LPARAM);
    BOOL MsgSysCommand(WPARAM);    
    void CmdOk(HWND);
    HWND       m_hDlg;
    LPRECT     m_pRect;
};


HBRUSH hBrushStatic;
WNDCLASSEX wincl;    
RECT rc, rect,m_rcPrint;
SYSTEMTIME st;
TCHAR buffer[MAX_PATH];
TCHAR szBuf[MAX_PATH];
TCHAR szApp[MAX_PATH];
HDC hdcScreen,hdcCompatible;
HBITMAP hbmScreen,bmp,hbmScaled;
BITMAP btmp;

const WORD cstTable128[] = {0x0a6d,0x0d2b,0x0b2b,0x0d95,0x0a6b,0x0d35,0x0b35,0x0a5b,0x0d2d,0x0b2d,0x0d4b,0x0b4b,0x0da5,0x0acb,0x0d65,0x0b65,0x0a9b,0x0d4d,0x0b4d,0x0acd,0x0d53,0x0b53,0x0da9,0x0ad3,0x0d69,0x0b69,0x0ab3,0x0d59,0x0b59,0x0ad9,0x0cab,0x09ab,0x0cd5,0x096b,0x0cb5,0x09b5,0x095b,0x0cad,0x09ad,0x096d,0x0927,0x0929,0x0949,0x0a49};
static const short cstMargin[4][2] = {0, 190, 0, 280, 10, 200, 2, 100};
char *jours[7] = {"dimanche", "lundi","mardi","mercredi","jeudi","vendredi","samedi"};
char *mois[12] = {"janvier", "février","mars", "avril", "mai", "juin","juillet","aout","septembre", "octobre", "novembre", "décembre"};
enum {ERR_NONE=0,ERR_VAR_CODE_NULL,ERR_CODE_INCOMPATIBLE,ERR_BAD_FORMAT_SIZE,ERR_SIZE_LESS,ERR_CODE_TRUNCATE    };

char m_szData[256];
int m_iError,m_nbBits,m_bitSize,m_step,m_height;

CDlgPrintSetup::CDlgPrintSetup(LPRECT p){    m_hDlg = NULL;    m_pRect = p;}
CDlgPrintSetup::~CDlgPrintSetup(){}
int CDlgPrintSetup::Create(HWND hwndParent){
	HINSTANCE hInst;
	hInst = (HINSTANCE)GetModuleHandle(NULL);
	return DialogBoxParam(hInst,MAKEINTRESOURCE(IDD_PRINTSETUP),hwndParent,
	DlgProc,(LPARAM)this);
}
BOOL CALLBACK CDlgPrintSetup::DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam){
    if(message==WM_INITDIALOG)        SetWindowLong(hDlg,GWL_USERDATA,lParam);    
    CDlgPrintSetup *pDlg = (CDlgPrintSetup*)GetWindowLong(hDlg,GWL_USERDATA);    
    if(!pDlg)       return FALSE;    
    switch(message)    {
        case WM_INITDIALOG:         return pDlg->MsgInitDialog(hDlg,wParam,lParam);
        case WM_COMMAND:            return pDlg->MsgCommand(hDlg,wParam,lParam);        
        case WM_VSCROLL:            return pDlg->MsgVScroll(hDlg,wParam,lParam); 
        case WM_SYSCOMMAND:         return pDlg->MsgSysCommand(wParam);
    }
    return FALSE;
}
BOOL CDlgPrintSetup::MsgInitDialog(HWND hDlg, WPARAM wParam, LPARAM lParam){    
    int i, j;
    m_hDlg = hDlg;        
    for(i = IDC_UD_LEFT, j=0; i<=IDC_UD_HEIGHT; i++, j++)    {
        SendDlgItemMessage(hDlg, i, UDM_SETBUDDY, (WPARAM)GetDlgItem(hDlg, i-4), 0);
        SendDlgItemMessage(hDlg, i-4, EM_LIMITTEXT, 3, 0);
        SendDlgItemMessage(hDlg, i, UDM_SETRANGE, 0, MAKELONG(cstMargin[j][1],cstMargin[j][0]));
    }
    m_pRect->left = max(cstMargin[0][0], min(cstMargin[0][1], m_pRect->left));
    m_pRect->top = max(cstMargin[1][0], min(cstMargin[1][1], m_pRect->top));
    m_pRect->right = max(cstMargin[2][0], min(cstMargin[2][1], m_pRect->right));
    m_pRect->bottom = max(cstMargin[3][0], min(cstMargin[3][1], m_pRect->bottom));
    SendDlgItemMessage(hDlg, IDC_UD_LEFT, UDM_SETPOS, 0, MAKELONG(m_pRect->left,0));
    SetDlgItemInt(hDlg, IDC_EDIT_LEFT, m_pRect->left, FALSE);
    SendDlgItemMessage(hDlg, IDC_UD_TOP, UDM_SETPOS, 0, MAKELONG(m_pRect->top,0));
    SetDlgItemInt(hDlg, IDC_EDIT_TOP, m_pRect->top, FALSE);
    SendDlgItemMessage(hDlg, IDC_UD_WIDTH, UDM_SETPOS, 0, MAKELONG(m_pRect->right,0));
    SetDlgItemInt(hDlg, IDC_EDIT_WIDTH, m_pRect->right, FALSE);
    SendDlgItemMessage(hDlg, IDC_UD_HEIGHT, UDM_SETPOS, 0, MAKELONG(m_pRect->bottom,0));
    SetDlgItemInt(hDlg, IDC_EDIT_HEIGHT, m_pRect->bottom, FALSE);
    return FALSE;
}
BOOL CDlgPrintSetup::MsgVScroll(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
    int id;
    
    id = GetWindowLong((HWND)lParam, GWL_ID);
    if((id<IDC_UD_LEFT)||(id>IDC_UD_HEIGHT))
        return FALSE;
    if(LOWORD(wParam)==SB_THUMBPOSITION)
        SetDlgItemInt(hDlg, id-4, HIWORD(wParam), FALSE);
        
    return FALSE;
}


BOOL CDlgPrintSetup::MsgCommand(HWND hDlg, WPARAM wParam, LPARAM lParam)
{    
    int val;
    switch(LOWORD(wParam))    {
        case IDC_EDIT_LEFT:		if(HIWORD(wParam)==EN_KILLFOCUS) {val = GetDlgItemInt(hDlg, LOWORD(wParam), NULL, TRUE);m_pRect->left = max(cstMargin[0][0], min(cstMargin[0][1], val));SendDlgItemMessage(hDlg, IDC_UD_LEFT, UDM_SETPOS, 0, MAKELONG(m_pRect->left,0));SetDlgItemInt(hDlg, IDC_EDIT_LEFT, m_pRect->left, FALSE);}  break;
        case IDC_EDIT_TOP:      if(HIWORD(wParam)==EN_KILLFOCUS) {val = GetDlgItemInt(hDlg, LOWORD(wParam), NULL, TRUE);m_pRect->top = max(cstMargin[1][0], min(cstMargin[1][1], val));SendDlgItemMessage(hDlg, IDC_UD_TOP, UDM_SETPOS, 0, MAKELONG(m_pRect->top,0));SetDlgItemInt(hDlg, IDC_EDIT_TOP, m_pRect->top, FALSE);}break;
        case IDC_EDIT_WIDTH:    if(HIWORD(wParam)==EN_KILLFOCUS) {val = GetDlgItemInt(hDlg, LOWORD(wParam), NULL, TRUE);m_pRect->right = max(cstMargin[2][0], min(cstMargin[2][1], val));SendDlgItemMessage(hDlg, IDC_UD_WIDTH, UDM_SETPOS, 0, MAKELONG(m_pRect->right,0));SetDlgItemInt(hDlg, IDC_EDIT_WIDTH, m_pRect->right, FALSE);}break;
        case IDC_EDIT_HEIGHT:	if(HIWORD(wParam)==EN_KILLFOCUS) {val = GetDlgItemInt(hDlg, LOWORD(wParam), NULL, TRUE);m_pRect->bottom = max(cstMargin[3][0], min(cstMargin[3][1], val));SendDlgItemMessage(hDlg, IDC_UD_HEIGHT, UDM_SETPOS, 0, MAKELONG(m_pRect->bottom,0));SetDlgItemInt(hDlg, IDC_EDIT_HEIGHT, m_pRect->bottom, FALSE);}break;
        case IDOK:            CmdOk(hDlg);            break;
        case IDCANCEL:            EndDialog(m_hDlg, 0);
	}                            
    return FALSE;
}
BOOL CDlgPrintSetup::MsgSysCommand(WPARAM wParam){ switch(LOWORD(wParam)){case SC_CLOSE:EndDialog(m_hDlg, 0);return TRUE;  }return FALSE;}
void CDlgPrintSetup::CmdOk(HWND hDlg){
    m_pRect->left = GetDlgItemInt(hDlg, IDC_EDIT_LEFT, NULL, TRUE);
    m_pRect->top = GetDlgItemInt(hDlg, IDC_EDIT_TOP, NULL, TRUE);
    m_pRect->right = GetDlgItemInt(hDlg, IDC_EDIT_WIDTH, NULL, TRUE);
    m_pRect->bottom = GetDlgItemInt(hDlg, IDC_EDIT_HEIGHT, NULL, TRUE);
    EndDialog(m_hDlg, 1);
}


void SetResultCode(HWND hDlg);
BOOL PreparePrint(HDC hdc, POINTFLOAT ptf, LPRECT lprc);
int GetAppLastError(){    return m_iError;}
const char* ErrorString(int error);
const char* GetCodeString(){    return m_szData;}
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
PBITMAPINFO CreateBitmapInfoStruct(HWND hDlg, HBITMAP hBmp);
void CreateBMPFile(HWND hDlg, LPTSTR pszFile, PBITMAPINFO pbi,HBITMAP hBMP, HDC hDC);
void errhandler(LPCTSTR Texte,HWND hDlg);
void SetBits(char**, int*, WORD, int);
LRESULT CALLBACK WindowProcedure (HWND, UINT, WPARAM, LPARAM);
void SetResultCode(HWND hDlg);
BOOL PreparePrint(HDC hdc, POINTFLOAT ptf, LPRECT lprc);
int ModeTelecommande(HWND hDlg);


char       szClassName[] = "CodeBarreWindowsApp";
char       g_szWndTitle[] = "Code Barre 1.83 ©Patrice Waechter-Ebling 2019-2020 | Code: EAN 128 | WinNT32";



int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszArgument, int nFunsterStil){
    HWND hDlg;
    MSG messages;
    InitCommonControls();
    wincl.hInstance = hInstance;
    wincl.lpszClassName = szClassName;
    wincl.lpfnWndProc = WindowProcedure;
    wincl.style = CS_DBLCLKS;
    wincl.cbSize = sizeof (WNDCLASSEX);
    wincl.hIcon = LoadIcon(wincl.hInstance, MAKEINTRESOURCE(IDI_APPMAIN));
    wincl.hIconSm = LoadIcon(wincl.hInstance, MAKEINTRESOURCE(IDI_APPMAIN));
    wincl.hCursor = LoadCursor (NULL, IDC_ARROW);
    wincl.lpszMenuName = NULL;
    wincl.cbClsExtra = 0;
    wincl.cbWndExtra = 0;
    wincl.hbrBackground = (HBRUSH) CreateSolidBrush(RGB(128,255,128));
    
    if(!RegisterClassEx (&wincl))  return 0;
	hDlg = CreateWindowEx (WS_EX_DLGMODALFRAME|WS_EX_CLIENTEDGE, szClassName, g_szWndTitle,WS_CAPTION|WS_VISIBLE|WS_SYSMENU,CW_USEDEFAULT, CW_USEDEFAULT, 640, 480,HWND_DESKTOP, NULL, hInstance, NULL);
	GetClientRect(hDlg, &rc);
	rc.bottom=rc.bottom-21;

	HWND hCtrl = CreateWindowEx(0, "button", " Code barre ",WS_CHILD|WS_VISIBLE|BS_GROUPBOX,5, 5, rc.right-10, 55, hDlg, (HMENU)0, wincl.hInstance, NULL);    
	SendMessage(hCtrl, WM_SETFONT, (WPARAM)GetStockObject(18), 1L);
	if(__argc>1){
		hCtrl = CreateWindowEx(512, "edit", __argv[1],WS_CHILD|WS_VISIBLE|ES_AUTOHSCROLL, 15, 25, rc.right-248, 22, hDlg, (HMENU)IDC_EDIT, wincl.hInstance, NULL);    
	// ModeTelecommande(hDlg);
	}else{
		hCtrl = CreateWindowEx(512, "edit", "",WS_CHILD|WS_VISIBLE|ES_AUTOHSCROLL, 15, 25, rc.right-248, 22, hDlg, (HMENU)IDC_EDIT, wincl.hInstance, NULL);    
	}
	SendMessage(hCtrl, WM_SETFONT, (WPARAM)GetStockObject(18), 1L);
	hCtrl = CreateWindowEx(0, "Button", "Générer code",WS_CHILD|WS_VISIBLE, rc.right-120, 25, 110, 22, hDlg, (HMENU)IDC_DECODE, wincl.hInstance, NULL);    
	SendMessage(hCtrl, WM_SETFONT, (WPARAM)GetStockObject(18), 1L);
	hCtrl = CreateWindowEx(0, "Button", "Checkdigit",WS_CHILD|WS_VISIBLE, rc.right-221, 25, 90, 22, hDlg, (HMENU)IDC_CHECKDIGIT, wincl.hInstance, NULL);    
	SendMessage(hCtrl, WM_SETFONT, (WPARAM)GetStockObject(18), 1L);
	hCtrl = CreateWindowEx(0, "button", " Code résultant ",WS_CHILD|WS_VISIBLE|BS_GROUPBOX, 5, 65, rc.right-10, 55, hDlg, NULL, wincl.hInstance, NULL);    
	SendMessage(hCtrl, WM_SETFONT, (WPARAM)GetStockObject(18), 1L);
	hCtrl = CreateWindowEx(512, "edit", "© Patrice Waechter-Ebling 2020",WS_CHILD|WS_VISIBLE|ES_READONLY|ES_AUTOHSCROLL, 15, 85, rc.right-137, 22, hDlg, (HMENU)IDC_EDITRES, wincl.hInstance, NULL);    
	SendMessage(hCtrl, WM_SETFONT, (WPARAM)GetStockObject(18), 1L);
	hCtrl = CreateWindowEx(0, "Button", TEXT_BTN1,WS_CHILD|WS_VISIBLE|WS_DISABLED, rc.right-120, 85, 110, 22, hDlg, (HMENU)IDC_BINARY, wincl.hInstance, NULL);    
	SendMessage(hCtrl, WM_SETFONT, (WPARAM)GetStockObject(18), 1L);
	hCtrl = CreateWindowEx(0, "button", " Affichage du code barre généré ",WS_CHILD|WS_VISIBLE|BS_GROUPBOX, 5, 125, rc.right-10, rc.bottom - 160, hDlg, (HMENU)0, wincl.hInstance, NULL);    
	SendMessage(hCtrl, WM_SETFONT, (WPARAM)GetStockObject(1), 1L);
	hCtrl = CreateWindowEx(WS_EX_DLGMODALFRAME|WS_EX_CLIENTEDGE, "static", "",WS_CHILD|WS_VISIBLE|SS_OWNERDRAW,/*|WS_BORDER,*/15, 145, rc.right-30, rc.bottom - 225,hDlg, (HMENU)IDC_IMAGE, wincl.hInstance, NULL);    
	SendMessage(hCtrl, WM_SETFONT, (WPARAM)GetStockObject(18), 1L);
	hCtrl = CreateWindowEx(0, "static", "",WS_CHILD|WS_VISIBLE|SS_ETCHEDHORZ, 15, rc.bottom-75, rc.right-30, 3,hDlg, NULL, wincl.hInstance, NULL); 
	hCtrl = CreateWindowEx(0, "Button", "Imprimer",WS_CHILD|WS_VISIBLE|WS_DISABLED, rc.right-125, rc.bottom-65, 110, 22, hDlg, (HMENU)IDC_PRINT, wincl.hInstance, NULL);    
	SendMessage(hCtrl, WM_SETFONT, (WPARAM)GetStockObject(18), 1L);
	hCtrl = CreateWindowEx(0, "Button", "Copier",WS_CHILD|WS_VISIBLE|WS_DISABLED, rc.right-236, rc.bottom-65, 110, 22, hDlg, (HMENU)IDC_COPY, wincl.hInstance, NULL);    
	SendMessage(hCtrl, WM_SETFONT, (WPARAM)GetStockObject(18), 1L);
	hCtrl = CreateWindowEx(0, "Button", "I -",WS_CHILD|WS_VISIBLE|WS_DISABLED, 15, rc.bottom-65, 40, 22, hDlg, (HMENU)IDC_VZOOMDOWN, wincl.hInstance, NULL);    
	SendMessage(hCtrl, WM_SETFONT, (WPARAM)GetStockObject(18), 1L);
	hCtrl = CreateWindowEx(0, "Button", "I +",WS_CHILD|WS_VISIBLE|WS_DISABLED, 56, rc.bottom-65, 40, 22, hDlg, (HMENU)IDC_VZOOMUP, wincl.hInstance, NULL);    
	SendMessage(hCtrl, WM_SETFONT, (WPARAM)GetStockObject(18), 1L);
	hCtrl = CreateWindowEx(0, "Button", "> - <",WS_CHILD|WS_VISIBLE|WS_DISABLED, 97, rc.bottom-65, 40, 22, hDlg, (HMENU)IDC_HZOOMDOWN, wincl.hInstance, NULL);    
	SendMessage(hCtrl, WM_SETFONT, (WPARAM)GetStockObject(18), 1L);
	hCtrl = CreateWindowEx(0, "Button", "< + >",WS_CHILD|WS_VISIBLE|WS_DISABLED, 138, rc.bottom-65, 40, 22, hDlg, (HMENU)IDC_HZOOMUP, wincl.hInstance, NULL);    
	SendMessage(hCtrl, WM_SETFONT, (WPARAM)GetStockObject(18), 1L);
	hCtrl = CreateWindowEx(0, "Button", "Fermer",WS_CHILD|WS_VISIBLE, rc.right-125, rc.bottom-25, 110, 22, hDlg, (HMENU)IDC_FILECLOSE, wincl.hInstance, NULL);    
	SendMessage(hCtrl, WM_SETFONT, (WPARAM)GetStockObject(18), 1L);
	hCtrl = CreateWindowEx(0, "Button", "Exporter",WS_CHILD|WS_VISIBLE, rc.right-236, rc.bottom-25, 110, 22, hDlg, (HMENU)IDC_FILEHELP, wincl.hInstance, NULL);    
	SendMessage(hCtrl, WM_SETFONT, (WPARAM)GetStockObject(18), 1L);
	hCtrl = CreateWindowEx(0, "Button", "A propos ...",WS_CHILD|WS_VISIBLE, rc.right-347, rc.bottom-25, 110, 22, hDlg, (HMENU)IDC_ABOUTHELP, wincl.hInstance, NULL);    
	SendMessage(hCtrl, WM_SETFONT, (WPARAM)GetStockObject(18), 1L);
	GetLocalTime(&st);
	wsprintf(buffer,"Nous sommes %s, le %2d %s %4d",jours[st.wDayOfWeek],st.wDay,mois[st.wMonth-1],st.wYear);
	hCtrl =CreateStatusWindow(WS_CHILD|WS_VISIBLE,buffer,hDlg,6000);
	SendMessage(hCtrl, WM_SETFONT, (WPARAM)GetStockObject(10), 1L);
	SendMessage(hCtrl, SB_SETBKCOLOR, (WPARAM)0,(LPARAM)RGB(192,192,192));
	HWND imghWnd = CreateWindowEx(0, "STATIC", NULL, WS_VISIBLE|WS_CHILD|SS_ICON,1, rc.bottom-33, 10, 10,hDlg , (HMENU)45000, wincl.hInstance, NULL);
	SendMessage(imghWnd, STM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadIcon(wincl.hInstance,(LPCTSTR)102)); //Logo Autheur
	SetTimer(hDlg,IDM_TIMER1,1000,(TIMERPROC) NULL);
	GetCurrentDirectory(256,szApp);
	EnableWindow(GetDlgItem(hDlg, IDC_FILEHELP), FALSE);
	ShowWindow (hDlg, nFunsterStil);
    while (GetMessage (&messages, NULL, 0, 0))    {        TranslateMessage(&messages);        DispatchMessage(&messages);    }    return messages.wParam;
}
LRESULT CALLBACK WindowProcedure (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam){
    switch (message)    {
	case WM_CREATE:{
        LPCREATESTRUCT lpcs = (LPCREATESTRUCT)lParam;
        SetWindowLong(hDlg, GWL_USERDATA,(LONG)lpcs->lpCreateParams);
			   SendMessage(hDlg, WM_SETFONT, (WPARAM)GetStockObject(18), 1L);
            }break;
        
        case WM_DESTROY:{
            PostQuitMessage (0);
            }break;
       case WM_COMMAND:{
		   int digit;
			switch(LOWORD(wParam))			{
			case IDC_DECODE:{
				GetDlgItemText(hDlg, IDC_EDIT, szBuf, 255);
				m_bitSize = Decode(szBuf);
				InvalidateRect(GetDlgItem(hDlg, IDC_IMAGE), NULL, TRUE);
				EnableWindow(GetDlgItem(hDlg, IDC_BINARY), (BOOL)m_bitSize);
				for (int i = IDC_PRINT; i <= IDC_VZOOMDOWN; i++)
					EnableWindow(GetDlgItem(hDlg, i), (BOOL)m_bitSize);
				SetResultCode(hDlg);
				if (!m_bitSize)
					MessageBox(hDlg, ErrorString(GetAppLastError()), g_szWndTitle, 64);
			}break;
				case IDC_BINARY:
					GetDlgItemText(hDlg, IDC_BINARY, szBuf, 64);
					if(!lstrcmp(szBuf, TEXT_BTN1))
						SetDlgItemText(hDlg, IDC_BINARY, TEXT_BTN2);
					else
						SetDlgItemText(hDlg, IDC_BINARY, TEXT_BTN1);
					SetResultCode(hDlg);
					break;
				case IDC_CHECKDIGIT:
					GetDlgItemText(hDlg, IDC_EDIT, szBuf, 255);
					digit = CheckDigit(szBuf, FALSE);
					wsprintf(szBuf, "Le checkdigit est %i", digit);
					MessageBox(hDlg, szBuf, g_szWndTitle, 64);
					break;
				case IDC_HZOOMUP:
					m_step = min(4, m_step+1);
					InvalidateRect(GetDlgItem(hDlg, IDC_IMAGE), NULL, TRUE);
					if(!IsWindowEnabled(GetDlgItem(hDlg, IDC_COPY)))
						EnableWindow(GetDlgItem(hDlg, IDC_COPY), TRUE);
						EnableWindow(GetDlgItem(hDlg, IDC_FILEHELP), TRUE);
					break;
				case IDC_HZOOMDOWN:
					m_step = max(1, m_step-1);
					InvalidateRect(GetDlgItem(hDlg, IDC_IMAGE), NULL, TRUE);
					if(!IsWindowEnabled(GetDlgItem(hDlg, IDC_COPY)))
						EnableWindow(GetDlgItem(hDlg, IDC_COPY), TRUE);
						EnableWindow(GetDlgItem(hDlg, IDC_FILEHELP), TRUE);
					break;
				case IDC_VZOOMUP:
					m_height = min(140, m_height+5);
					InvalidateRect(GetDlgItem(hDlg, IDC_IMAGE), NULL, TRUE);
					if(!IsWindowEnabled(GetDlgItem(hDlg, IDC_COPY)))
						EnableWindow(GetDlgItem(hDlg, IDC_COPY), TRUE);
						EnableWindow(GetDlgItem(hDlg, IDC_FILEHELP), TRUE);
					break;
				case IDC_VZOOMDOWN:
					HDC hdc;
					hdc = GetWindowDC(hDlg);
					m_height = max(GetMinimalHeight(hdc, m_step), m_height-5);
					ReleaseDC(hDlg, hdc);
					InvalidateRect(GetDlgItem(hDlg, IDC_IMAGE), NULL, TRUE);
					if(!IsWindowEnabled(GetDlgItem(hDlg, IDC_COPY)))
						EnableWindow(GetDlgItem(hDlg, IDC_FILEHELP), TRUE);
					break;        
				case IDC_PRINT:{
					CDlgPrintSetup * pDlg;    
					pDlg = new CDlgPrintSetup(&m_rcPrint);
					if(!pDlg)        return 2;
					if(!pDlg->Create(hDlg))    {delete pDlg; return 2;}
					delete pDlg;
					PRINTDLG prd;
					DOCINFO di;
					HDC hdcPrn;
					POINT ptLog;
					POINTFLOAT ptFloat; // nbre de pixel pour 1mm
					BOOL bOk, bEnd;
					ZeroMemory(&di, sizeof(DOCINFO));
					di.cbSize = sizeof(DOCINFO);
					di.lpszDocName = "Impression Code barre";
					ZeroMemory(&prd, sizeof(PRINTDLG));
					prd.lStructSize = sizeof(PRINTDLG);
					prd.hwndOwner = hDlg;
					prd.Flags = PD_RETURNDC|PD_NOPAGENUMS|PD_NOSELECTION|PD_HIDEPRINTTOFILE;
					if(!PrintDlg(&prd))        return 1;
					if((hdcPrn=prd.hDC)==NULL)        return 1;
					ptLog.x = GetDeviceCaps(hdcPrn, LOGPIXELSX);
					ptLog.y = GetDeviceCaps(hdcPrn, LOGPIXELSY);
					ptFloat.x = (float)ptLog.x/25.4f;
					ptFloat.y = (float)ptLog.y/25.4f;
					bOk = StartDoc(hdcPrn, &di);
					SetMapMode(hdcPrn, MM_TEXT);
					SetBkMode(hdcPrn, TRANSPARENT);
					bEnd = FALSE;
					while((bOk)&&(!bEnd))    {
						StartPage(hdcPrn);        
						bEnd = PreparePrint(hdcPrn, ptFloat, &m_rcPrint);
						bEnd = TRUE;
						bOk = EndPage(hdcPrn);
					}
					if(bOk) EndDoc(hdcPrn);
					else
						AbortDoc(hdcPrn);
					return DeleteDC(hdcPrn);
							   }break;
				case IDC_COPY:{
					HWND hMain;
					HBITMAP hBmp;
					HDC     hMem, hdc;
					POINT   pt;
					RECT    rc;    
					hMain = GetParent(GetParent(hDlg));
					if(!OpenClipboard(hMain))          return 3;
					EmptyClipboard();
					hdc = GetDC(hMain);
					hBmp = CreateCompatibleBitmap(hdc, GetApproximateWidth(m_step), m_height);
					if(!hBmp)  {        CloseClipboard();        return 4;    }
					hMem = CreateCompatibleDC(hdc);
					if(!hMem)    {        CloseClipboard();        DeleteObject(hBmp);        return 5;    }
					pt.x = pt.y = 0;
					rc.left = rc.top = 0;
					rc.right = GetApproximateWidth(m_step);
					rc.bottom = m_height;
					SelectObject(hMem, hBmp);
					FillRect(hMem, &rc, (HBRUSH)GetStockObject(WHITE_BRUSH));
					Draw(hMem, pt, m_height, m_step);
					SetClipboardData(CF_BITMAP, hBmp);
					DeleteDC(hMem);
					ReleaseDC(hMain, hdc);    
					DeleteObject(hBmp);
					CloseClipboard();
					EnableWindow(GetDlgItem(hDlg, IDC_COPY), FALSE);
							  }break;
				case IDC_FILEHELP:{
					HWND hMain;
					HBITMAP hBmp;
					HDC     hMem, hdc;
					POINT   pt;
					RECT    rc;    
					GetDlgItemText(hDlg,IDC_EDIT,buffer,128);
					hMain = GetParent(GetParent(hDlg));
					if(!OpenClipboard(hMain))          return 3;
					EmptyClipboard();
					hdc = GetDC(hMain);
					hBmp = CreateCompatibleBitmap(hdc, GetApproximateWidth(m_step), m_height);
					if(!hBmp)  {        CloseClipboard();        return 4;    }
					hMem = CreateCompatibleDC(hdc);
					if(!hMem)    {        CloseClipboard();        DeleteObject(hBmp);        return 5;    }
					pt.x = pt.y = 0;
					rc.left = rc.top = 0;
					rc.right = GetApproximateWidth(m_step);
					rc.bottom = m_height;
					SelectObject(hMem, hBmp);
					FillRect(hMem, &rc, (HBRUSH)GetStockObject(WHITE_BRUSH));
					Draw(hMem, pt, m_height, m_step);
					SetClipboardData(CF_BITMAP, hBmp);
					char szTmp[260];
					char buffer2[260];
					wsprintf(szTmp,"%s\\Datas\\CodesBarres\\%s.BMP",szApp,buffer);
					wsprintf(buffer2,"Voulez vous sauvegarder le codebarre graphique pour %s sous: \n%s ?",buffer,szTmp);
					if(MsgBox(wincl.hInstance,hDlg,buffer2,"Sauvegarde Graphique",MB_YESNO|MB_DEFBUTTON2,103)==0x06){CreateBMPFile(hMain,szTmp , CreateBitmapInfoStruct(hMain,hBmp),hBmp,hMem);}
					DeleteDC(hMem);
					ReleaseDC(hMain, hdc);    
					DeleteObject(hBmp);
					EmptyClipboard();
					CloseClipboard();
								  }break;
				case IDC_ABOUTHELP:{
					MSGBOXPARAMS  mbp;    
					mbp.cbSize      = sizeof(MSGBOXPARAMS);
					mbp.dwStyle     = MB_USERICON;
					mbp.hInstance   = wincl.hInstance;
					mbp.hwndOwner   = hDlg;
					mbp.lpszIcon    = MAKEINTRESOURCE(102);
					mbp.lpszCaption = " A propos de...";
					mbp.lpszText    = "      Code barre       Version 1.8 \n\n    ©Patrice Waechter-Ebling 2019-2020\n";
				   return  MessageBoxIndirect(&mbp);
								   }break;
				case IDC_FILECLOSE:
					PostMessage((hDlg), WM_CLOSE, 0, 0);
					break;
			}
					   }return 0;
			break;
        default:
            return DefWindowProc (hDlg, message, wParam, lParam);
    }
    return 0;
}
void SetResultCode(HWND hDlg){
    char szBuf[32], szBin[2048];
    int i;
    BYTE mask;
    const char * p = GetCodeString();    
    GetDlgItemText(hDlg, IDC_BINARY, szBuf, 64);
    if(!lstrcmp(szBuf, TEXT_BTN1))
        SetDlgItemText(hDlg, IDC_EDITRES, GetCodeString());
    else {
        szBin[0] = 0;
        for(i=0; i<m_bitSize; i++){
            mask = 0x80 >> (i%8);
            if(p[i/8] & mask)
                szBin[i] = '1';
            else
                szBin[i] = '0';
        }
        szBin[i] = 0;
        SetDlgItemText(hDlg, IDC_EDITRES, szBin);
    }
}
BOOL PreparePrint(HDC hdc, POINTFLOAT ptf, LPRECT lprc){
    POINT pt;
    int step, cy;    
    if(!m_bitSize)       return TRUE;
    pt.x = (int)(lprc->left*ptf.x);
    pt.y = (int)(lprc->top*ptf.y);
    step = GetApproximateStep((int)(lprc->right*ptf.x));
    cy = max((int)(lprc->bottom*ptf.y), GetMinimalHeight(hdc, step));
    Draw(hdc, pt, cy, step);
    return TRUE;
}
const char* ErrorString(int error){
    switch(error)    {
        case ERR_NONE:				 return "Pas d'erreur";
        case ERR_VAR_CODE_NULL:      return "Variable code ne devrait pas être nulle";
        case ERR_CODE_INCOMPATIBLE:  return "Code incompatible";
        case ERR_BAD_FORMAT_SIZE:    return "La taille de code barre est incompatible";
        case ERR_SIZE_LESS:          return "Taille de variable insuffisante";
        case ERR_CODE_TRUNCATE:      return "Le code barre est tronqué";
        default:					 return "Erreur inconnue";
    }
}
void SetBits(char **lppData, int * lpPos, WORD wCode, int len){
    if((!lppData)||(!lpPos)||(*lpPos>=8)||(len>16)||(len<0)||(*lpPos<0))        return;
    WORD wBuf;
    char byBuf; 
    BYTE byMask;
    int i, k;
    do{
        wBuf = wCode << (16 - len);
        byBuf = (BYTE)(wBuf >> (*lpPos + 8));
        byMask = 0xFF>>(*lpPos);
        **lppData = ((**lppData) & (~byMask)) | (byBuf & byMask);
        k = len + (*lpPos) - 8;
        if(k>=0) {*lpPos = 0;(*lppData)++;  } else  (*lpPos) += len;
        len = k;            
    }while(len>0);
}
int Decode(const char * pszCode)
{
	ZeroMemory(m_szData, sizeof(m_szData));
	m_nbBits = 0;
	if(!pszCode){m_iError = ERR_VAR_CODE_NULL;return 0;}    
	int len, i, j;
	len = lstrlen(pszCode);
	for(i=0; i<len; i++){if(SetIndex(pszCode[i])==-1){m_iError = ERR_CODE_INCOMPATIBLE;return 0;}}
	if(pszCode[0]=='*'){pszCode++;len--;}
	if(pszCode[len-1]=='*'){len--;}
	if(len==0){m_iError = ERR_VAR_CODE_NULL;return 0;}
	m_iError = ERR_NONE;
	int count = 0;
	int pos = 0;
	char * pData = m_szData;
	SetBits(&pData, &pos, cstTable128[39], 12);
	SetBits(&pData, &pos, 0, 1);count = 13;
	for(i=0; i<len; i++){SetBits(&pData, &pos, cstTable128[SetIndex(pszCode[i])], 12);SetBits(&pData, &pos, 0, 1);count += 13;}
	SetBits(&pData, &pos, cstTable128[39], 12);
	count += 12;
	m_nbBits = count;
	return count;
}
int GetApproximateStep(int width){return  (int)(width/(m_nbBits+20));}
int GetMinimalHeight(HDC hdc, int step){
    POINT ptLog;
    POINTFLOAT ptFloat;
    ptLog.y = GetDeviceCaps(hdc, LOGPIXELSY);
    ptFloat.y = (float)ptLog.y/25.4f;    
    return  (int)(ptFloat.y*7);
}
int CheckDigit(const char * pszCode, BOOL bComplet){    return 0;}

void Draw(HDC hdc, POINT pt, int height, int step){
    int len, i, j, count;
    RECT rect;
    BYTE mask;
    POINT ptLog;
    POINTFLOAT ptFloat;
    len = lstrlen(m_szData);
    if(len==0) return;
    step = max(1, step);
    ptLog.x = GetDeviceCaps(hdc, LOGPIXELSX);
    ptLog.y = GetDeviceCaps(hdc, LOGPIXELSY);
    ptFloat.x = (float)ptLog.x/25.4f;
    ptFloat.y = (float)ptLog.y/25.4f;
    SetRect(&rect, pt.x, pt.y, GetApproximateWidth(step) + pt.x, height + pt.y);    
    FillRect(hdc, &rect, (HBRUSH)GetStockObject(WHITE_BRUSH));
    pt.y += (int)(ptFloat.y*2.f);
    height -= (int)(ptFloat.y*4.f);
    rect.top = pt.y;
    for(i=0; i<len; i++)  {
        for(j=0; j<8; j++){
            mask = 0x80 >> j;
            rect.left = pt.x + step*10 + step*(8*i+j);
            rect.right = rect.left + step;
            rect.bottom = pt.y + height;            
            if(m_szData[i] & mask){
                FillRect(hdc, &rect, (HBRUSH)GetStockObject(BLACK_BRUSH));
            }
        }
    }
}
int SetIndex(char car){
    if((car>=48)&&(car<=57))        return (car-48);
    else if((car>=65)&&(car<=90))   return (car-55);
    else if((car>=97)&&(car<=122))  return (car-87);
    else if(car=='-')		        return 36;
    else if(car=='.')		        return 37;
    else if(car==' ')		        return 38;
    else if(car=='*')		        return 39;
    else if(car=='$')		        return 40;
    else if(car=='/')		        return 41;
    else if(car=='+')		        return 42;
    else if(car=='%')		        return 43;
    return -1;
}

int GetApproximateWidth(int step){    if(!m_nbBits) return 0;     return (m_nbBits+20)*step;}
int MsgBox(HINSTANCE hInst,HWND hDlg,char* lpszText,char* lpszCaption, DWORD dwStyle,int lpszIcon){
	MSGBOXPARAMS lpmbp;
	lpmbp.hInstance=hInst;
	lpmbp.cbSize=sizeof(MSGBOXPARAMS);
	lpmbp.hwndOwner=hDlg;
	lpmbp.dwLanguageId=MAKELANGID(0x0800,0x0800); //par defaut celui du systeme
	lpmbp.lpszText=lpszText;
	lpmbp.lpszCaption=lpszCaption;
	lpmbp.dwStyle=dwStyle|0x00000080L;
	lpmbp.lpszIcon=(LPCTSTR)lpszIcon;
	lpmbp.lpfnMsgBoxCallback=0;
	return  MessageBoxIndirect(&lpmbp);
}
void errhandler(LPCTSTR Texte,HWND hDlg){
	MsgBox(wincl.hInstance,hDlg,(char*)Texte,"Gestionnaire d'erreur",0,100);
}
PBITMAPINFO CreateBitmapInfoStruct(HWND hDlg, HBITMAP hBmp){ 
    PBITMAPINFO pbmi; 
    WORD    cClrBits; 
    if (!GetObject(hBmp, sizeof(BITMAP), (LPSTR)&btmp)) 
        errhandler("GetObject", hDlg); 
    cClrBits = (WORD)(btmp.bmPlanes * btmp.bmBitsPixel); 
    if (cClrBits == 1)         cClrBits = 1; 
    else if (cClrBits <= 4)    cClrBits = 4; 
    else if (cClrBits <= 8)    cClrBits = 8; 
    else if (cClrBits <= 16)   cClrBits = 16; 
    else if (cClrBits <= 24)   cClrBits = 24; 
    else cClrBits = 32; 
    if (cClrBits != 24)  pbmi = (PBITMAPINFO) LocalAlloc(LPTR,sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * (1<< cClrBits));      else          pbmi = (PBITMAPINFO) LocalAlloc(LPTR, sizeof(BITMAPINFOHEADER)); 
    pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER); 
    pbmi->bmiHeader.biWidth = btmp.bmWidth; 
    pbmi->bmiHeader.biHeight = btmp.bmHeight; 
    pbmi->bmiHeader.biPlanes = btmp.bmPlanes; 
    pbmi->bmiHeader.biBitCount = btmp.bmBitsPixel; 
    if (cClrBits < 24) pbmi->bmiHeader.biClrUsed = (1<<cClrBits); 
    pbmi->bmiHeader.biCompression = BI_RGB; 
    pbmi->bmiHeader.biSizeImage = ((pbmi->bmiHeader.biWidth * cClrBits +31) & ~31) /8* pbmi->bmiHeader.biHeight; 
    pbmi->bmiHeader.biClrImportant = 0; 
    return pbmi; 
}
void CreateBMPFile(HWND hDlg, LPTSTR pszFile, PBITMAPINFO pbi,HBITMAP hBMP, HDC hDC) { 
     HANDLE hf;
	 BITMAPFILEHEADER hdr;
	 PBITMAPINFOHEADER pbih;
	 LPBYTE lpBits;
	 DWORD dwTotal,cb,dwTmp; 
	 BYTE *hp; 
	 pbih = (PBITMAPINFOHEADER) pbi; 
	 lpBits = (LPBYTE) GlobalAlloc(GMEM_FIXED, pbih->biSizeImage);
	 if (!lpBits)          errhandler("Alloction des tempons globaux", hDlg); 
	 if (!GetDIBits(hDC, hBMP, 0, (WORD) pbih->biHeight, lpBits, pbi,DIB_RGB_COLORS)) {     errhandler("Détection des bist couleur de l'image", hDlg);     }
     hf = CreateFile(pszFile,GENERIC_READ | GENERIC_WRITE, (DWORD) 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, (HANDLE) NULL); 
     if (hf == INVALID_HANDLE_VALUE) errhandler("Création du fichier Image", hDlg); 
     hdr.bfType = 0x4d42;
     hdr.bfSize = (DWORD) (sizeof(BITMAPFILEHEADER) + pbih->biSize + pbih->biClrUsed * sizeof(RGBQUAD) + pbih->biSizeImage); 
     hdr.bfReserved1 = 0; 
     hdr.bfReserved2 = 0; 
     hdr.bfOffBits = (DWORD) sizeof(BITMAPFILEHEADER) + pbih->biSize + pbih->biClrUsed * sizeof (RGBQUAD); 
     if (!WriteFile(hf, (LPVOID) &hdr, sizeof(BITMAPFILEHEADER), (LPDWORD) &dwTmp,  NULL)){ errhandler("Écriture du fichier Image", hDlg); }
     if (!WriteFile(hf, (LPVOID) pbih, sizeof(BITMAPINFOHEADER) + pbih->biClrUsed * sizeof (RGBQUAD), (LPDWORD) &dwTmp, ( NULL)) )   errhandler("Écriture du fichier Image", hDlg); 
     dwTotal = cb = pbih->biSizeImage; 
     hp = lpBits; 
     if (!WriteFile(hf, (LPSTR) hp, (int) cb, (LPDWORD) &dwTmp,NULL))    errhandler("Écriture du fichier Image", hDlg); 
     if (!CloseHandle(hf))     errhandler("Fermeture de la main", hDlg); 
	 GlobalFree((HGLOBAL)lpBits);
} 
int ModeTelecommande(HWND hDlg){
	HWND hMain;
	HBITMAP hBmp;
	HDC     hMem, hdc;
	POINT   pt;
	RECT    rc;    
	GetDlgItemText(hDlg, IDC_EDIT, buffer, 255);
	m_bitSize = Decode(buffer);
	InvalidateRect(GetDlgItem(hDlg, IDC_IMAGE), NULL, TRUE);
	EnableWindow(GetDlgItem(hDlg, IDC_BINARY), (BOOL)m_bitSize);
	for(int i=IDC_PRINT; i<=IDC_VZOOMDOWN; i++)
	EnableWindow(GetDlgItem(hDlg, i),(BOOL)m_bitSize );
	SetResultCode(hDlg);
	if(!m_bitSize)MessageBox(hDlg,ErrorString(GetAppLastError()),g_szWndTitle, 64);
	m_step = min(4, m_step+1);
	InvalidateRect(GetDlgItem(hDlg, IDC_IMAGE), NULL, TRUE);
	if(!IsWindowEnabled(GetDlgItem(hDlg, IDC_COPY)))
	EnableWindow(GetDlgItem(hDlg, IDC_COPY), TRUE);
	GetDlgItemText(hDlg,IDC_EDIT,buffer,128);
	hMain = GetParent(GetParent(hDlg));
	if(!OpenClipboard(hMain))          return 3;
	EmptyClipboard();
	hdc = GetDC(hMain);
	hBmp = CreateCompatibleBitmap(hdc, GetApproximateWidth(m_step), m_height);
	if(!hBmp)  {        CloseClipboard();        return 4;    }
	hMem = CreateCompatibleDC(hdc);
	if(!hMem)    {        CloseClipboard();        DeleteObject(hBmp);        return 5;    }
	pt.x = pt.y = 0;
	rc.left = rc.top = 0;
	rc.right = GetApproximateWidth(m_step);
	rc.bottom = m_height;
	SelectObject(hMem, hBmp);
	FillRect(hMem, &rc, (HBRUSH)GetStockObject(WHITE_BRUSH));
	Draw(hMem, pt, m_height, m_step);
	SetClipboardData(CF_BITMAP, hBmp);
	strcat(buffer,".bmp");
	MsgBox(wincl.hInstance,hDlg,buffer,"Info Destination",64,102);
	CreateBMPFile(hMain,buffer , CreateBitmapInfoStruct(hMain,hBmp),hBmp,hMem);
	DeleteDC(hMem);
	ReleaseDC(hMain, hdc);    
	DeleteObject(hBmp);
	EmptyClipboard();
	return CloseClipboard();
	
}