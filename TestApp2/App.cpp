// App.cpp : Defines the entry point for the application.
//

#include "stdafx.h"

#include <io.h>

//#include <olectl.h>
//#pragma comment(lib, "oleaut32.lib")
#include "save_icon_file_by_handle.h"

#include "OSSObjectSample.h"

#include "VendorInfo.h"
#include "UploadFileInfo.h"
#include "GetWindowsInfo.h"

using namespace yg_icon;

std::string g_strWorkDir;
std::string g_strBucketName;
std::vector<UploadFileInfo> g_arrUploadFiles;

DWORD g_dwMajorVersion;
DWORD g_dwMinorVersion;
DWORD g_dwBuildNumber;
std::string g_strWindowsName;
DWORD g_dwProcessorArchitecture;  

#ifndef _DWMAPI_H_
typedef struct DWM_BLURBEHIND
{
    DWORD dwFlags;
    BOOL fEnable;
    HRGN hRgnBlur;
    BOOL fTransitionOnMaximized;
} DWM_BLURBEHIND;

typedef struct tagDWL_MARGINS {
    int cxLeftWidth;
    int cxRightWidth;
    int cyTopHeight;
    int cyBottomHeight;
} DWM_MARGINS, *PDWM_MARGINS;

// Window attributes
enum DWMWINDOWATTRIBUTE
{
    DWMWA_NCRENDERING_ENABLED = 1,      // [get] Is non-client rendering enabled/disabled
    DWMWA_NCRENDERING_POLICY,           // [set] Non-client rendering policy
    DWMWA_TRANSITIONS_FORCEDISABLED,    // [set] Potentially enable/forcibly disable transitions
    DWMWA_ALLOW_NCPAINT,                // [set] Allow contents rendered in the non-client area to be visible on the DWM-drawn frame.
    DWMWA_CAPTION_BUTTON_BOUNDS,        // [get] Bounds of the caption button area in window-relative space.
    DWMWA_NONCLIENT_RTL_LAYOUT,         // [set] Is non-client content RTL mirrored
    DWMWA_FORCE_ICONIC_REPRESENTATION,  // [set] Force this window to display iconic thumbnails.
    DWMWA_FLIP3D_POLICY,                // [set] Designates how Flip3D will treat the window.
    DWMWA_EXTENDED_FRAME_BOUNDS,        // [get] Gets the extended frame bounds rectangle in screen space
    DWMWA_HAS_ICONIC_BITMAP,            // [set] Indicates an available bitmap when there is no better thumbnail representation.
    DWMWA_DISALLOW_PEEK,                // [set] Don't invoke Peek on the window.
    DWMWA_EXCLUDED_FROM_PEEK,           // [set] LivePreview exclusion information
    DWMWA_CLOAK,                        // [set] Cloak or uncloak the window
    DWMWA_CLOAKED,                      // [get] Gets the cloaked state of the window
    DWMWA_FREEZE_REPRESENTATION,        // [set] Force this window to freeze the thumbnail without live update
    DWMWA_LAST
};

// Non-client rendering policy attribute values
enum DWMNCRENDERINGPOLICY
{
    DWMNCRP_USEWINDOWSTYLE, // Enable/disable non-client rendering based on window style
    DWMNCRP_DISABLED,       // Disabled non-client rendering; window style is ignored
    DWMNCRP_ENABLED,        // Enabled non-client rendering; window style is ignored
    DWMNCRP_LAST
};

// Values designating how Flip3D treats a given window.
enum DWMFLIP3DWINDOWPOLICY
{
    DWMFLIP3D_DEFAULT,      // Hide or include the window in Flip3D based on window style and visibility.
    DWMFLIP3D_EXCLUDEBELOW, // Display the window under Flip3D and disabled.
    DWMFLIP3D_EXCLUDEABOVE, // Display the window above Flip3D and enabled.
    DWMFLIP3D_LAST
};

#define DWM_BB_ENABLE                 0x00000001
#define DWM_BB_BLURREGION             0x00000002
#define DWM_BB_TRANSITIONONMAXIMIZED  0x00000004

#define DWM_EC_DISABLECOMPOSITION     0x00000000
#define DWM_EC_ENABLECOMPOSITION      0x00000001
#endif // _DWMAPI_H_


class CDwm
{
public:
    typedef HRESULT (WINAPI *FNDWMENABLECOMPOSITION)(UINT);
    typedef HRESULT (WINAPI *FNDWNISCOMPOSITIONENABLED)(LPBOOL);
    typedef HRESULT (WINAPI *FNENABLEBLURBEHINDWINDOW)(HWND, CONST DWM_BLURBEHIND*);
    typedef HRESULT (WINAPI *FNDWMEXTENDFRAMEINTOCLIENTAREA)(HWND, CONST DWM_MARGINS*);
    typedef HRESULT (WINAPI *FNDWMSETWINDOWATTRIBUTE)(HWND, DWORD, LPCVOID pvAttribute, DWORD);

    FNDWMENABLECOMPOSITION fnDwmEnableComposition;
    FNDWNISCOMPOSITIONENABLED fnDwmIsCompositionEnabled;
    FNENABLEBLURBEHINDWINDOW fnDwmEnableBlurBehindWindow;
    FNDWMEXTENDFRAMEINTOCLIENTAREA fnDwmExtendFrameIntoClientArea;
    FNDWMSETWINDOWATTRIBUTE fnDwmSetWindowAttribute;

    CDwm()
    {
        static HINSTANCE hDwmInstance = ::LoadLibrary(_T("dwmapi.dll"));
        if( hDwmInstance != NULL ) {
            fnDwmEnableComposition = (FNDWMENABLECOMPOSITION) ::GetProcAddress(hDwmInstance, "DwmEnableComposition");
            fnDwmIsCompositionEnabled = (FNDWNISCOMPOSITIONENABLED) ::GetProcAddress(hDwmInstance, "DwmIsCompositionEnabled");
            fnDwmEnableBlurBehindWindow = (FNENABLEBLURBEHINDWINDOW) ::GetProcAddress(hDwmInstance, "DwmEnableBlurBehindWindow");
            fnDwmExtendFrameIntoClientArea = (FNDWMEXTENDFRAMEINTOCLIENTAREA) ::GetProcAddress(hDwmInstance, "DwmExtendFrameIntoClientArea");
            fnDwmSetWindowAttribute = (FNDWMSETWINDOWATTRIBUTE) ::GetProcAddress(hDwmInstance, "DwmSetWindowAttribute");
        }
        else {
            fnDwmEnableComposition = NULL;
            fnDwmIsCompositionEnabled = NULL;
            fnDwmEnableBlurBehindWindow = NULL;
            fnDwmExtendFrameIntoClientArea = NULL;
            fnDwmSetWindowAttribute = NULL;
        }
    }

    BOOL IsCompositionEnabled() const
    {
        HRESULT Hr = E_NOTIMPL;
        BOOL bRes = FALSE;
        if( fnDwmIsCompositionEnabled != NULL ) Hr = fnDwmIsCompositionEnabled(&bRes);
        return SUCCEEDED(Hr) && bRes;
    }

    BOOL EnableComposition(UINT fEnable)
    {
        BOOL bRes = FALSE;
        if( fnDwmEnableComposition != NULL ) bRes = SUCCEEDED(fnDwmEnableComposition(fEnable));
        return bRes;
    }

    BOOL EnableBlurBehindWindow(HWND hWnd)
    {
        BOOL bRes = FALSE;
        if( fnDwmEnableBlurBehindWindow != NULL ) {
            DWM_BLURBEHIND bb = { 0 };
            bb.dwFlags = DWM_BB_ENABLE;
            bb.fEnable = TRUE;
            bRes = SUCCEEDED(fnDwmEnableBlurBehindWindow(hWnd, &bb));
        }
        return bRes;
    }

    BOOL EnableBlurBehindWindow(HWND hWnd, CONST DWM_BLURBEHIND& bb)
    {
        BOOL bRes = FALSE;
        if( fnDwmEnableBlurBehindWindow != NULL ) {
            bRes = SUCCEEDED(fnDwmEnableBlurBehindWindow(hWnd, &bb));
        }
        return bRes;
    }

    BOOL ExtendFrameIntoClientArea(HWND hWnd, CONST DWM_MARGINS& Margins)
    {
        BOOL bRes = FALSE;
        if( fnDwmEnableComposition != NULL ) bRes = SUCCEEDED(fnDwmExtendFrameIntoClientArea(hWnd, &Margins));
        return bRes;
    }

    BOOL SetWindowAttribute(HWND hwnd, DWORD dwAttribute, LPCVOID pvAttribute, DWORD cbAttribute)
    {
        BOOL bRes = FALSE;
        if( fnDwmSetWindowAttribute != NULL ) bRes = SUCCEEDED(fnDwmSetWindowAttribute(hwnd, dwAttribute, pvAttribute, cbAttribute));
        return bRes;
    }
};

#ifndef WM_DPICHANGED
#   define WM_DPICHANGED       0x02E0
#endif


#ifndef _SHELLSCALINGAPI_H_
typedef enum _PROCESS_DPI_AWARENESS { 
    PROCESS_DPI_UNAWARE            = 0,
    PROCESS_SYSTEM_DPI_AWARE       = 1,
    PROCESS_PER_MONITOR_DPI_AWARE  = 2
} PROCESS_DPI_AWARENESS;

