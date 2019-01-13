#ifndef __UIICON_H__
#define __UIICON_H__

#pragma once

namespace DuiLib
{
	//////////////////////////////////////////////////////////////////////////
	// CIconUI
	class DUILIB_API CIconUI : public CControlUI
	{
		friend class CIconWnd;
	public:
		CIconUI(void);
		~CIconUI(void);

		LPCTSTR	GetClass() const;
		LPVOID	GetInterface(LPCTSTR pstrName);
		void	SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue);

		void	SetPos(RECT rc);
		void	SetInset(RECT rc);
		void	SetVisible(bool bVisible);
		void	SetInternVisible(bool bVisible);
		//void	DoPaint(HDC hDC, const RECT& rcPaint);
		void    PaintBkImage(HDC hDC);
		void    DoEvent(TEventUI& event);
	private:
		DuiLib::CDuiString	m_sIcoImage;
		RECT				m_rcInset;
	};
}


#endif // __UIICON_H__
