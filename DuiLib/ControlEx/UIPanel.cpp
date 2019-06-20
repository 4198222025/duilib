#include "StdAfx.h"
#include "UIPanel.h"

#ifdef _USE_GDIPLUS
using namespace Gdiplus;
#endif

namespace DuiLib
{
	class AutoBITMAPHandle
	{
	public:
		AutoBITMAPHandle(HBITMAP handle)
			: handle_(handle)
		{
		};
		~AutoBITMAPHandle()
		{
			::DeleteObject(handle_);
		}
		operator HBITMAP() const
		{
			return handle_;
		}
	private:
		HBITMAP handle_;
	};

	//////////////////////////////////////////////////////////////////////////
	// CPanelUI
	CPanelUI::CPanelUI(void)
	{
		m_bShowMask = 0;
		memset(&m_rcInset, 0, sizeof(m_rcInset));
		SetAttribute(_T("bkcolor"), _T("#00000000"));
	}

	CPanelUI::~CPanelUI(void)
	{
	}

	LPCTSTR CPanelUI::GetClass() const
	{
		return _T("PanelUI");
	}

	LPVOID CPanelUI::GetInterface(LPCTSTR pstrName)
	{
		if (_tcscmp(pstrName, _T("Panel")) == 0)
			return static_cast<CPanelUI*>(this);
		return __super::GetInterface(pstrName);
	}