typedef enum _MONITOR_DPI_TYPE { 
    MDT_EFFECTIVE_DPI  = 0,
    MDT_ANGULAR_DPI    = 1,
    MDT_RAW_DPI        = 2,
    MDT_DEFAULT        = MDT_EFFECTIVE_DPI
} Monitor_DPI_Type;
#endif // _SHELLSCALINGAPI_H_

// reply of the requery  
size_t req_reply(void *ptr, size_t size, size_t nmemb, void *stream)
{
	cout << "----->reply" << endl;
	string *str = (string*)stream;
	cout << *str << endl;
	(*str).append((char*)ptr, size*nmemb);
	return size * nmemb;
}

class CDPI
{
public:
    typedef BOOL (WINAPI *FNSETPROCESSDPIAWARE)(VOID);
    typedef HRESULT (WINAPI *FNSETPROCESSDPIAWARENESS)(PROCESS_DPI_AWARENESS);
    typedef HRESULT (WINAPI *FNGETDPIFORMONITOR)(HMONITOR, Monitor_DPI_Type, UINT*, UINT*);

    FNSETPROCESSDPIAWARE fnSetProcessDPIAware; // vista,win7
    FNSETPROCESSDPIAWARENESS fnSetProcessDpiAwareness; // win8+
    FNGETDPIFORMONITOR fnGetDpiForMonitor; //

    CDPI() {
        m_nScaleFactor = 0;
        m_nScaleFactorSDA = 0;
        m_Awareness = PROCESS_DPI_UNAWARE;

        static HINSTANCE hUser32Instance = ::LoadLibrary(_T("User32.dll"));
        static HINSTANCE hShcoreInstance = ::LoadLibrary(_T("Shcore.dll"));
        if( hUser32Instance != NULL ) {
            fnSetProcessDPIAware = (FNSETPROCESSDPIAWARE) ::GetProcAddress(hUser32Instance, "SetProcessDPIAware");
        }
        else {
            fnSetProcessDPIAware = NULL;
        }

        if( hShcoreInstance != NULL ) {
            fnSetProcessDpiAwareness = (FNSETPROCESSDPIAWARENESS) ::GetProcAddress(hShcoreInstance, "SetProcessDpiAwareness");
            fnGetDpiForMonitor = (FNGETDPIFORMONITOR) ::GetProcAddress(hShcoreInstance, "GetDpiForMonitor");
        }
        else {
            fnSetProcessDpiAwareness = NULL;
            fnGetDpiForMonitor = NULL;
        }

        if (fnGetDpiForMonitor != NULL) {
            UINT     dpix = 0, dpiy = 0;
            HRESULT  hr = E_FAIL;
            POINT pt = {1, 1};
            HMONITOR hMonitor = ::MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
            hr = fnGetDpiForMonitor(hMonitor, MDT_EFFECTIVE_DPI, &dpix, &dpiy);
            SetScale(dpix);
        }
        else {
            UINT     dpix = 0;
            HDC hDC = ::GetDC(::GetDesktopWindow());
            dpix = GetDeviceCaps(hDC, LOGPIXELSX);
            ::ReleaseDC(::GetDesktopWindow(), hDC);
            SetScale(dpix);
        }

        SetAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
    }

    PROCESS_DPI_AWARENESS GetAwareness()
    {
        return m_Awareness;
    }

    int  Scale(int x)
    {
        if (m_Awareness == PROCESS_DPI_UNAWARE) return x;
        if (m_Awareness == PROCESS_SYSTEM_DPI_AWARE) return MulDiv(x, m_nScaleFactorSDA, 100);
        return MulDiv(x, m_nScaleFactor, 100); // PROCESS_PER_MONITOR_DPI_AWARE
    }

    UINT GetScale()
    {
        if (m_Awareness == PROCESS_DPI_UNAWARE) return 100;
        if (m_Awareness == PROCESS_SYSTEM_DPI_AWARE) return m_nScaleFactorSDA;
        return m_nScaleFactor;
    }

    void ScaleRect(__inout RECT *pRect)
    {
        pRect->left = Scale(pRect->left);
        pRect->right = Scale(pRect->right);
        pRect->top = Scale(pRect->top);
        pRect->bottom = Scale(pRect->bottom);
    }

    void ScalePoint(__inout POINT *pPoint)
    {
        pPoint->x = Scale(pPoint->x);
        pPoint->y = Scale(pPoint->y);
    }
    
    void OnDPIChanged(HWND hWnd, WPARAM wParam, LPARAM lParam) 
    {
        SetScale(LOWORD(wParam));
        RECT* const prcNewWindow = (RECT*)lParam;
        ::SetWindowPos(hWnd,
            NULL,
            prcNewWindow ->left,
            prcNewWindow ->top,
            prcNewWindow->right - prcNewWindow->left,
            prcNewWindow->bottom - prcNewWindow->top,
            SWP_NOZORDER | SWP_NOACTIVATE);
    }

private:
    UINT m_nScaleFactor;
    UINT m_nScaleFactorSDA;
    PROCESS_DPI_AWARENESS m_Awareness;

    BOOL SetAwareness(PROCESS_DPI_AWARENESS value) 
    {
        if( fnSetProcessDpiAwareness != NULL ) {
            HRESULT Hr = E_NOTIMPL;
            Hr = fnSetProcessDpiAwareness(value);
            if (Hr == S_OK) {
                m_Awareness = value;
                return TRUE;
            }
            else {
                return FALSE;
            }
        }
        else {
            if (fnSetProcessDPIAware) {
                BOOL bRet = fnSetProcessDPIAware();
                if (bRet) m_Awareness = PROCESS_SYSTEM_DPI_AWARE;
                return bRet;
            }
        }
        return FALSE;
    }

    void SetScale(__in UINT iDPI)
    {
        m_nScaleFactor = MulDiv(iDPI, 100, 96);
        if (m_nScaleFactorSDA == 0) m_nScaleFactorSDA = m_nScaleFactor;
    }
};

int idx = 0;
ICONDIR ico;

void SaveIcon2(HICON hIconToSave, LPCTSTR sIconFileName)
{
	SaveFileByHIcon save_by_handle;
	std::map<unsigned int, HICON> map_icon;
	map_icon.insert(std::make_pair(1, hIconToSave));
	save_by_handle.SaveIconFile(map_icon, sIconFileName);
}

void SaveIcon(HICON hIconToSave, LPCTSTR sIconFileName)
{
	/*if (hIconToSave == NULL || sIconFileName == NULL)
		return;
	//warning: this code snippet is not bullet proof.  
	//do error check by yourself [masterz]  
	PICTDESC picdesc;
	picdesc.cbSizeofstruct = sizeof(PICTDESC);
	picdesc.picType = PICTYPE_ICON;
	picdesc.icon.hicon = hIconToSave;
	IPicture* pPicture = NULL;
	OleCreatePictureIndirect(&picdesc, IID_IPicture, TRUE, (VOID**)&pPicture);
	LPSTREAM pStream;
	CreateStreamOnHGlobal(NULL, TRUE, &pStream);
	LONG size;
	HRESULT hr = pPicture->SaveAsFile(pStream, TRUE, &size);
	char pathbuf[1024];
	memcpy(pathbuf, sIconFileName, strlen(sIconFileName));
	
	FILE *f = NULL;
	
	fopen_s(&f, sIconFileName, "wb");
	//fwrite(pData, nSizeOfIconRes, 1, f);
	//fclose(f);

	LARGE_INTEGER li;
	li.HighPart = 0;
	li.LowPart = 0;
	ULARGE_INTEGER ulnewpos;
	pStream->Seek(li, STREAM_SEEK_SET, &ulnewpos);
	ULONG uReadCount = 1;
	while (uReadCount>0)
	{
		pStream->Read(pathbuf, sizeof(pathbuf), &uReadCount);
		if (uReadCount>0)
			//iconfile.Write(pathbuf, uReadCount);
			fwrite(pathbuf, uReadCount, 1, f);
	}
	pStream->Release();
	//iconfile.Close();
	fclose(f);*/
}

typedef struct tagEnumResourceNamesParams {
	int i;
	int j;
	int k;
	char iconDir[1024];
} ENUMRESOURCENAMESPARAMS, *PENUMRESOURCENAMESPARAMS;

BOOL  UpdateIcons(HMODULE hModule, LPCTSTR lpszType, LPCTSTR lpszName, LONG lParam)
{
	ENUMRESOURCENAMESPARAMS *params = (ENUMRESOURCENAMESPARAMS *)lParam;
	HRSRC hRes = FindResourceA(hModule, lpszName, lpszType);
	HGLOBAL hResLoaded = LoadResource(hModule, hRes);
	void* pData = LockResource(hResLoaded);
	int nSizeOfIconRes = SizeofResource(hModule, hRes);

	int nWidth = ((byte*)pData)[4];

	/*
	int cbString = 0;
	char szBuffer[256];
	if ((ULONG)lpszName & 0xFFFF0000)
	{
	cbString = sprintf(szBuffer, "%s", lpszName);
	}
	else
	{
	cbString = sprintf(szBuffer, "%u", (USHORT)lpszName);
	}
	*/

	int index = ++idx;

	
	int CXICON = nWidth;
	int CYICON = nWidth;

	HICON  hIcon = CreateIconFromResourceEx((PBYTE)pData, SizeofResource(hModule, hRes), TRUE, 0x00030000, CXICON, CYICON, LR_DEFAULTCOLOR);
	
	ICONINFO icon_info;
	::GetIconInfo(hIcon, &icon_info);

	TCHAR icon_file_path[256];
	sprintf(icon_file_path, _T("%s\\%d_%dx%d.ico"), params->iconDir, index, CXICON, CYICON);

	SaveIcon2(hIcon, icon_file_path);
	
	

	UnlockResource(hResLoaded);
	FreeResource(hResLoaded);

	return TRUE;
}

