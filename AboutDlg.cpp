/*---------------------------------------------------------------------------*/
//       Author : hiyohiyo
//         Mail : hiyohiyo@crystalmark.info
//          Web : http://crystalmark.info/
//      License : The modified BSD license
//
//                           Copyright 2008-2009 hiyohiyo. All rights reserved.
/*---------------------------------------------------------------------------*/

#include "stdafx.h"
#include "DiskInfo.h"
#include "AboutDlg.h"

IMPLEMENT_DYNCREATE(CAboutDlg, CDHtmlDialog)

CAboutDlg::CAboutDlg(CWnd* pParent /*=NULL*/)
	: CDHtmlDialogEx(CAboutDlg::IDD, CAboutDlg::IDH, pParent)
{
	m_CurrentLangPath = ((CDHtmlMainDialog*)pParent)->m_CurrentLangPath;
	m_DefaultLangPath = ((CDHtmlMainDialog*)pParent)->m_DefaultLangPath;
}

CAboutDlg::~CAboutDlg()
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDHtmlDialogEx::DoDataExchange(pDX);

	DDX_DHtml_ElementInnerText(pDX, _T("Version"), m_Version);
	DDX_DHtml_ElementInnerText(pDX, _T("Edition"), m_Edition);
	DDX_DHtml_ElementInnerText(pDX, _T("Release"), m_Release);
	DDX_DHtml_ElementInnerText(pDX, _T("Copyright"), m_Copyright);
}

BOOL CAboutDlg::OnInitDialog()
{
	CDHtmlDialogEx::OnInitDialog();

	SetWindowText(i18n(_T("WindowTitle"), _T("ABOUT")));

	InitDHtmlDialog(SIZE_X, SIZE_Y, ((CDiskInfoApp*)AfxGetApp())->m_AboutDlgPath);

	return TRUE;
}

void CAboutDlg::OnDocumentComplete(LPDISPATCH pDisp, LPCTSTR szUrl)
{
	CString cstr;
	cstr = szUrl;
	if(cstr.Find(_T("html")) != -1 || cstr.Find(_T("dlg")) != -1)
	{
		m_FlagShowWindow = TRUE;
		m_Version = PRODUCT_VERSION;
		m_Edition = PRODUCT_EDITION;
		m_Release = PRODUCT_RELEASE;
		m_Copyright = PRODUCT_COPYRIGHT;

		UpdateData(FALSE);
		ShowWindow(SW_SHOW);
	}
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDHtmlDialogEx)
END_MESSAGE_MAP()

BEGIN_DHTML_EVENT_MAP(CAboutDlg)
	DHTML_EVENT_ONCLICK(_T("CrystalDewWorld"), OnCrystalDewWorld)
END_DHTML_EVENT_MAP()

HRESULT CAboutDlg::OnCrystalDewWorld(IHTMLElement* /*pElement*/)
{
	if(GetUserDefaultLCID() == 0x0411)// Japanese
	{
		ShellExecute(NULL, NULL, URL_CRYSTAL_DEW_WORLD_JA, NULL, NULL, SW_SHOWNORMAL);
	}
	else // Other Language
	{
		ShellExecute(NULL, NULL, URL_CRYSTAL_DEW_WORLD_EN, NULL, NULL, SW_SHOWNORMAL);
	}

	return S_FALSE;
}