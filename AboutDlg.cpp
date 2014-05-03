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
	m_ZoomType = ((CDHtmlMainDialog*)pParent)->GetZoomType();
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

	EnableDpiAware();
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

		ChangeZoomType(m_ZoomType);
		SetClientRect((DWORD)(SIZE_X * m_ZoomRatio), (DWORD)(SIZE_Y * m_ZoomRatio), 0);

		CString arg;
		TCHAR path[MAX_PATH];
		TCHAR exe[_MAX_FNAME];
		GetModuleFileName(NULL, path, sizeof(path));
		_wsplitpath_s(path, NULL, 0, NULL, 0, exe, _MAX_FNAME, NULL, 0);

#ifdef SUISYO_SHIZUKU_SUPPORT
		arg.Format(_T("res://%s.exe/#2110/%d"), exe, IDR_SHIZUKU_ABOUT);
		CallScript(_T("setShizuku"), arg);
#endif
		arg.Format(_T("res://%s.exe/#2110/%d"), exe, IDR_ABOUT_ICON);
		CallScript(_T("setIcon"), arg);

		UpdateData(FALSE);
		CenterWindow();
		ShowWindow(SW_SHOW);
	}
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDHtmlDialogEx)
END_MESSAGE_MAP()

BEGIN_DHTML_EVENT_MAP(CAboutDlg)
	DHTML_EVENT_ONCLICK(_T("CrystalDewWorld"), OnCrystalDewWorld)
	DHTML_EVENT_ONCLICK(_T("ProjectShizuku"), OnProjectShizuku)
END_DHTML_EVENT_MAP()

HRESULT CAboutDlg::OnCrystalDewWorld(IHTMLElement* /*pElement*/)
{
	if(GetUserDefaultLCID() == 0x0411)// Japanese
	{
		OpenUrl(URL_CRYSTAL_DEW_WORLD_JA);
	}
	else // Other Language
	{
		OpenUrl(URL_CRYSTAL_DEW_WORLD_EN);
	}

	return S_FALSE;
}

HRESULT CAboutDlg::OnProjectShizuku(IHTMLElement* /*pElement*/)
{
	if(GetUserDefaultLCID() == 0x0411)// Japanese
	{
		OpenUrl(URL_PROJECT_SHIZUKU_JA);
	}
	else // Other Language
	{
		OpenUrl(URL_PROJECT_SHIZUKU_EN);
	}

	return S_FALSE;
}