	void CPanelUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if (_tcscmp(pstrName, _T("icon")) == 0)
			m_sIcoImage = pstrValue;
		if (_tcscmp(pstrName, _T("inset")) == 0)
		{
			RECT rcInset = { 0 };
			LPTSTR pstr = NULL;
			rcInset.left = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);
			rcInset.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);
			rcInset.right = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);
			rcInset.bottom = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);
			SetInset(rcInset);
		}
		if (_tcscmp(pstrName, _T("pos")) == 0)
		{
			RECT rcInset = { 0 };
			LPTSTR pstr = NULL;
			rcInset.left = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);
			rcInset.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);
			rcInset.right = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);
			rcInset.bottom = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);
			SetPos(rcInset);
		}
		Invalidate();
		__super::SetAttribute(pstrName, pstrValue);
	}

	void CPanelUI::SetPos(RECT rc)
	{
		CControlUI::SetPos(rc);
	}

	void CPanelUI::SetInset(RECT rc)
	{
		m_rcInset = rc;
	}

	void CPanelUI::DoEvent(TEventUI& event)
	{

		if (event.Type == UIEVENT_SETFOCUS)
		{
			Invalidate();
		}
		if (event.Type == UIEVENT_KILLFOCUS)
		{
			Invalidate();
		}

		if (event.Type == UIEVENT_MOUSEENTER)
		{
			/*if (::PtInRect(&m_rcItem, event.ptMouse)) {
				if (IsEnabled()) {
					if ((m_uButtonState & UISTATE_HOT) == 0) {
						m_uButtonState |= UISTATE_HOT;
						Invalidate();
					}
				}
			}
			if (GetFadeAlphaDelta() > 0) {
				m_pManager->SetTimer(this, FADE_TIMERID, FADE_ELLAPSE);
			}*/
			m_bShowMask = true;
			Invalidate();
		}
		if (event.Type == UIEVENT_MOUSELEAVE)
		{
			/*if (!::PtInRect(&m_rcItem, event.ptMouse)) {
				if (IsEnabled()) {
					if ((m_uButtonState & UISTATE_HOT) != 0) {
						m_uButtonState &= ~UISTATE_HOT;
						Invalidate();
					}
				}
				if (m_pManager) m_pManager->RemoveMouseLeaveNeeded(this);
				if (GetFadeAlphaDelta() > 0) {
					m_pManager->SetTimer(this, FADE_TIMERID, FADE_ELLAPSE);
				}
			}
			else {
				if (m_pManager) m_pManager->AddMouseLeaveNeeded(this);
				return;
			}*/
			m_bShowMask = false;
			Invalidate();
		}

		CControlUI::DoEvent(event);


	}

	void CPanelUI::SetVisible(bool bVisible)
	{
		CControlUI::SetVisible(bVisible);
	}

	void CPanelUI::SetInternVisible(bool bVisible)
	{
		CControlUI::SetInternVisible(bVisible);

	}

	void CPanelUI::PaintTextY(HDC hDC)
	{
		if (m_dwTextColor == 0) m_dwTextColor = m_pManager->GetDefaultFontColor();
		if (m_dwDisabledTextColor == 0) m_dwDisabledTextColor = m_pManager->GetDefaultDisabledColor();

		RECT rc = m_rcItem;
		rc.left += m_rcTextPadding.left;
		rc.right -= m_rcTextPadding.right;
		rc.top += m_rcTextPadding.top;
		rc.bottom -= m_rcTextPadding.bottom;

		//if (!GetEnabledEffect())
		{
			m_sText = "下载";
			m_iFont = 2;
			m_uTextStyle = DT_SINGLELINE | DT_VCENTER | DT_CENTER;
			m_dwTextColor = m_pManager->GetDefaultFontColor();

			if (m_sText.IsEmpty()) return;
			int nLinks = 0;
			if (IsEnabled()) {
				if (m_bShowHtml)
					CRenderEngine::DrawHtmlText(hDC, m_pManager, rc, m_sText, m_dwTextColor, \
					NULL, NULL, nLinks, m_iFont, m_uTextStyle);
				else
					CRenderEngine::DrawText(hDC, m_pManager, rc, m_sText, m_dwTextColor, \
					m_iFont, m_uTextStyle);
			}
			else {
				if (m_bShowHtml)
					CRenderEngine::DrawHtmlText(hDC, m_pManager, rc, m_sText, m_dwDisabledTextColor, \
					NULL, NULL, nLinks, m_iFont, m_uTextStyle);
				else
					CRenderEngine::DrawText(hDC, m_pManager, rc, m_sText, m_dwDisabledTextColor, \
					m_iFont, m_uTextStyle);
			}
		}
		//else
		{
		}
	}

	void    CPanelUI::PaintBkImage(HDC hDC)
	//void CIconUI::DoPaint(HDC hDC, const RECT& rcPaint)
	{
		if (m_bShowMask)
		{
			PaintTextY(hDC);
			return;
		}

		RECT rcTemp = { 0 };
		//if (!::IntersectRect(&m_rcPaint, &rcPaint, &m_rcItem)) return;

		HICON hIcon = (HICON)::LoadImage(m_pManager->GetInstance(), m_sIcoImage, IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR | LR_LOADFROMFILE );
		if (hIcon == NULL)
			hIcon = (HICON)::LoadImage(m_pManager->GetInstance(), m_pManager->GetResourcePath() + _T("\\Favicon\\blank_favicon.ico"), IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR | LR_LOADFROMFILE | LR_DEFAULTSIZE);
		if (NULL != hIcon)
		{
			ICONINFO icon_info;
			::GetIconInfo(hIcon, &icon_info);
			AutoBITMAPHandle hbmColor(icon_info.hbmColor);
			AutoBITMAPHandle hbmMask(icon_info.hbmMask);

			RECT rect;
			rect.left = m_rcItem.left + m_rcInset.left;
			rect.right = m_rcItem.right - m_rcInset.right;
			rect.top = m_rcItem.top + m_rcInset.top;
			rect.bottom = m_rcItem.bottom - m_rcInset.bottom;

			
			HDC dcCompatible;
			dcCompatible = ::CreateCompatibleDC(hDC);  // 创建与当前DC（pDC）兼容的DC
			::SelectObject(dcCompatible, hbmColor);

			
			BITMAP bmp;
			GetObject(hbmColor, sizeof(bmp), &bmp);

			//BitBlt(hDC, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, dcCompatible, 0, 0, SRCCOPY);
			//StretchBlt(hDC, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, dcCompatible, 0, 0, bmp.bmWidth, bmp.bmHeight, SRCCOPY);
			
			::DrawIconEx(hDC, rect.left, rect.top, hIcon, bmp.bmWidth, bmp.bmHeight, 0, NULL, DI_NORMAL);
			::DeleteObject(hIcon);
		}
	}


}