class CDialogBuilderCallbackEx : public IDialogBuilderCallback
{
public:
	CControlUI* CreateControl(LPCTSTR pstrClass)
	{
		//if (_tcscmp(pstrClass, _T("GameList")) == 0) return new GameListUI;
		//else if (_tcscmp(pstrClass, _T("DeskList")) == 0) return new DeskListUI;
		return NULL;
	}
};

class CLoginFrameWnd : public CWindowWnd, public INotifyUI, public IMessageFilterUI
{
public:
	CLoginFrameWnd() { };
	LPCTSTR GetWindowClassName() const { return _T("UILoginFrame"); };
	UINT GetClassStyle() const { return UI_CLASSSTYLE_DIALOG; };
	void OnFinalMessage(HWND /*hWnd*/)
	{
		m_pm.RemovePreMessageFilter(this);
		delete this;
	};

	void Init() {
		CComboUI* pAccountCombo = static_cast<CComboUI*>(m_pm.FindControl(_T("accountcombo")));
		CEditUI* pAccountEdit = static_cast<CEditUI*>(m_pm.FindControl(_T("accountedit")));
		if (pAccountCombo && pAccountEdit) pAccountEdit->SetText(pAccountCombo->GetText());
		pAccountEdit->SetFocus();
	}

	void Notify(TNotifyUI& msg)
	{
		if (msg.sType == _T("click")) 
		{
			if (msg.pSender->GetName() == _T("closebtn")) 
			{ 
				PostQuitMessage(0); 
				return; 
			}
			else if (msg.pSender->GetName() == _T("loginBtn")) 
			{ 
				Close(); 
				return; 
			}
		}
		else if (msg.sType == _T("itemselect")) 
		{
			if (msg.pSender->GetName() == _T("accountcombo")) 
			{
				CEditUI* pAccountEdit = static_cast<CEditUI*>(m_pm.FindControl(_T("accountedit")));
				if (pAccountEdit) pAccountEdit->SetText(msg.pSender->GetText());
			}
		}
	}

	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		LONG styleValue = ::GetWindowLong(*this, GWL_STYLE);
		styleValue &= ~WS_CAPTION;
		::SetWindowLong(*this, GWL_STYLE, styleValue | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);

		m_pm.Init(m_hWnd);
		m_pm.AddPreMessageFilter(this);
		CDialogBuilder builder;
		CDialogBuilderCallbackEx cb;
		CControlUI* pRoot = builder.Create(_T("./skin/YouziRes/login.xml"), (UINT)0, &cb, &m_pm);
		ASSERT(pRoot && "Failed to parse XML");
		m_pm.AttachDialog(pRoot);
		m_pm.AddNotifier(this);

		Init();
		return 0;
	}

	LRESULT OnNcActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if (::IsIconic(*this)) bHandled = FALSE;
		return (wParam == 0) ? TRUE : FALSE;
	}

	LRESULT OnNcCalcSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		return 0;
	}

	LRESULT OnNcPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		return 0;
	}

	LRESULT OnNcHitTest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		POINT pt; pt.x = GET_X_LPARAM(lParam); pt.y = GET_Y_LPARAM(lParam);
		::ScreenToClient(*this, &pt);

		RECT rcClient;
		::GetClientRect(*this, &rcClient);

		RECT rcCaption = m_pm.GetCaptionRect();
		if (pt.x >= rcClient.left + rcCaption.left && pt.x < rcClient.right - rcCaption.right \
			&& pt.y >= rcCaption.top && pt.y < rcCaption.bottom) {
			CControlUI* pControl = static_cast<CControlUI*>(m_pm.FindControl(pt));
			if (pControl && _tcscmp(pControl->GetClass(), DUI_CTR_BUTTON) != 0)
				return HTCAPTION;
		}

		return HTCLIENT;
	}

	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		SIZE szRoundCorner = m_pm.GetRoundCorner();
		if (!::IsIconic(*this) && (szRoundCorner.cx != 0 || szRoundCorner.cy != 0)) {
			CDuiRect rcWnd;
			::GetWindowRect(*this, &rcWnd);
			rcWnd.Offset(-rcWnd.left, -rcWnd.top);
			rcWnd.right++; rcWnd.bottom++;
			HRGN hRgn = ::CreateRoundRectRgn(rcWnd.left, rcWnd.top, rcWnd.right, rcWnd.bottom, szRoundCorner.cx, szRoundCorner.cy);
			::SetWindowRgn(*this, hRgn, TRUE);
			::DeleteObject(hRgn);
		}

		bHandled = FALSE;
		return 0;
	}

	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		LRESULT lRes = 0;
		BOOL bHandled = TRUE;
		switch (uMsg) {
		case WM_CREATE:        lRes = OnCreate(uMsg, wParam, lParam, bHandled); break;
		case WM_NCACTIVATE:    lRes = OnNcActivate(uMsg, wParam, lParam, bHandled); break;
		case WM_NCCALCSIZE:    lRes = OnNcCalcSize(uMsg, wParam, lParam, bHandled); break;
		case WM_NCPAINT:       lRes = OnNcPaint(uMsg, wParam, lParam, bHandled); break;
		case WM_NCHITTEST:     lRes = OnNcHitTest(uMsg, wParam, lParam, bHandled); break;
		case WM_SIZE:          lRes = OnSize(uMsg, wParam, lParam, bHandled); break;
		default:
			bHandled = FALSE;
		}
		if (bHandled) return lRes;
		if (m_pm.MessageHandler(uMsg, wParam, lParam, lRes)) return lRes;
		return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
	}

	LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
	{
		if (uMsg == WM_KEYDOWN) {
			if (wParam == VK_RETURN) {
				CEditUI* pEdit = static_cast<CEditUI*>(m_pm.FindControl(_T("accountedit")));
				if (pEdit->GetText().IsEmpty()) pEdit->SetFocus();
				else {
					pEdit = static_cast<CEditUI*>(m_pm.FindControl(_T("pwdedit")));
					if (pEdit->GetText().IsEmpty()) pEdit->SetFocus();
					else Close();
				}
				return true;
			}
			else if (wParam == VK_ESCAPE) {
				PostQuitMessage(0);
				return true;
			}

		}
		return false;
	}

public:
	CPaintManagerUI m_pm;
};

class CFrameWindowWnd : public CWindowWnd, public INotifyUI, public CDwm, public CDPI
{
public:
    CFrameWindowWnd() : m_pWndShadow(NULL) { };
    LPCTSTR GetWindowClassName() const { return _T("UIMainFrame"); };
    UINT GetClassStyle() const { return UI_CLASSSTYLE_FRAME | CS_DBLCLKS; };
    void OnFinalMessage(HWND /*hWnd*/) { delete this; };

    void Init() {
		int j = 0;

		m_pMinBtn = static_cast<CButtonUI*>(m_pm.FindControl(_T("minmim_button")));
	}

    bool OnHChanged(void* param) {
        TNotifyUI* pMsg = (TNotifyUI*)param;
        if( pMsg->sType == _T("valuechanged") ) {
            short H, S, L;
            CPaintManagerUI::GetHSL(&H, &S, &L);
            CPaintManagerUI::SetHSL(true, (static_cast<CSliderUI*>(pMsg->pSender))->GetValue(), S, L);
        }
        return true;
    }

    bool OnSChanged(void* param) {
        TNotifyUI* pMsg = (TNotifyUI*)param;
        if( pMsg->sType == _T("valuechanged") ) {
            short H, S, L;
            CPaintManagerUI::GetHSL(&H, &S, &L);
            CPaintManagerUI::SetHSL(true, H, (static_cast<CSliderUI*>(pMsg->pSender))->GetValue(), L);
        }
        return true;
    }

    bool OnLChanged(void* param) {
        TNotifyUI* pMsg = (TNotifyUI*)param;
        if( pMsg->sType == _T("valuechanged") ) {
            short H, S, L;
            CPaintManagerUI::GetHSL(&H, &S, &L);
            CPaintManagerUI::SetHSL(true, H, S, (static_cast<CSliderUI*>(pMsg->pSender))->GetValue());
        }
        return true;
    }

    bool OnAlphaChanged(void* param) {
        TNotifyUI* pMsg = (TNotifyUI*)param;
        if( pMsg->sType == _T("valuechanged") ) {
            m_pm.SetOpacity((static_cast<CSliderUI*>(pMsg->pSender))->GetValue());
        }
        return true;
    }

