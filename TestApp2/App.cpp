// App.cpp : Defines the entry point for the application.
//

#include "stdafx.h"



//#include <olectl.h>
//#pragma comment(lib, "oleaut32.lib")
#include "save_icon_file_by_handle.h"

using namespace yg_icon;

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

	char icon_file_path[256];
	sprintf(icon_file_path, _T("%s\\%d_%dx%d.ico"), params->iconDir, index, CXICON, CYICON);

	SaveIcon2(hIcon, icon_file_path);
	
	

	UnlockResource(hResLoaded);
	FreeResource(hResLoaded);

	return TRUE;
}

class CFrameWindowWnd : public CWindowWnd, public INotifyUI, public CDwm, public CDPI
{
public:
    CFrameWindowWnd() : m_pWndShadow(NULL) { };
    LPCTSTR GetWindowClassName() const { return _T("UIMainFrame"); };
    UINT GetClassStyle() const { return UI_CLASSSTYLE_FRAME | CS_DBLCLKS; };
    void OnFinalMessage(HWND /*hWnd*/) { delete this; };

    void Init() {
		int j = 0;
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

			pTileLayout->AddAt(pTileElement, 0);
		}
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
			}
			else if (msg.pSender->GetName() == _T("Option02")) {
				CTabLayoutUI * pTab = (CTabLayoutUI*)m_pm.FindControl(_T("TabLayoutMain"));
				pTab->SelectItem(1);//1代表第二个Tab页

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
						pisl->SetPath("E:\\github\\duilib\\bin\\TestApp2_d.exe");

						pisl->SetArguments("start 2019");
						pisl->SetIconLocation("E:\\github\\duilib\\bin\\IconFromPE\\QQScLauncher\\5_48x48.ico", 0);

						hr = pisl->QueryInterface(IID_IPersistFile, (void**)&pIPF);
						if (SUCCEEDED(hr))
						{

							/////////////////////////////////////////////////////////////////////////////////////////////////////////////

							//这里是我们要创建快捷方式的目标地址


							pIPF->Save(L"C:\\Users\\王磊\\Desktop\\START QQ.lnk", FALSE);
							pIPF->Release();
						}
						pisl->Release();
					}
					CoUninitialize();
				}
			}
            else if( msg.pSender->GetName() == _T("changeskinbtn") ) {
                if( CPaintManagerUI::GetResourcePath() == CPaintManagerUI::GetInstancePath() )
                    CPaintManagerUI::SetResourcePath(CPaintManagerUI::GetInstancePath() + _T("skin\\FlashRes"));
                else
                    CPaintManagerUI::SetResourcePath(CPaintManagerUI::GetInstancePath());
                CPaintManagerUI::ReloadSkin();
            }		
			else if (msg.pSender->GetName() == _T("load_all_button")){
				CTileLayoutUI* pTileLayout = static_cast<CTileLayoutUI*>(m_pm.FindControl(_T("software_list")));
				LoadSoftwareFromJson(pTileLayout, "local_software.json");			

				//MessageBox(NULL, _T("加载所有软件！"), _T("提示"), MB_OK);
			}
			else if (msg.pSender->GetName() == _T("load_office_button")){
				CTileLayoutUI* pTileLayout = static_cast<CTileLayoutUI*>(m_pm.FindControl(_T("software_list")));
				LoadSoftwareFromJson(pTileLayout, "local_software_office.json");

				//MessageBox(NULL, _T("加载办公软件！"), _T("提示"), MB_OK);
			}
			else if (msg.pSender->GetName() == _T("load_music_button")){
				CTileLayoutUI* pTileLayout = static_cast<CTileLayoutUI*>(m_pm.FindControl(_T("software_list")));
				LoadSoftwareFromJson(pTileLayout, "local_software_music.json");

				//MessageBox(NULL, _T("加载音乐软件！"), _T("提示"), MB_OK);
			}
			else if (msg.pSender->GetName() == _T("load_other_button")){
				CTileLayoutUI* pTileLayout = static_cast<CTileLayoutUI*>(m_pm.FindControl(_T("software_list")));
				LoadSoftwareFromJson(pTileLayout, "local_software_other.json");

				//MessageBox(NULL, _T("加载其他软件！"), _T("提示"), MB_OK);
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
        }
    }

	LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
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

		RECT rcCaption = m_pm.GetCaptionRect();
		if (pt.x >= rcClient.left + rcCaption.left && pt.x < rcClient.right - rcCaption.right \
			&& pt.y >= rcCaption.top && pt.y < rcCaption.bottom) {
			CControlUI* pControl = static_cast<CControlUI*>(m_pm.FindControl(pt));
			if (pControl && _tcscmp(pControl->GetClass(), DUI_CTR_BUTTON) != 0)
				return HTCAPTION;
		}

		return HTCLIENT;
	}

    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        if( uMsg == WM_CREATE ) {

			// 去掉WINDOWS的外框
			LONG styleValue = ::GetWindowLong(*this, GWL_STYLE);
			styleValue &= ~WS_CAPTION;
			::SetWindowLong(*this, GWL_STYLE, styleValue | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);

            m_pm.Init(m_hWnd);
            CDialogBuilder builder;
            CControlUI* pRoot = builder.Create(_T("test2.xml"), (UINT)0, NULL, &m_pm);
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
        else if( uMsg == WM_NCACTIVATE ) {
            if( !::IsIconic(*this) ) return (wParam == 0) ? TRUE : FALSE;
        }
		else
		{
			LRESULT lRes = 0;
			BOOL bHandled = TRUE;
			switch (uMsg) {
			case WM_CLOSE:         lRes = OnClose(uMsg, wParam, lParam, bHandled); break;
			case WM_DESTROY:       lRes = OnDestroy(uMsg, wParam, lParam, bHandled); break;
			case WM_NCACTIVATE:    lRes = OnNcActivate(uMsg, wParam, lParam, bHandled); break;
			case WM_NCCALCSIZE:    lRes = OnNcCalcSize(uMsg, wParam, lParam, bHandled); break;
			case WM_NCPAINT:       lRes = OnNcPaint(uMsg, wParam, lParam, bHandled); break;
			case WM_NCHITTEST:     lRes = OnNcHitTest(uMsg, wParam, lParam, bHandled); break;
			//case WM_SIZE:          lRes = OnSize(uMsg, wParam, lParam, bHandled); break;
			default:
				bHandled = FALSE;
			}
		}
        LRESULT lRes = 0;
        if( m_pm.MessageHandler(uMsg, wParam, lParam, lRes) ) return lRes;
        return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
    }

public:
    CPaintManagerUI m_pm;
    CWndShadow* m_pWndShadow;
};


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

    ::CoUninitialize();
    return 0;
}
