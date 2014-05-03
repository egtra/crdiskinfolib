/*---------------------------------------------------------------------------*/
//       Author : hiyohiyo
//         Mail : hiyohiyo@crystalmark.info
//          Web : http://crystalmark.info/
//      License : The modified BSD license
//
//                                Copyright 2008 hiyohiyo. All rights reserved.
/*---------------------------------------------------------------------------*/

#pragma once
#include "afxwin.h"

class CSettingDlg : public CDHtmlDialogEx
{
	DECLARE_DYNCREATE(CSettingDlg)

	static const int SIZE_X = 444;
	static const int SIZE_Y = 310;

public:
	CSettingDlg(CWnd* pParent = NULL);
	virtual ~CSettingDlg();

	enum { IDD = IDD_SETTING, IDH = IDR_HTML_DUMMY };

protected:
	HICON m_hIcon;
	DWORD m_DiskIndex;
	CString m_SelectDisk;

	CString m_AamLow;
	CString m_AamHigh;
	CString m_AamRecommend;
	CString m_ApmLow;
	CString m_ApmHigh;

	CString m_AamStatus;
	CString m_ApmStatus;

	CString m_CurrentAam;
	CString m_RecommendAam;
	CString m_CurrentApm;
//	CString m_RecommendApm;

	CString m_EnableAam;
	CString m_DisableAam;
	CString m_EnableApm;
	CString m_DisableApm;

	VARIANT dummy;

	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	virtual void OnDocumentComplete(LPDISPATCH pDisp, LPCTSTR szUrl);

	void InitSelectDisk();
	void InitLang();

	HRESULT OnSelectDisk(IHTMLElement* /*pElement*/);
	HRESULT OnEnableAam(IHTMLElement* /*pElement*/);
	HRESULT OnDisableAam(IHTMLElement* /*pElement*/);
	HRESULT OnEnableApm(IHTMLElement* /*pElement*/);
	HRESULT OnDisableApm(IHTMLElement* /*pElement*/);

	void UpdateSelectDisk(DWORD index);

	DECLARE_MESSAGE_MAP()
	DECLARE_DHTML_EVENT_MAP()
public:
	CScrollBar m_AamScrollbar;
	CScrollBar m_ApmScrollbar;
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
};