    void OnPrepare() 
    {
        CSliderUI* pSilder = static_cast<CSliderUI*>(m_pm.FindControl(_T("alpha_controlor")));
        if( pSilder ) pSilder->OnNotify += MakeDelegate(this, &CFrameWindowWnd::OnAlphaChanged);
        pSilder = static_cast<CSliderUI*>(m_pm.FindControl(_T("h_controlor")));
        if( pSilder ) pSilder->OnNotify += MakeDelegate(this, &CFrameWindowWnd::OnHChanged);
        pSilder = static_cast<CSliderUI*>(m_pm.FindControl(_T("s_controlor")));
        if( pSilder ) pSilder->OnNotify += MakeDelegate(this, &CFrameWindowWnd::OnSChanged);
        pSilder = static_cast<CSliderUI*>(m_pm.FindControl(_T("l_controlor")));
        if( pSilder ) pSilder->OnNotify += MakeDelegate(this, &CFrameWindowWnd::OnLChanged);

		// 获取系统磁盘
		std::vector<string> systemDrives =  GetSystemDrives();
		CComboUI* pSystemDriveComboBox = static_cast<CComboUI*>(m_pm.FindControl(_T("system_drive_combo")));
		pSystemDriveComboBox->RemoveAll();
		for (int i = 0; i < systemDrives.size(); ++i)
		{
			CListLabelElementUI* elem = new CListLabelElementUI();
			elem->SetText(systemDrives[i].c_str());			
			pSystemDriveComboBox->Add(elem);
		}
		if (systemDrives.size() > 0)
		{
			if (systemDrives.size() == 1)
				pSystemDriveComboBox->SelectItem(0);
			else
				pSystemDriveComboBox->SelectItem(1);
		}

		CEditUI* pVolumeEdit = static_cast<CEditUI*>(m_pm.FindControl(_T("volume_edit")));
		std::string driveName(pSystemDriveComboBox->GetText().GetData());
		pVolumeEdit->SetText(GetVolumeName(driveName.substr(0, 2)).c_str());

		CEditUI* pUidEdit = static_cast<CEditUI*>(m_pm.FindControl(_T("uid_edit")));
		pUidEdit->SetText(GetUserId().c_str());


		// 显示本地已安装的软件
		CVerticalLayoutUI* pVerticalLayout = static_cast<CVerticalLayoutUI*>(m_pm.FindControl(_T("installed_software_container")));
		LoadLocalSoftwareFromJson(pVerticalLayout, "./data/installed_software.json");
		
		
    }



	std::vector<VendorInfo> PraseVendorInfo(std::string strJsonFile)
	{

		std::vector<VendorInfo>  arrVendor;

		FILE * f = fopen(strJsonFile.c_str(), "rb");
		/* 获取文件大小 */
		fseek(f, 0, SEEK_END);
		long lSize = ftell(f);
		rewind(f);

		/* 分配内存存储整个文件 */
		char * buffer = (char*)malloc(sizeof(char)*(lSize + 1));
		if (buffer == NULL)
		{
			return arrVendor;
		}
		memset(buffer, 0, lSize + 1);


		/* 将文件拷贝到buffer中 */
		long result = fread(buffer, 1, lSize, f);
		if (result != lSize)
		{
			return arrVendor;
		}

		fclose(f);

		std::string utf8Str(buffer);

		std::string asciiStr = Utf8ToAscii(utf8Str);


		// char * jsonStr = "{\"semantic\":{\"slots\":{\"name\":\"张三\"}}, \"rc\":0, \"operation\":\"CALL\", \"service\":\"telephone\", \"text\":\"打电话给张三\"}";
		const char * jsonStr = asciiStr.c_str();
		cJSON * root = NULL;
		cJSON * item = NULL;//cjson对象

		root = cJSON_Parse(jsonStr);
		if (!root)
		{
			printf("Error before: [%s]\n", cJSON_GetErrorPtr());
		}
		else
		{

			cJSON * arr = cJSON_GetObjectItem(root, "softwares");//
			int count = cJSON_GetArraySize(arr);
			for (int i = 0; i < count; i++)
			{
				cJSON * v = cJSON_GetArrayItem(arr, i);

				cJSON * nameProp = cJSON_GetObjectItem(v, "name");
				cJSON * exeArr = cJSON_GetObjectItem(v, "exeList");//

				VendorInfo vendor;
				vendor.name = nameProp->valuestring;

				int exeCount = cJSON_GetArraySize(exeArr);
				for (int j = 0; j < exeCount; j++)
				{
					cJSON * exe = cJSON_GetArrayItem(exeArr, j);

					cJSON * idProp = cJSON_GetObjectItem(exe, "id");
					cJSON * nameProp = cJSON_GetObjectItem(exe, "name");
					cJSON * iconProp = cJSON_GetObjectItem(exe, "icon");
					cJSON * osProp = cJSON_GetObjectItem(exe, "os");
					cJSON * descProp = cJSON_GetObjectItem(exe, "desc");
					cJSON * pathProp = cJSON_GetObjectItem(exe, "path");
					cJSON * argsProp = cJSON_GetObjectItem(exe, "args");

					SoftwareInfo software;
					software.id = idProp->valuestring;
					software.name = nameProp->valuestring;
					software.icon = iconProp->valuestring;
					software.os = osProp->valuestring;
					software.desc = descProp->valuestring;
					software.path = pathProp->valuestring;
					software.args = argsProp->valuestring;

					vendor.arrSoftware.push_back(software);

				}

				arrVendor.push_back(vendor);

			}
		}

		free(buffer);

		return arrVendor;
	}

	void LoadLocalSoftwareFromJson(CVerticalLayoutUI* pVerticalLayout, std::string jsonFile)
	{
		pVerticalLayout->RemoveAll();


		std::vector<VendorInfo> vendorArr = PraseVendorInfo(jsonFile);
		for (int i = 0; i < vendorArr.size(); i++){

			VendorInfo vendor = vendorArr[i];

			CContainerUI* pTileElement = NULL;
			CDialogBuilder builder;
			if (!builder.GetMarkup()->IsValid()) {
				//pTileElement = static_cast<CContainerUI*>(builder.Create(_T("test2_item.xml"), (UINT)0, NULL, &m_pm));
				pTileElement = static_cast<CContainerUI*>(builder.Create(CreateInstalledItemXml(vendor).c_str(), (UINT)0, NULL, &m_pm));
			}

			pVerticalLayout->AddAt(pTileElement, i);
		}
		
	}


	void LoadSoftwareFromJson(CTileLayoutUI* pTileLayout, std::string jsonFile)
	{
		pTileLayout->RemoveAll();

		std::vector<SoftwareInfo> softwareArr = PraseJson(jsonFile);
		for (int i = 0; i < softwareArr.size(); i++){

			SoftwareInfo software = softwareArr[i];

			CContainerUI* pTileElement = NULL;
			CDialogBuilder builder;
			if (!builder.GetMarkup()->IsValid()) {
				//pTileElement = static_cast<CContainerUI*>(builder.Create(_T("test2_item.xml"), (UINT)0, NULL, &m_pm));
				pTileElement = static_cast<CContainerUI*>(builder.Create(CreateItemXml(software.icon.c_str(), software.name.c_str(), software.os.c_str(), software.desc.c_str()).c_str(), (UINT)0, NULL, &m_pm));
			}

			pTileLayout->AddAt(pTileElement, i);
		}
	}

	bool ListFiles(std::string dir)
	{
		if (!DirIsValid(dir))
		{
			return false;
		}


		char dirNew[200];
		strcpy(dirNew, dir.c_str());
		strcat(dirNew, "\\*.*");    // 在目录后面加上"\\*.*"进行第一次搜索

		intptr_t handle;
		_finddatai64_t findData;

		handle = _findfirsti64(dirNew, &findData);
		if (handle == -1)        // 检查是否成功
			return false;

		do
		{
			if (findData.attrib & _A_SUBDIR)
			{
				if (strcmp(findData.name, ".") == 0 || strcmp(findData.name, "..") == 0)
					continue;

				cout << findData.name << "\t<dir>\n";

				// 在目录后面加上"\\"和搜索到的目录名进行下一次搜索
				strcpy(dirNew, dir.c_str());
				strcat(dirNew, "\\");
				strcat(dirNew, findData.name);

				ListFiles(dirNew);
			}
			else
			{
				cout << findData.name << "\t" << findData.size << " bytes.\n";
				UploadFileInfo fi;
				fi.name = findData.name;
				fi.dir = dir;
				fi.size = findData.size;
				g_arrUploadFiles.push_back(fi);
			}
		} while (_findnexti64(handle, &findData) == 0);

		_findclose(handle);    // 关闭搜索句柄

		return true;
	}

	bool SetContorlText(string controlName, string text)
	{
		CEditUI* pEdit = static_cast<CEditUI*>(m_pm.FindControl(controlName.c_str()));
		if (pEdit)
		{
			pEdit->SetText(text.c_str());
			return true;
		}
		return false;

	}

	bool SetContorlText(string controlName, int count)
	{
		char buf[128];
		sprintf(buf, "%d", count);
		CEditUI* pEdit = static_cast<CEditUI*>(m_pm.FindControl(controlName.c_str()));
		if (pEdit)
		{
			pEdit->SetText(buf);
			return true;
		}
		return false;

	}
	bool GetContorlText(string controlName, string& text)
	{
		CEditUI* pEdit = static_cast<CEditUI*>(m_pm.FindControl(controlName.c_str()));
		if (pEdit)
		{
			text =  pEdit->GetText();
			return true;
		}
		return false;

	}
	

