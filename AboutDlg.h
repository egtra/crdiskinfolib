/*---------------------------------------------------------------------------*/
//       Author : hiyohiyo
//         Mail : hiyohiyo@crystalmark.info
//          Web : http://crystalmark.info/
//      License : The modified BSD license
//
//                           Copyright 2008-2009 hiyohiyo. All rights reserved.
/*---------------------------------------------------------------------------*/

#pragma once

class CAboutDlg : public CDHtmlDialogEx
{
	DECLARE_DYNCREATE(CAboutDlg)

	static const int SIZE_X = 260;
	static const int SIZE_Y = 180;

public:
	CAboutDlg(CWnd* pParent = NULL);
	virtual ~CAboutDlg();

	enum { IDD = IDD_ABOUT, IDH = IDR_HTML_DUMMY };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	virtual void OnDocumentComplete(LPDISPATCH pDisp, LPCTSTR szUrl);

	CString m_Version;
	CString m_Edition;
	CString m_Release;
	CString m_Copyright;;

	HRESULT OnCrystalDewWorld(IHTMLElement *pElement);

	DECLARE_MESSAGE_MAP()
	DECLARE_DHTML_EVENT_MAP()
public:
	afx_msg void OnZoom200();
};
