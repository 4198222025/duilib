#ifndef __UIPANEL_H__
#define __UIPANEL_H__

#pragma once

namespace DuiLib
{
	//////////////////////////////////////////////////////////////////////////
	// CPanelUI
	class DUILIB_API CPanelUI : public CControlUI
	{
		friend class CIconWnd;
	public:
		CPanelUI(void);
		~CPanelUI(void);

		LPCTSTR	GetClass() const;
		LPVOID	GetInterface(LPCTSTR pstrName);
		void	SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);

		void	SetPos(RECT rc);
		void	SetInset(RECT rc);
		void	SetVisible(bool bVisible);
		void	SetInternVisible(bool bVisible);
		//void	DoPaint(HDC hDC, const RECT& rcPaint);
		bool    Activate();
		void    PaintBkImage(HDC hDC);
		void    PaintTextY(HDC hDC);
		void    DoEvent(TEventUI& event);


		void SetMaskColor(DWORD dwColor);
		DWORD GetMaskColor() const;
	private:
		DuiLib::CDuiString	m_sIcoImage;
		RECT				m_rcInset;

		UINT   m_bShowMask;

		UINT m_uButtonState;


		DWORD	m_dwMaskColor;

		LPWSTR  m_pWideText;
		DWORD	m_dwTextColor;
		DWORD	m_dwDisabledTextColor;
		int		m_iFont;
		UINT	m_uTextStyle;
		RECT	m_rcTextPadding;
		bool	m_bShowHtml;
		SIZE    m_szAvailableLast;
		SIZE    m_cxyFixedLast;
		bool    m_bNeedEstimateSize;
	};
}


#endif // __UIPANEL_H__