    void Notify(TNotifyUI& msg)
    {
		if (msg.sType == _T("windowinit")) {
			OnPrepare();
		}
		else if (msg.sType == _T("selectchanged")){

			if (msg.pSender->GetName() == _T("Option01")) {
				CTabLayoutUI * pTab = (CTabLayoutUI*)m_pm.FindControl(_T("TabLayoutMain"));
				pTab->SelectItem(0);//1代表第二个Tab页

				CVerticalLayoutUI* pVerticalLayout = static_cast<CVerticalLayoutUI*>(m_pm.FindControl(_T("installed_software_container")));
				LoadLocalSoftwareFromJson(pVerticalLayout, "./data/installed_software.json");
			}
			else if (msg.pSender->GetName() == _T("Option02")) {
				CTabLayoutUI * pTab = (CTabLayoutUI*)m_pm.FindControl(_T("TabLayoutMain"));
				pTab->SelectItem(1);//1代表第二个Tab页

				CTileLayoutUI* pTileLayout = static_cast<CTileLayoutUI*>(m_pm.FindControl(_T("remote_software_list")));
				LoadSoftwareFromJson(pTileLayout, "./data/local_software.json");

			}
			else if (msg.pSender->GetName() == _T("Option03")) {
				CTabLayoutUI * pTab = (CTabLayoutUI*)m_pm.FindControl(_T("TabLayoutMain"));
				pTab->SelectItem(2);//2代表第三个Tab页

				

			}
			else if (msg.pSender->GetName() == _T("Option04")) {
				CTabLayoutUI * pTab = (CTabLayoutUI*)m_pm.FindControl(_T("TabLayoutMain"));
				pTab->SelectItem(3);//1代表第四个Tab页

				

			}
			else if (msg.pSender->GetName() == _T("Option05")) {
				CTabLayoutUI * pTab = (CTabLayoutUI*)m_pm.FindControl(_T("TabLayoutMain"));
				pTab->SelectItem(4);//1代表第五个Tab页



			}
			else if (msg.pSender->GetName() == _T("Option06")) {
				CTabLayoutUI * pTab = (CTabLayoutUI*)m_pm.FindControl(_T("TabLayoutMain"));
				pTab->SelectItem(5);//1代表第六个Tab页



			}
			else if (msg.pSender->GetName() == _T("Option07")) {
				CTabLayoutUI * pTab = (CTabLayoutUI*)m_pm.FindControl(_T("TabLayoutMain"));
				pTab->SelectItem(6);//1代表第七个Tab页



			}
		}
		else if (msg.sType == _T("itemselect")) {
			if (msg.pSender->GetName() == _T("system_drive_combo")) {
				CComboUI* pSystemDriveComboBox = static_cast<CComboUI*>(m_pm.FindControl(_T("system_drive_combo")));
				CEditUI* pVolumeEdit = static_cast<CEditUI*>(m_pm.FindControl(_T("volume_edit")));
				std::string driveName(pSystemDriveComboBox->GetText().GetData());
				pVolumeEdit->SetText(GetVolumeName(driveName.substr(0, 2)).c_str());
			}
		}
        else if( msg.sType == _T("click") ) {

			
            if( msg.pSender->GetName() == _T("insertimagebtn") ) {
                CRichEditUI* pRich = static_cast<CRichEditUI*>(m_pm.FindControl(_T("testrichedit")));
                if( pRich ) {
                    pRich->RemoveAll();
                }
			}
			else if (msg.pSender == m_pMinBtn) {
				SendMessage(WM_SYSCOMMAND, SC_MINIMIZE, 0); return;
			}		
			else if (strcmp(msg.pSender->GetClass(), _T("PanelUI")) == 0) {

				
				MessageBox(this->GetHWND(), msg.pSender->GetUserData(), _T("提示"), MB_OK);

				if (!PathFileExists(msg.pSender->GetUserData()))
				{
					MessageBox(this->GetHWND(), msg.pSender->GetUserData() + " 不存在！", _T("提示"), MB_OK);
				}

				ShellExecute(NULL, "open", msg.pSender->GetUserData(), NULL, NULL, SW_SHOWMAXIMIZED);
			}
			else if (msg.pSender->GetName() == _T("close_button")) {				
				COptionUI* pControl = static_cast<COptionUI*>(m_pm.FindControl(_T("hallswitch")));
				if (pControl && pControl->IsSelected() == false) {
					CControlUI* pFadeControl = m_pm.FindControl(_T("fadeEffect"));
					if (pFadeControl) pFadeControl->SetVisible(true);
				}
				else {
					/*Close()*/PostQuitMessage(0); // 因为activex的原因，使用close可能会出现错误
				}
			}
			else if (msg.pSender->GetName() == _T("login_button")) {

				OnLogin();
				
			}
            else if( msg.pSender->GetName() == _T("changeskinbtn") ) {
                if( CPaintManagerUI::GetResourcePath() == CPaintManagerUI::GetInstancePath() )
                    CPaintManagerUI::SetResourcePath(CPaintManagerUI::GetInstancePath() + _T("skin\\FlashRes"));
                else
                    CPaintManagerUI::SetResourcePath(CPaintManagerUI::GetInstancePath());
                CPaintManagerUI::ReloadSkin();

				
            }		
			else if (msg.pSender->GetName() == _T("refresh_installed_software_button")){
				CVerticalLayoutUI* pVerticalLayout = static_cast<CVerticalLayoutUI*>(m_pm.FindControl(_T("installed_software_container")));
				LoadLocalSoftwareFromJson(pVerticalLayout, "./data/installed_software.json");

				//MessageBox(NULL, _T("加载所有软件！"), _T("提示"), MB_OK);
			}
			else if (msg.pSender->GetName() == _T("load_all_button")){
				CTileLayoutUI* pTileLayout = static_cast<CTileLayoutUI*>(m_pm.FindControl(_T("remote_software_list")));
				LoadSoftwareFromJson(pTileLayout, "./data/local_software.json");			

				//MessageBox(NULL, _T("加载所有软件！"), _T("提示"), MB_OK);
			}
			else if (msg.pSender->GetName() == _T("load_office_button")){
				CTileLayoutUI* pTileLayout = static_cast<CTileLayoutUI*>(m_pm.FindControl(_T("remote_software_list")));
				LoadSoftwareFromJson(pTileLayout, "./data/local_software_office.json");

				//MessageBox(NULL, _T("加载办公软件！"), _T("提示"), MB_OK);
			}
			else if (msg.pSender->GetName() == _T("load_music_button")){
				CTileLayoutUI* pTileLayout = static_cast<CTileLayoutUI*>(m_pm.FindControl(_T("remote_software_list")));
				LoadSoftwareFromJson(pTileLayout, "./data/local_software_music.json");

				//MessageBox(NULL, _T("加载音乐软件！"), _T("提示"), MB_OK);
			}
			else if (msg.pSender->GetName() == _T("load_other_button")){
				CTileLayoutUI* pTileLayout = static_cast<CTileLayoutUI*>(m_pm.FindControl(_T("remote_software_list")));
				LoadSoftwareFromJson(pTileLayout, "./data/local_software_other.json");

				//MessageBox(NULL, _T("加载其他软件！"), _T("提示"), MB_OK);
			}
			else if (msg.pSender->GetName() == _T("sec_load_button")){

				bool bRet = SecLoad(g_strWorkDir + "sys\\SecMFDock.sys");
				if (bRet){
					MessageBox(NULL, _T("加载文件驱动结束！"), _T("提示"), MB_OK);
				}
				
				bRet = SecLoad(g_strWorkDir + "sys\\SecMKDock.sys");
				if (bRet){
					MessageBox(NULL, _T("加载注册表驱动结束！"), _T("提示"), MB_OK);
				}
				MessageBox(NULL, _T("加载驱动结束！"), _T("提示"), MB_OK);
			}
			else if (msg.pSender->GetName() == _T("sec_init_button")){

				CComboUI* pSystemDriveComboBox = static_cast<CComboUI*>(m_pm.FindControl(_T("system_drive_combo")));
				std::string driveName(pSystemDriveComboBox->GetText().GetData());

				CEditUI* pVolumeNameEdit = static_cast<CEditUI*>(m_pm.FindControl(_T("volume_edit")));
				CEditUI* pSidNameEdit = static_cast<CEditUI*>(m_pm.FindControl(_T("uid_edit")));				
				
				const char* volumeName = pVolumeNameEdit->GetText().GetData();
				const char* sidName = pSidNameEdit->GetText().GetData();
				SecInit(driveName.substr(0, 2), volumeName, sidName);

				//MessageBox(NULL, _T("操作结束！"), _T("提示"), MB_OK);
			}
			else if (msg.pSender->GetName() == _T("sec_fsd_module_button")){
				CComboUI* pSystemDriveComboBox = static_cast<CComboUI*>(m_pm.FindControl(_T("system_drive_combo")));
				std::string driveName(pSystemDriveComboBox->GetText().GetData());

				CEditUI* pVolumeNameEdit = static_cast<CEditUI*>(m_pm.FindControl(_T("volume_edit")));
				CEditUI* pSidNameEdit = static_cast<CEditUI*>(m_pm.FindControl(_T("uid_edit")));

				const char* volumeName = pVolumeNameEdit->GetText().GetData();
				const char* sidName = pSidNameEdit->GetText().GetData();

				SecFsdModule(volumeName);
				MessageBox(NULL, _T("文件模块操作结束！"), _T("提示"), MB_OK);
			}
			else if (msg.pSender->GetName() == _T("sec_regkey_module_button")){

				CComboUI* pSystemDriveComboBox = static_cast<CComboUI*>(m_pm.FindControl(_T("system_drive_combo")));
				std::string driveName(pSystemDriveComboBox->GetText().GetData());

				CEditUI* pVolumeNameEdit = static_cast<CEditUI*>(m_pm.FindControl(_T("volume_edit")));
				CEditUI* pSidNameEdit = static_cast<CEditUI*>(m_pm.FindControl(_T("uid_edit")));

				const char* volumeName = pVolumeNameEdit->GetText().GetData();
				const char* sidName = pSidNameEdit->GetText().GetData();

				SecRegkeyModule(driveName.substr(0, 2), sidName);
				MessageBox(NULL, _T("注册表模块操作结束！"), _T("提示"), MB_OK);
			}
			else if (msg.pSender->GetName() == _T("sec_desktop_mode_button")){

				SecDesktopMode();
				MessageBox(NULL, _T("桌面模式操作结束！"), _T("提示"), MB_OK);
			}
			else if (msg.pSender->GetName() == _T("sec_use_mode_button")){
				CComboUI* pSystemDriveComboBox = static_cast<CComboUI*>(m_pm.FindControl(_T("system_drive_combo")));
				std::string driveName(pSystemDriveComboBox->GetText().GetData());

				CEditUI* pVolumeNameEdit = static_cast<CEditUI*>(m_pm.FindControl(_T("volume_edit")));
				CEditUI* pSidNameEdit = static_cast<CEditUI*>(m_pm.FindControl(_T("uid_edit")));

				const char* volumeName = pVolumeNameEdit->GetText().GetData();
				const char* sidName = pSidNameEdit->GetText().GetData();

				SecUseMode(driveName.substr(0, 2), volumeName, sidName);
				MessageBox(NULL, _T("使用模式操作结束！"), _T("提示"), MB_OK);
			}
			else if (msg.pSender->GetName() == _T("sec_shutdown_mode_button")){

				SecShutdownMode();
				MessageBox(NULL, _T("注销模式操作结束！"), _T("提示"), MB_OK);
			}
			else if (msg.pSender->GetName() == _T("sec_finish_button")){

				SecFinish();
				MessageBox(NULL, _T("制作完成操作结束！"), _T("提示"), MB_OK);
			}
			else if (msg.pSender->GetName() == _T("get_icon_button")){
				
				CEditUI* pEdit = static_cast<CEditUI*>(m_pm.FindControl(_T("exe_file_path_edit")));
				CEditUI* pIconDirEdit = static_cast<CEditUI*>(m_pm.FindControl(_T("icon_dir_edit")));

				const char* exePath = pEdit->GetText().GetData();//_T("C:\\Program Files (x86)\\Tencent\\QQ\\Bin\\QQ.exe");
				const char* iconDir = pIconDirEdit->GetText().GetData();
				if (!PathFileExistsA(exePath))
				{
					
					MessageBox(NULL, _T("指定的EXE文件不存在！"), _T("提示"), MB_OK);
					return;
				}

				if (!PathFileExistsA(iconDir))
				{
					MessageBox(NULL, _T("指定的文件夹不存在！"), _T("提示"), MB_OK);
					return;
				}

				char fname[256];
				_splitpath(exePath, NULL, NULL, fname, NULL);
				char newIconDir[1024];
				sprintf(newIconDir, _T("%s\\%s"), iconDir, fname);
				CreateDirectory(newIconDir, NULL);
				
				MessageBox(NULL, exePath, _T("可执行程序路径"), MB_OK);
				MessageBox(NULL, iconDir, _T("图标存放目录"), MB_OK);

				CButtonUI* pButton = static_cast<CButtonUI*>(m_pm.FindControl(_T("get_icon_button")));

				ENUMRESOURCENAMESPARAMS params;
				memset(params.iconDir, 0, 1024);
				memcpy(params.iconDir, newIconDir, strlen(newIconDir));

				idx = 0;
				HMODULE hModule = LoadLibraryExA(exePath, NULL, LOAD_LIBRARY_AS_DATAFILE);//::LoadLibraryA(appName.toLocal8Bit().data());
				EnumResourceNamesA(hModule, RT_ICON, (ENUMRESNAMEPROCA)UpdateIcons, (long)&params);
				::FreeLibrary(hModule);

				MessageBox(NULL, _T("完成"), _T("导出图标"), MB_OK);
			}
			else if (msg.pSender->GetName() == _T("get_packageid_button")){
				MessageBox(NULL, _T("开始获取！"), _T("提示"), MB_OK);

				string serverurl = "";
				GetContorlText("serverurl_edit", serverurl);


				string packagename = "";
				GetContorlText("packagename_edit", packagename);

				if (packagename.length() == 0)
				{
					MessageBox(NULL, _T("请输入packagename！"), _T("提示"), MB_OK);
					return;
				}

				CURL *curl;
				CURLcode res;

				curl = curl_easy_init();
				if (!curl)
				{
					MessageBox(NULL, _T("curl_easy_init 失败！"), _T("提示"), MB_OK);
				}

				curl_easy_setopt(curl, CURLOPT_POST, 1); // post req  
				curl_easy_setopt(curl, CURLOPT_URL, serverurl + "/client/dockpackage/new");

				string fmt = "dp_name=%s" \
					"&dp_desc=商业分析软件最新版本！" \
					"&os_name=%s" \
					"&os_version=%d.%d.%d" \
					"&os_buildnumber=%d" \
					"&os_arch=%d" \
					"&vendor_id=UDKS8923SD" \
					"&vendor_name=商业分析有限公司" \
					"&product_id=DIU839EW" \
					"&product_name=商业分析系统" \
					"&installer_name=abc.exe" \
					"&installer_version=1.0.435" \
					"&free_flag=1";

				char requestbuf[1024];
				memset(requestbuf, 0, 1024);
				sprintf(requestbuf, fmt.c_str(), packagename.c_str(), g_strWindowsName.c_str(), g_dwMajorVersion, g_dwMinorVersion, g_dwBuildNumber, g_dwBuildNumber, g_dwProcessorArchitecture);
				

				string utf8post = AsciiToUtf8(requestbuf);
				curl_easy_setopt(curl, CURLOPT_POSTFIELDS, utf8post.c_str()); // params  

				curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, req_reply);
				string response;
				curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&response);
				curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);

