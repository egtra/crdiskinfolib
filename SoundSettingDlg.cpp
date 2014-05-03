/*---------------------------------------------------------------------------*/
//       Author : hiyohiyo
//         Mail : hiyohiyo@crystalmark.info
//          Web : http://crystalmark.info/
//      License : Simplified BSD license
//
//                                Copyright 2012 hiyohiyo. All rights reserved.
/*---------------------------------------------------------------------------*/


#include "stdafx.h"
#include "DiskInfo.h"
#include "DiskInfoDlg.h"
#include "SoundSettingDlg.h"

IMPLEMENT_DYNCREATE(CSoundSettingDlg, CDHtmlDialog)

CSoundSettingDlg::CSoundSettingDlg(CWnd* pParent /*=NULL*/)
	: CDHtmlDialogEx(CSoundSettingDlg::IDD, CSoundSettingDlg::IDH, pParent)
{
	_tcscpy_s(m_Ini, MAX_PATH, ((CDiskInfoApp*)AfxGetApp())->m_Ini.GetString());
	
	m_CurrentLangPath = ((CDHtmlMainDialog*)pParent)->m_CurrentLangPath;
	m_DefaultLangPath = ((CDHtmlMainDialog*)pParent)->m_DefaultLangPath;
	m_ZoomType = ((CDHtmlMainDialog*)pParent)->GetZoomType();
}

CSoundSettingDlg::~CSoundSettingDlg()
{
}

void CSoundSettingDlg::DoDataExchange(CDataExchange* pDX)
{
	CDHtmlDialogEx::DoDataExchange(pDX);

	DDX_DHtml_ElementInnerText(pDX, _T("SoundFile"), m_SoundFile);
}

BOOL CSoundSettingDlg::OnInitDialog()
{
	CDHtmlDialogEx::OnInitDialog();

	SetWindowText(i18n(_T("WindowTitle"), _T("SOUND_SETTINGS")));

	EnableDpiAware();
	InitDHtmlDialog(SIZE_X, SIZE_Y, ((CDiskInfoApp*)AfxGetApp())->m_SoundSettingDlgPath);

	return TRUE;
}

BEGIN_MESSAGE_MAP(CSoundSettingDlg, CDHtmlDialogEx)
END_MESSAGE_MAP()

BEGIN_DHTML_EVENT_MAP(CSoundSettingDlg)
	DHTML_EVENT_ONCLICK(_T("SelectSoundFile"), OnSelectSoundFile)
	DHTML_EVENT_ONCLICK(_T("NoSoundFile"), OnNoSoundFile)
	DHTML_EVENT_ONCLICK(_T("PlaySoundFile"), OnPlaySoundFile)
END_DHTML_EVENT_MAP()

void CSoundSettingDlg::OnDocumentComplete(LPDISPATCH pDisp, LPCTSTR szUrl)
{
	CString cstr;
	cstr = szUrl;
	if(cstr.Find(_T("html")) != -1 || cstr.Find(_T("dlg")) != -1)
	{
		m_FlagShowWindow = TRUE;

		TCHAR str[256];
		GetPrivateProfileString(_T("Setting"), _T("AlertSoundPath"), _T(""), str, 256, m_Ini);
		m_SoundFile = str;
		SetElementPropertyEx(_T("SoundFile"), DISPID_IHTMLELEMENT_TITLE, m_SoundFile);

		UpdateData(FALSE);
		ChangeZoomType(m_ZoomType);
		SetClientRect((DWORD)(SIZE_X * m_ZoomRatio), (DWORD)(SIZE_Y * m_ZoomRatio), 0);

		ShowWindow(SW_SHOW);
	}
}

HRESULT CSoundSettingDlg::OnSelectSoundFile(IHTMLElement* /*pElement*/)
{
	CFileDialog dlg(TRUE, _T(""), _T(""), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_EXPLORER,
		i18n(_T("Customize"), _T("SUPPORTED_SOUND_FORMAT"))
		+ _T("|*.wav;*.mp3;|WAVE (*.wav)|*.wav|MP3 (*.mp3)|*.mp3|")
		+ i18n(_T("Customize"), _T("ALL_FILES"))
		+ _T(" (*.*)|*.*||"));

	if(dlg.DoModal() == IDOK)
	{
		m_SoundFile = dlg.GetPathName();
		WritePrivateProfileString(_T("Setting"), _T("AlertSoundPath"), _T("\"") + m_SoundFile + _T("\""), m_Ini);
		SetElementPropertyEx(_T("SoundFile"), DISPID_IHTMLELEMENT_TITLE, m_SoundFile);
		UpdateData(FALSE);
	}
	return S_FALSE;
}


HRESULT CSoundSettingDlg::OnNoSoundFile(IHTMLElement* /*pElement*/)
{
	m_SoundFile = _T("");
	WritePrivateProfileString(_T("Setting"), _T("AlertSoundPath"), m_SoundFile, m_Ini);
	SetElementPropertyEx(_T("SoundFile"), DISPID_IHTMLELEMENT_TITLE, m_SoundFile);
	UpdateData(FALSE);
	return S_FALSE;
}

HRESULT CSoundSettingDlg::OnPlaySoundFile(IHTMLElement* /*pElement*/)
{
	::PostMessage(m_ParentWnd->GetSafeHwnd(), MY_PLAY_ALERT_SOUND, NULL, NULL);
	return S_FALSE;
}