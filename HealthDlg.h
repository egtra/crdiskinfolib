/*---------------------------------------------------------------------------*/
//       Author : hiyohiyo
//         Mail : hiyohiyo@crystalmark.info
//          Web : http://crystalmark.info/
//      License : Simplified BSD license
//
//                                Copyright 2009 hiyohiyo. All rights reserved.
/*---------------------------------------------------------------------------*/

#pragma once
#include "afxwin.h"

class CHealthDlg : public CDHtmlDialogEx
{
	DECLARE_DYNCREATE(CHealthDlg)

	static const int SIZE_X = 444;
	static const int SIZE_Y = 280;

public:
	CHealthDlg(CWnd* pParent = NULL);
	virtual ~CHealthDlg();

	enum { IDD = IDD_HEALTH, IDH = IDR_HTML_DUMMY };

protected:
	DWORD m_DiskIndex;
	CString m_SelectDisk;

	CString m_Value05;
	CString m_ValueC5;
	CString m_ValueC6;

	CString m_Value05X;
	CString m_ValueC5X;
	CString m_ValueC6X;

	CString m_Label05;
	CString m_LabelC5;
	CString m_LabelC6;

	CString m_Apply;
	CString m_Default;

	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	virtual void OnDocumentComplete(LPDISPATCH pDisp, LPCTSTR szUrl);

	void InitSelectDisk();
	void InitLang();

	HRESULT OnSelectDisk(IHTMLElement* /*pElement*/);
	void UpdateSelectDisk(DWORD index);

	HRESULT OnApply(IHTMLElement* /*pElement*/);
	HRESULT OnDefault(IHTMLElement* /*pElement*/);

	DECLARE_MESSAGE_MAP()
	DECLARE_DHTML_EVENT_MAP()
public:
	CScrollBar m_Scrollbar05;
	CScrollBar m_ScrollbarC5;
	CScrollBar m_ScrollbarC6;
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
};