				struct curl_slist *head = NULL;
				head = curl_slist_append(head, "Content-Type:application/x-www-form-urlencoded;charset=UTF-8");
				curl_easy_setopt(curl, CURLOPT_HTTPHEADER, head);
			
				curl_easy_setopt(curl, CURLOPT_HEADER, 0);
				curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3);
				curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3);

				
				res = curl_easy_perform(curl);

				string packageid = "";
				if (res != CURLE_OK){
					MessageBox(NULL, _T("获取packageid过程中遇到问题！！"), _T("提示"), MB_OK);
					fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
				}
				else
				{					
					packageid = PrasePackageId(response);
					MessageBox(NULL, packageid.c_str(), _T("提示"), MB_OK);
					SetContorlText("packageid_edit", packageid);
				}

				
				g_arrUploadFiles.clear();

				string packageDir = "";
				GetContorlText("package_dir_edit", packageDir);

				if (packageDir.length() == 0)
				{
					MessageBox(NULL, "packagedir为空！", _T("提示"), MB_OK);
					return;
				}

				if (!DirIsValid(packageDir))
				{
					MessageBox(NULL, "packagedir不存在！", _T("提示"), MB_OK);
					return;
				}

				ListFiles(packageDir);

				SetContorlText("package_file_count_edit", g_arrUploadFiles.size());

