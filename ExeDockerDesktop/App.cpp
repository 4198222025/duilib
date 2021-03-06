﻿// App.cpp : Defines the entry point for the application.
//

#include "stdafx.h"

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


class CFrameWindowWnd : public CWindowWnd, public INotifyUI, public CDwm, public CDPI
{
public:
    CFrameWindowWnd() : m_pWndShadow(NULL) { };
    LPCTSTR GetWindowClassName() const { return _T("UIMainFrame"); };
    UINT GetClassStyle() const { return UI_CLASSSTYLE_FRAME | CS_DBLCLKS; };
    void OnFinalMessage(HWND /*hWnd*/) { delete this; };

    void Init()
	{ 
		m_pm.SetOpacity(240);

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
    }

    void Notify(TNotifyUI& msg)
    {
        if( msg.sType == _T("windowinit") ) OnPrepare();
        else if( msg.sType == _T("click") ) {
            if( msg.pSender->GetName() == _T("insertimagebtn") ) {
                CRichEditUI* pRich = static_cast<CRichEditUI*>(m_pm.FindControl(_T("testrichedit")));
                if( pRich ) {
                    pRich->RemoveAll();
                }
            }
			else if (strcmp(msg.pSender->GetClass(), _T("ShortcutUI")) == 0) {


				MessageBox(this->GetHWND(), msg.pSender->GetUserData(), _T("提示"), MB_OK);

				/*if (!PathFileExists(msg.pSender->GetUserData()))
				{
					MessageBox(this->GetHWND(), msg.pSender->GetUserData() + " 不存在！", _T("提示"), MB_OK);
				}

				ShellExecute(NULL, "open", msg.pSender->GetUserData(), NULL, NULL, SW_SHOWMAXIMIZED);*/
			}
            else if( msg.pSender->GetName() == _T("changeskinbtn") ) {
                if( CPaintManagerUI::GetResourcePath() == CPaintManagerUI::GetInstancePath() )
                    CPaintManagerUI::SetResourcePath(CPaintManagerUI::GetInstancePath() + _T("skin\\FlashRes"));
                else
                    CPaintManagerUI::SetResourcePath(CPaintManagerUI::GetInstancePath());
                CPaintManagerUI::ReloadSkin();
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
        }
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
            CControlUI* pRoot = builder.Create(_T("ExeDockerDesktop.xml"), (UINT)0, NULL, &m_pm);
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
			//case WM_CLOSE:         lRes = OnClose(uMsg, wParam, lParam, bHandled); break;
			case WM_DESTROY:       lRes = OnDestroy(uMsg, wParam, lParam, bHandled); break;
			case WM_NCACTIVATE:    lRes = OnNcActivate(uMsg, wParam, lParam, bHandled); break;
			case WM_NCCALCSIZE:    lRes = OnNcCalcSize(uMsg, wParam, lParam, bHandled); break;
			//case WM_NCPAINT:       lRes = OnNcPaint(uMsg, wParam, lParam, bHandled); break;
			//case WM_NCHITTEST:     lRes = OnNcHitTest(uMsg, wParam, lParam, bHandled); break;
			//case WM_MOUSEHOVER:     lRes = OnMouseHover(uMsg, wParam, lParam, bHandled); break;
			//case WM_SIZE:          lRes = OnSize(uMsg, wParam, lParam, bHandled); break;
			//case WM_GETMINMAXINFO: lRes = OnGetMinMaxInfo(uMsg, wParam, lParam, bHandled); break;
			//case WM_SYSCOMMAND:    lRes = OnSysCommand(uMsg, wParam, lParam, bHandled); break;
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

public:
    CPaintManagerUI m_pm;
    CWndShadow* m_pWndShadow;
};

static HWND FindDTWindow()
{
	HWND hWnd = ::FindWindow(_T("Progman"), _T("Program Manager"));
	static DWORD dResult = 0;

	SendMessageTimeout(hWnd, 0x052C, 0, NULL, SMTO_NORMAL, 1000, (PDWORD_PTR)&dResult);

	HWND hwndWorkW = NULL;
	do
	{
		hwndWorkW = ::FindWindowEx(NULL, hwndWorkW, _T("WorkerW"), NULL);
		if (NULL == hwndWorkW)
		{
			continue;
		}

		HWND hView = ::FindWindowEx(hwndWorkW, NULL, _T("SHELLDLL_DefView"), NULL);
		if (NULL == hView)
		{
			continue;
		}

		HWND h = ::FindWindowEx(NULL, hwndWorkW, _T("WorkerW"), NULL);
		while (NULL != h)
		{
			SendMessage(h, WM_CLOSE, 0, 0);
			h = ::FindWindowEx(NULL, hwndWorkW, _T("WorkerW"), NULL);
		}
		break;

	} while (true);

	return hWnd;

}

static void DockWindow(CFrameWindowWnd* pFrame)
{
	HWND m_hWnd = pFrame->GetHWND();

	HWND hWndProgram = NULL;
	HWND hWndShellDLL = NULL;

	hWndProgram = ::FindWindow(_T("Progman"), _T("Program Manager"));
	if (hWndProgram != NULL)
	{
		hWndShellDLL = FindWindowEx(hWndProgram, NULL, _T("SHELLDLL_DefView"), NULL);
	}

	if (hWndShellDLL == NULL)
	{
		hWndProgram = FindWindowEx(NULL, NULL, "WorkerW", NULL);

		while (hWndProgram != NULL)
		{			

			if (::GetWindowLong(hWndProgram, GWL_STYLE) & WS_VISIBLE)
			{
				break;
			}

			hWndProgram = FindWindowEx(NULL, hWndProgram, "WorkerW", NULL);
		}

		hWndShellDLL = FindWindowEx(hWndProgram, NULL, "SHELLDLL_DefView", NULL);		
	}


	if (hWndShellDLL != NULL	&& hWndShellDLL != ::GetParent(m_hWnd))
	{
		::SetWindowLong(m_hWnd, GWLP_HWNDPARENT, (LONG)hWndShellDLL);
	}

	HWND hDesktop = FindDTWindow();

	if (hDesktop != NULL	&& hDesktop != ::GetParent(m_hWnd))
	{
		::SetWindowLong(m_hWnd, GWLP_HWNDPARENT, (LONG)hDesktop);
	}

	
	ASSERT(::IsWindow(pFrame->GetHWND()));
	ASSERT((GetWindowStyle(pFrame->GetHWND())&WS_CHILD) == 0);
	RECT rcDlg = { 0 };
	::GetWindowRect(pFrame->GetHWND(), &rcDlg);
	RECT rcArea = { 0 };
	RECT rcCenter = { 0 };
	HWND hWnd = pFrame->GetHWND();
	HWND hWndParent = ::GetParent(m_hWnd);
	HWND hWndCenter = ::GetWindowOwner(m_hWnd);
	if (hWndCenter != NULL)
		hWnd = hWndCenter;

	// 处理多显示器模式下屏幕居中
	MONITORINFO oMonitor = {};
	oMonitor.cbSize = sizeof(oMonitor);
	::GetMonitorInfo(::MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST), &oMonitor);
	rcArea = oMonitor.rcWork;

	if (hWndCenter == NULL || IsIconic(hWndCenter))
		rcCenter = rcArea;
	else
		::GetWindowRect(hWndCenter, &rcCenter);

	int DlgWidth = rcDlg.right - rcDlg.left;
	int DlgHeight = rcDlg.bottom - rcDlg.top;

	// Find dialog's upper left based on rcCenter
	int xLeft = rcCenter.right - DlgWidth - 5;
	int yTop = 5;

	int cy = rcCenter.bottom - rcCenter.top - 10;

	// The dialog is outside the screen, move it inside
	if (xLeft < rcArea.left) xLeft = rcArea.left;
	else if (xLeft + DlgWidth > rcArea.right) xLeft = rcArea.right - DlgWidth;
	if (yTop < rcArea.top) yTop = rcArea.top;
	else if (yTop + DlgHeight > rcArea.bottom) yTop = rcArea.bottom - DlgHeight;
	::SetWindowPos(m_hWnd, NULL, xLeft, yTop, DlgWidth, cy, SWP_NOZORDER | SWP_NOACTIVATE);

	SetWindowLong(m_hWnd, GWL_EXSTYLE, GetWindowLong(m_hWnd, GWL_EXSTYLE) | 0x08000000);

	SetWindowPos(m_hWnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE | SWP_SHOWWINDOW);
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPSTR /*lpCmdLine*/, int nCmdShow)
{
    CPaintManagerUI::SetInstance(hInstance);
    CPaintManagerUI::SetResourcePath(CPaintManagerUI::GetInstancePath());

    HRESULT Hr = ::CoInitialize(NULL);
    if( FAILED(Hr) ) return 0;

    //CWndShadow::Initialize(hInstance);

    CFrameWindowWnd* pFrame = new CFrameWindowWnd();
    if( pFrame == NULL ) return 0;
    pFrame->Create(NULL, _T("ExeDocker v1.0.0"), UI_WNDSTYLE_FRAME|WS_CLIPCHILDREN, WS_EX_WINDOWEDGE);
   
	//DockWindow(pFrame);
	
	pFrame->CenterWindow();
    pFrame->ShowWindow(true);
	
    CPaintManagerUI::MessageLoop();

    ::CoUninitialize();
    return 0;
}
