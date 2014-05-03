/*---------------------------------------------------------------------------*/
//       Author : hiyohiyo
//         Mail : hiyohiyo@crystalmark.info
//          Web : http://crystalmark.info/
//      License : The modified BSD license
//
//                                Copyright 2008 hiyohiyo. All rights reserved.
/*---------------------------------------------------------------------------*/

#pragma once

#include "AtaSmart.h"

class COptionDlg : public CDHtmlDialogEx
{
	DECLARE_DYNCREATE(COptionDlg)

	static const int SIZE_X = 480;
	static const int SIZE_Y = 360;

public:
	COptionDlg(CWnd* pParent = NULL);
	virtual ~COptionDlg();
	COLORREF m_CurrentLineColor[CAtaSmart::MAX_DISK + 1];
	COLORREF m_DefaultLineColor[CAtaSmart::MAX_DISK + 1];

	enum { IDD = IDD_OPTION, IDH = IDR_HTML_DUMMY };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	virtual void OnDocumentComplete(LPDISPATCH pDisp, LPCTSTR szUrl);

	CString m_ColorCode[CAtaSmart::MAX_DISK + 1];
	CString m_LabelLineColor;
	CString m_LabelThreshold;
	CString m_LabelBgImage;
	CString m_BgImage;
	CString m_Reset;

	HRESULT OnSelect0(IHTMLElement *pElement);
	HRESULT OnSelect1(IHTMLElement *pElement);
	HRESULT OnSelect2(IHTMLElement *pElement);
	HRESULT OnSelect3(IHTMLElement *pElement);
	HRESULT OnSelect4(IHTMLElement *pElement);
	HRESULT OnSelect5(IHTMLElement *pElement);
	HRESULT OnSelect6(IHTMLElement *pElement);
	HRESULT OnSelect7(IHTMLElement *pElement);
	HRESULT OnSelect8(IHTMLElement *pElement);
	HRESULT OnSelect9(IHTMLElement *pElement);
	HRESULT OnSelect10(IHTMLElement *pElement);
	HRESULT OnSelect11(IHTMLElement *pElement);
	HRESULT OnSelect12(IHTMLElement *pElement);
	HRESULT OnSelect13(IHTMLElement *pElement);
	HRESULT OnSelect14(IHTMLElement *pElement);
	HRESULT OnSelect15(IHTMLElement *pElement);
	HRESULT OnSelect16(IHTMLElement *pElement);
	HRESULT OnSelect17(IHTMLElement *pElement);
	HRESULT OnSelect18(IHTMLElement *pElement);
	HRESULT OnSelect19(IHTMLElement *pElement);
	HRESULT OnSelect20(IHTMLElement *pElement);
	HRESULT OnSelect21(IHTMLElement *pElement);
	HRESULT OnSelect22(IHTMLElement *pElement);
	HRESULT OnSelect23(IHTMLElement *pElement);
	HRESULT OnSelect24(IHTMLElement *pElement);
	HRESULT OnSelect25(IHTMLElement *pElement);
	HRESULT OnSelect26(IHTMLElement *pElement);
	HRESULT OnSelect27(IHTMLElement *pElement);
	HRESULT OnSelect28(IHTMLElement *pElement);
	HRESULT OnSelect29(IHTMLElement *pElement);
	HRESULT OnSelect30(IHTMLElement *pElement);
	HRESULT OnSelect31(IHTMLElement *pElement);
	HRESULT OnSelect32(IHTMLElement *pElement);

	HRESULT OnSelectBgImage(IHTMLElement *pElement);
	HRESULT OnNoBgImage(IHTMLElement *pElement);
	HRESULT OnReset(IHTMLElement *pElement);

	void SelectColor(DWORD index);

	DECLARE_MESSAGE_MAP()
	DECLARE_DHTML_EVENT_MAP()
};