/*
				curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:9999/client/dockpackage/file/new");

				char buf[1024];
				for (int i = 0; i < g_arrUploadFiles.size(); i++)
				{
					std::string filedir = g_arrUploadFiles[i].dir;					
					string filename = g_arrUploadFiles[i].name;
					memset(buf, 0, 1024);
					sprintf(buf, ("fileno=%d&packageid=%s&filepath=%s&filename=%s&fileext=%s&filesize=%d"), i, packageid, filedir.c_str(), filename.c_str(), "", g_arrUploadFiles[i].size);
					string postParams(buf);

					string utf8str =  AsciiToUtf8(buf);

					curl_easy_setopt(curl, CURLOPT_POSTFIELDS, utf8str.c_str()); // params  

					res = curl_easy_perform(curl);

					if (res != CURLE_OK)
					{
						MessageBox(NULL, _T("上传过程遇到问题！！"), _T("提示"), MB_OK);
						break;
					}
				}
				*/

				curl_slist_free_all(head);//记得要释放

				curl_easy_cleanup(curl);


				MessageBox(NULL, _T("获取成功！"), _T("提示"), MB_OK);
			}
			else if (msg.pSender->GetName() == _T("upload_button")){				
				MessageBox(NULL, _T("开始上传！"), _T("提示"), MB_OK);

				string packageid = "";
				GetContorlText("packageid_edit", packageid);

				if (packageid.length() == 0)
				{
					MessageBox(NULL, "packageid为空！", _T("提示"), MB_OK);
					return;
				}

				g_arrUploadFiles.clear();
				string packageDir = "";
				GetContorlText("package_dir_edit", packageDir);

				if (packageDir.length() == 0)
				{
					MessageBox(NULL, "packagedir为空！", _T("提示"), MB_OK);
					return;
				}

				if (!DirIsValid(packageDir))
				{
					MessageBox(NULL, "packagedir不存在！", _T("提示"), MB_OK);
					return;
				}

				ListFiles(packageDir);

				SetContorlText("package_file_count_edit", g_arrUploadFiles.size());


				ObjectSample objectSample(g_strBucketName);

				string indexfilecontent = "";
				std::shared_ptr<std::stringstream> indexfilestream = std::make_shared<std::stringstream>();
				*indexfilestream << "{\"filecount\":" << g_arrUploadFiles.size() << ",\"filelist\":[";
				for (int i = 0; i < g_arrUploadFiles.size() - 1; i++)
				{					
					string tmpdir = g_arrUploadFiles[i].dir.substr(packageDir.length());
					*indexfilestream << "{" << "\"fileno\":\"" << i << "\"," << "\"filedir\":\"" << tmpdir << "\"," << "\"filename\":\"" << g_arrUploadFiles[i].name << "\"" << "},";
				}
				string tmpdir = g_arrUploadFiles[g_arrUploadFiles.size() - 1].dir.substr(packageDir.length());
				*indexfilestream << "{"
					<< "\"fileno\":\"" << (g_arrUploadFiles.size() - 1) << "\","
					<< "\"filedir\":\"" << tmpdir << "\","
					<< "\"filename\":\"" << g_arrUploadFiles[g_arrUploadFiles.size() - 1].name << "\""
					<< "}";
				*indexfilestream << "]}";

				indexfilecontent = indexfilestream->str();

				objectSample.PutObjectFromBuffer(packageid, indexfilecontent);

				char buf[128];
				for (int i = 0; i < g_arrUploadFiles.size(); i++)				
				{
					_stprintf(buf, "%s/%d", packageid.c_str(), i);
					objectSample.PutObjectFromFile(buf, g_arrUploadFiles[i].dir + "\\" + g_arrUploadFiles[i].name);
				}

				MessageBox(NULL, _T("上传成功！"), _T("提示"), MB_OK);
			}
			else if (msg.pSender->GetName() == _T("test_download_pkg_button")){
				MessageBox(NULL, _T("开始下载！"), _T("提示"), MB_OK);

				DWORD t1 = ::GetTickCount();

				string packageid = "";
				GetContorlText("packageid_edit", packageid);

				if (packageid.length() == 0)
				{
					MessageBox(NULL, "packageid为空！", _T("提示"), MB_OK);
					return;
				}


				string downloaddir = "";
				GetContorlText("download_dir_edit", downloaddir);

				if (downloaddir.length() == 0)
				{
					MessageBox(NULL, "downloaddir为空！", _T("提示"), MB_OK);
					return;
				}

				if (!DirIsValid(downloaddir))
				{
					MessageBox(NULL, "downloaddir不存在！", _T("提示"), MB_OK);
					return;
				}

				string packagefileinfo = "";

				ObjectSample objectSample(g_strBucketName);

				objectSample.GetObjectToFile(packageid, downloaddir + "\\" + packageid + ".json");

				objectSample.GetObjectToBuffer(packageid, packagefileinfo);

				std::replace(packagefileinfo.begin(), packagefileinfo.end(), '\\', '=');

				std::vector<UploadFileInfo> fileList = PrasePackageFileInfo(packagefileinfo);
				for (int i = 0; i < fileList.size(); i++)
				{					
					string filedir = fileList[i].dir;
					std::replace(filedir.begin(), filedir.end(), '=', '\\');

					string fulldir = downloaddir + "\\" + packageid + filedir;
					CreateDir(fulldir.c_str());
					objectSample.GetObjectToFile(packageid + "/" + fileList[i].uuid, fulldir + "\\" + fileList[i].name);
				}

				DWORD t2 = ::GetTickCount();

				char buf[1024];
				sprintf(buf, "下载完成，共耗时 %d.%d 秒", (t2 - t1) / 1000, (t2 - t1) % 1000);

				MessageBox(NULL, buf, _T("提示"), MB_OK);
			}
			else if (msg.pSender->GetName() == _T("open_calc_button")){
				system("start calc");
			}
			else if (msg.pSender->GetName() == _T("open_notepad_button")){
				system("start notepad");
			}
			else if (msg.pSender->GetName() == _T("open_mspaint_button")){
				system("start mspaint");
			}
			else if (msg.pSender->GetName() == _T("open_explorer_button")){
				system("start explorer");
			}
        }
    }

	// 登录
	void OnLogin()
	{
		CLoginFrameWnd* pLoginFrame = new CLoginFrameWnd();
		if (pLoginFrame == NULL) { Close(); return; }
		pLoginFrame->Create(m_hWnd, _T(""), UI_WNDSTYLE_DIALOG, 0, 0, 0, 0, 0, NULL);
		//pLoginFrame->SetIcon(IDI_ICON_DUILIB);
		pLoginFrame->CenterWindow();
		pLoginFrame->ShowModal();
	}

	// 创建桌面快捷方式
	void OnCreateDescktopIcon()
	{
		HRESULT hr = CoInitialize(NULL);
		if (SUCCEEDED(hr))
		{
			IShellLink *pisl;
			hr = CoCreateInstance(CLSID_ShellLink, NULL,
				CLSCTX_INPROC_SERVER, IID_IShellLink, (void**)&pisl);
			if (SUCCEEDED(hr))
			{
				IPersistFile* pIPF;


				/////////////////////////////////////////////////////////////////////////////////////////////////////////////


				//这里是我们要创建快捷方式的原始文件地址
				pisl->SetPath("F:\\github\\duilib\\bin\\TestApp2_d.exe");

				pisl->SetArguments("start 2019");
				pisl->SetIconLocation("F:\\github\\duilib\\bin\\IconFromPE\\QQScLauncher\\5_48x48.ico", 0);

				hr = pisl->QueryInterface(IID_IPersistFile, (void**)&pIPF);
				if (SUCCEEDED(hr))
				{

					/////////////////////////////////////////////////////////////////////////////////////////////////////////////

					//这里是我们要创建快捷方式的目标地址

					pIPF->Save(L"C:\\Users\\hanxi\\Desktop\\START QQ.lnk", FALSE);
					pIPF->Release();
				}
				pisl->Release();
			}
			CoUninitialize();
		}
	}

	LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		bHandled = FALSE;
		return 0;
	}

	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		SIZE szRoundCorner = m_pm.GetRoundCorner();
		if (!::IsIconic(*this) && (szRoundCorner.cx != 0 || szRoundCorner.cy != 0)) {
			CDuiRect rcWnd;
			::GetWindowRect(*this, &rcWnd);
			rcWnd.Offset(-rcWnd.left, -rcWnd.top);
			rcWnd.right++; rcWnd.bottom++;
			HRGN hRgn = ::CreateRoundRectRgn(rcWnd.left, rcWnd.top, rcWnd.right, rcWnd.bottom, szRoundCorner.cx, szRoundCorner.cy);
			::SetWindowRgn(*this, hRgn, TRUE);
			::DeleteObject(hRgn);
		}

		bHandled = FALSE;
		return 0;
	}

	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		::PostQuitMessage(0L);

		bHandled = FALSE;
		return 0;
	}

	LRESULT OnNcActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if (::IsIconic(*this)) bHandled = FALSE;
		return (wParam == 0) ? TRUE : FALSE;
	}

	LRESULT OnNcCalcSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		return 0;
	}

	LRESULT OnNcPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		return 0;
	}

	LRESULT OnNcHitTest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		POINT pt; pt.x = GET_X_LPARAM(lParam); pt.y = GET_Y_LPARAM(lParam);
		::ScreenToClient(*this, &pt);

		RECT rcClient;
		::GetClientRect(*this, &rcClient);

		if (!::IsZoomed(*this)) {
			RECT rcSizeBox = m_pm.GetSizeBox();
			if (pt.y < rcClient.top + rcSizeBox.top) {
				if (pt.x < rcClient.left + rcSizeBox.left) return HTTOPLEFT;
				if (pt.x > rcClient.right - rcSizeBox.right) return HTTOPRIGHT;
				return HTTOP;
			}
			else if (pt.y > rcClient.bottom - rcSizeBox.bottom) {
				if (pt.x < rcClient.left + rcSizeBox.left) return HTBOTTOMLEFT;
				if (pt.x > rcClient.right - rcSizeBox.right) return HTBOTTOMRIGHT;
				return HTBOTTOM;
			}
			if (pt.x < rcClient.left + rcSizeBox.left) return HTLEFT;
			if (pt.x > rcClient.right - rcSizeBox.right) return HTRIGHT;
		}

		RECT rcCaption = m_pm.GetCaptionRect();
		if (pt.x >= rcClient.left + rcCaption.left && pt.x < rcClient.right - rcCaption.right \
			&& pt.y >= rcCaption.top && pt.y < rcCaption.bottom) {
			CControlUI* pControl = static_cast<CControlUI*>(m_pm.FindControl(pt));
			if (pControl && _tcscmp(pControl->GetClass(), DUI_CTR_BUTTON) != 0 &&
				_tcscmp(pControl->GetClass(), DUI_CTR_OPTION) != 0 &&
				_tcscmp(pControl->GetClass(), DUI_CTR_TEXT) != 0)
				return HTCAPTION;
		}

		return HTCLIENT;
	}

	LRESULT OnGetMinMaxInfo(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		int primaryMonitorWidth = ::GetSystemMetrics(SM_CXSCREEN);
		int primaryMonitorHeight = ::GetSystemMetrics(SM_CYSCREEN);
		MONITORINFO oMonitor = {};
		oMonitor.cbSize = sizeof(oMonitor);
		::GetMonitorInfo(::MonitorFromWindow(*this, MONITOR_DEFAULTTOPRIMARY), &oMonitor);
		CDuiRect rcWork = oMonitor.rcWork;
		rcWork.Offset(-oMonitor.rcMonitor.left, -oMonitor.rcMonitor.top);
		if (rcWork.right > primaryMonitorWidth) rcWork.right = primaryMonitorWidth;
		if (rcWork.bottom > primaryMonitorHeight) rcWork.right = primaryMonitorHeight;
		LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;
		lpMMI->ptMaxPosition.x = rcWork.left;
		lpMMI->ptMaxPosition.y = rcWork.top;
		lpMMI->ptMaxSize.x = rcWork.right;
		lpMMI->ptMaxSize.y = rcWork.bottom;
		bHandled = FALSE;
		return 0;
	}

	LRESULT OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		// 有时会在收到WM_NCDESTROY后收到wParam为SC_CLOSE的WM_SYSCOMMAND
		if (wParam == SC_CLOSE) {
			::PostQuitMessage(0L);
			bHandled = TRUE;
			return 0;
		}
		BOOL bZoomed = ::IsZoomed(*this);
		LRESULT lRes = CWindowWnd::HandleMessage(uMsg, wParam, lParam);
		if (::IsZoomed(*this) != bZoomed) {
			if (!bZoomed) {
				CControlUI* pControl = static_cast<CControlUI*>(m_pm.FindControl(_T("maxbtn")));
				if (pControl) pControl->SetVisible(false);
				pControl = static_cast<CControlUI*>(m_pm.FindControl(_T("restorebtn")));
				if (pControl) pControl->SetVisible(true);
			}
			else {
				CControlUI* pControl = static_cast<CControlUI*>(m_pm.FindControl(_T("maxbtn")));
				if (pControl) pControl->SetVisible(true);
				pControl = static_cast<CControlUI*>(m_pm.FindControl(_T("restorebtn")));
				if (pControl) pControl->SetVisible(false);
			}
		}
		return lRes;
	}

    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
		LRESULT lRes = 0;
		BOOL bHandled = TRUE;

        if( uMsg == WM_CREATE ) {

			// 去掉WINDOWS的外框
			LONG styleValue = ::GetWindowLong(*this, GWL_STYLE);
			styleValue &= ~WS_CAPTION;
			::SetWindowLong(*this, GWL_STYLE, styleValue | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);

            m_pm.Init(m_hWnd);
            CDialogBuilder builder;
            CControlUI* pRoot = builder.Create(_T("./skin/YouziRes/main.xml"), (UINT)0, NULL, &m_pm);
            ASSERT(pRoot && "Failed to parse XML");
            m_pm.AttachDialog(pRoot);
            m_pm.AddNotifier(this);

            m_pWndShadow = new CWndShadow;
            m_pWndShadow->Create(m_hWnd);
            RECT rcCorner = {3,3,4,4};
            RECT rcHoleOffset = {0,0,0,0};
            m_pWndShadow->SetImage(_T("LeftWithFill.png"), rcCorner, rcHoleOffset);

            DWMNCRENDERINGPOLICY ncrp = DWMNCRP_ENABLED;
            SetWindowAttribute(m_hWnd, DWMWA_TRANSITIONS_FORCEDISABLED, &ncrp, sizeof(ncrp));

            //DWM_BLURBEHIND bb = {0};
            //bb.dwFlags = DWM_BB_ENABLE;
            //bb.fEnable = true;
            //bb.hRgnBlur = NULL;
            //EnableBlurBehindWindow(m_hWnd, bb);

            //DWM_MARGINS margins = {-1}/*{0,0,0,25}*/;
            //ExtendFrameIntoClientArea(m_hWnd, margins);

            Init();
            return 0;
        }       
		else
		{
			
			
			switch (uMsg) {
			case WM_CLOSE:         lRes = OnClose(uMsg, wParam, lParam, bHandled); break;
			case WM_DESTROY:       lRes = OnDestroy(uMsg, wParam, lParam, bHandled); break;
			case WM_NCACTIVATE:    lRes = OnNcActivate(uMsg, wParam, lParam, bHandled); break;
			case WM_NCCALCSIZE:    lRes = OnNcCalcSize(uMsg, wParam, lParam, bHandled); break;
			case WM_NCPAINT:       lRes = OnNcPaint(uMsg, wParam, lParam, bHandled); break;
			case WM_NCHITTEST:     lRes = OnNcHitTest(uMsg, wParam, lParam, bHandled); break;
			case WM_MOUSEHOVER:     lRes = OnMouseHover(uMsg, wParam, lParam, bHandled); break;
			case WM_SIZE:          lRes = OnSize(uMsg, wParam, lParam, bHandled); break;
			case WM_GETMINMAXINFO: lRes = OnGetMinMaxInfo(uMsg, wParam, lParam, bHandled); break;
			case WM_SYSCOMMAND:    lRes = OnSysCommand(uMsg, wParam, lParam, bHandled); break;
			default:
				bHandled = FALSE;
			}
		}

		if (bHandled){
			return lRes;
		}
        
		if (m_pm.MessageHandler(uMsg, wParam, lParam, lRes))
		{
			return lRes;
		}
        return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
    }

	LRESULT OnMouseHover(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
		CControlUI* pHover = m_pm.FindControl(pt);
		if (pHover == NULL) return 0;
		/*演示悬停在下载列表的图标上时，动态变换下载图标状态显示*/
		if (pHover->GetUserData() == _T("this_is_can_hover"))
		{
			//MessageBox(NULL, _T("鼠标在某控件例如按钮上悬停后，对目标控件操作，这里改变了状态图标大小"), _T("DUILIB DEMO"), MB_OK);
			//((CButtonUI *)pHover)->ApplyAttributeList( _T("normalimage=\"file='downlist_pause.png' dest='15,9,32,26'\""));

			DWORD color = 0xffff0000;
			pHover->SetBkColor(color);
		}
		return 0;
	}

