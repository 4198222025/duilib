#include "StdAfx.h"
#include "UIIcon.h"

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
	// CIconUI
	CIconUI::CIconUI(void)
	{
		memset(&m_rcInset, 0, sizeof(m_rcInset));
		SetAttribute(_T("bkcolor"), _T("#00000000"));
	}

	CIconUI::~CIconUI(void)
	{
	}

	LPCTSTR CIconUI::GetClass() const
	{
		return _T("IconUI");
	}

	LPVOID CIconUI::GetInterface(LPCTSTR pstrName)
	{
		if (_tcscmp(pstrName, _T("Icon")) == 0)
			return static_cast<CIconUI*>(this);
		return __super::GetInterface(pstrName);
	}

	void CIconUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
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

	void CIconUI::SetPos(RECT rc)
	{
		CControlUI::SetPos(rc);
	}

	void CIconUI::SetInset(RECT rc)
	{
		m_rcInset = rc;
	}

	void CIconUI::DoEvent(TEventUI& event)
	{
		CControlUI::DoEvent(event);


	}

	void CIconUI::SetVisible(bool bVisible)
	{
		CControlUI::SetVisible(bVisible);
	}

	void CIconUI::SetInternVisible(bool bVisible)
	{
		CControlUI::SetInternVisible(bVisible);

	}

	void    CIconUI::PaintBkImage(HDC hDC)
	//void CIconUI::DoPaint(HDC hDC, const RECT& rcPaint)
	{
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