public:
    CPaintManagerUI m_pm;
    CWndShadow* m_pWndShadow;

public:
	CButtonUI* m_pCloseBtn;
	CButtonUI* m_pMaxBtn;
	CButtonUI* m_pRestoreBtn;
	CButtonUI* m_pMinBtn;
};

void LogCallbackFunc(LogLevel level, const std::string &stream)
{
	if (level == LogLevel::LogOff)
		return;

	std::cout << stream;
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	// 处理命令行参数
	LPWSTR *szArgList;
	int argCount;

	szArgList = CommandLineToArgvW(GetCommandLineW(), &argCount);
	if (szArgList == NULL)
	{
		MessageBoxW(NULL, L"Unable to parse command line", L"Error", MB_OK);
		return 10;
	}

	if (argCount == 3){

		for (int i = 0; i < argCount; i++)
		{
			MessageBoxW(NULL, szArgList[i], L"Arglist contents", MB_OK);
		}

		return 0;
	}
	
	g_strWorkDir = "";

	WCHAR szCurrentDir[MAX_PATH * 2] = { 0 };
	GetModuleFileNameW(NULL, szCurrentDir, sizeof(szCurrentDir));

	string tmpCurDir = UnicodeToUtf8(szCurrentDir);
	g_strWorkDir = tmpCurDir.substr(0, tmpCurDir.rfind('\\') + 1);

	//MessageBox(NULL, g_strWorkDir.c_str(), _T("Arglist contents"), MB_OK);

	// 获取操作系统信息
	getOsInfo();

	
	BOOL ret = GetNtVersionNumbers(g_dwMajorVersion, g_dwMinorVersion,g_dwBuildNumber);

	
	ret = GetOperatingSystemName(g_strWindowsName, g_dwProcessorArchitecture);

	// OSS
	InitializeSdk();
	SetLogLevel(LogLevel::LogDebug);
	SetLogCallback(LogCallbackFunc);
	g_strBucketName = "ztsc666";

    CPaintManagerUI::SetInstance(hInstance);
    CPaintManagerUI::SetResourcePath(CPaintManagerUI::GetInstancePath());

    HRESULT Hr = ::CoInitialize(NULL);
    if( FAILED(Hr) ) return 0;

    CWndShadow::Initialize(hInstance);

    CFrameWindowWnd* pFrame = new CFrameWindowWnd();
    if( pFrame == NULL ) return 0;
    pFrame->Create(NULL, _T("这是一个最简单的测试用exe，修改test1.xml就可以看到效果1000"), UI_WNDSTYLE_FRAME|WS_CLIPCHILDREN, WS_EX_WINDOWEDGE);
    pFrame->CenterWindow();
    pFrame->ShowWindow(true);
    CPaintManagerUI::MessageLoop();

	ShutdownSdk();

    ::CoUninitialize();
    return 0;
}
