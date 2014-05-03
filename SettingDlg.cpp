/*---------------------------------------------------------------------------*/
//       Author : hiyohiyo
//         Mail : hiyohiyo@crystalmark.info
//          Web : http://crystalmark.info/
//      License : Simplified BSD license
//
//                                Copyright 2008 hiyohiyo. All rights reserved.
/*---------------------------------------------------------------------------*/


#include "stdafx.h"
#include "DiskInfo.h"
#include "DiskInfoDlg.h"
#include "SettingDlg.h"

IMPLEMENT_DYNCREATE(CSettingDlg, CDHtmlDialog)

CDiskInfoDlg *p;

CSettingDlg::CSettingDlg(CWnd* pParent /*=NULL*/)
	: CDHtmlDialogEx(CSettingDlg::IDD, CSettingDlg::IDH, pParent)
{
	p = (CDiskInfoDlg*)pParent;
	_tcscpy_s(m_Ini, MAX_PATH, ((CDiskInfoApp*)AfxGetApp())->m_Ini);

	m_CurrentLangPath = ((CDHtmlMainDialog*)p)->m_CurrentLangPath;
	m_DefaultLangPath = ((CDHtmlMainDialog*)p)->m_DefaultLangPath;
	m_ZoomType = ((CDHtmlMainDialog*)pParent)->GetZoomType();
}

CSettingDlg::~CSettingDlg()
{
}

void CSettingDlg::DoDataExchange(CDataExchange* pDX)
{
	CDHtmlDialog::DoDataExchange(pDX);

	DDX_DHtml_ElementInnerText(pDX, _T("AamStatus"), m_AamStatus);
	DDX_DHtml_ElementInnerText(pDX, _T("ApmStatus"), m_ApmStatus);

	DDX_DHtml_ElementInnerText(pDX, _T("LabelAam"), m_LabelAam);
	DDX_DHtml_ElementInnerText(pDX, _T("LabelApm"), m_LabelApm);


	DDX_DHtml_ElementInnerText(pDX, _T("AamLow"), m_AamLow);
	DDX_DHtml_ElementInnerText(pDX, _T("AamHigh"), m_AamHigh);
	DDX_DHtml_ElementInnerText(pDX, _T("AamRecommend"), m_AamRecommend);
	DDX_DHtml_ElementInnerText(pDX, _T("ApmLow"), m_ApmLow);
	DDX_DHtml_ElementInnerText(pDX, _T("ApmHigh"), m_ApmHigh);
	DDX_DHtml_ElementInnerText(pDX, _T("CurrentAam"), m_CurrentAam);
	DDX_DHtml_ElementInnerText(pDX, _T("RecommendAam"), m_RecommendAam);
	DDX_DHtml_ElementInnerText(pDX, _T("CurrentApm"), m_CurrentApm);
	DDX_DHtml_ElementInnerText(pDX, _T("EnableAam"), m_EnableAam);
	DDX_DHtml_ElementInnerText(pDX, _T("DisableAam"), m_DisableAam);
	DDX_DHtml_ElementInnerText(pDX, _T("EnableApm"), m_EnableApm);
	DDX_DHtml_ElementInnerText(pDX, _T("DisableApm"), m_DisableApm);

//	DDX_DHtml_ElementInnerText(pDX, _T("RecommendApm"), m_RecommendApm);
	DDX_DHtml_SelectValue(pDX, _T("SelectDisk"), m_SelectDisk);
	DDX_Control(pDX, IDC_AAM_SCROLLBAR, m_AamScrollbar);
	DDX_Control(pDX, IDC_APM_SCROLLBAR, m_ApmScrollbar);
}

BOOL CSettingDlg::OnInitDialog()
{
	CDHtmlDialogEx::OnInitDialog();

	if(p->m_Ata.vars.GetCount() == 0)
	{
		return FALSE;
	}

	SetWindowText(i18n(_T("WindowTitle"), _T("AAM_APM_CONTROL")));

	EnableDpiAware();
	InitDHtmlDialog(SIZE_X, SIZE_Y, ((CDiskInfoApp*)AfxGetApp())->m_SettingDlgPath);

	m_AamScrollbar.SetScrollRange(0x80, 0xFE);
	m_ApmScrollbar.SetScrollRange(0x01, 0xFE);

	return TRUE;
}

BEGIN_MESSAGE_MAP(CSettingDlg, CDHtmlDialogEx)
	ON_WM_HSCROLL()
END_MESSAGE_MAP()

BEGIN_DHTML_EVENT_MAP(CSettingDlg)
	DHTML_EVENT_ONCLICK(_T("SelectDisk"), OnSelectDisk)
	DHTML_EVENT_ONCLICK(_T("EnableAam"), OnEnableAam)
	DHTML_EVENT_ONCLICK(_T("DisableAam"), OnDisableAam)
	DHTML_EVENT_ONCLICK(_T("EnableApm"), OnEnableApm)
	DHTML_EVENT_ONCLICK(_T("DisableApm"), OnDisableApm)
END_DHTML_EVENT_MAP()

void CSettingDlg::OnDocumentComplete(LPDISPATCH pDisp, LPCTSTR szUrl)
{
	CString cstr;
	cstr = szUrl;
	if(cstr.Find(_T("html")) != -1 || cstr.Find(_T("dlg")) != -1)
	{
		m_FlagShowWindow = TRUE;
		InitLang();
		InitSelectDisk();
		ChangeZoomType(m_ZoomType);
		SetClientRect((DWORD)(SIZE_X * m_ZoomRatio), (DWORD)(SIZE_Y * m_ZoomRatio), 0);
		m_AamScrollbar.MoveWindow((DWORD)(22 * m_ZoomRatio), (DWORD)(100 * m_ZoomRatio), (DWORD)(360 * m_ZoomRatio), (DWORD)(20 * m_ZoomRatio));
		m_ApmScrollbar.MoveWindow((DWORD)(22 * m_ZoomRatio), (DWORD)(243 * m_ZoomRatio), (DWORD)(360 * m_ZoomRatio), (DWORD)(20 * m_ZoomRatio));
		CenterWindow();
		ShowWindow(SW_SHOW);
	}
}

void CSettingDlg::InitLang()
{
	m_AamLow = i18n(_T("AamApm"), _T("AAM_LOW"));
	m_AamHigh = i18n(_T("AamApm"), _T("AAM_HIGH"));
	m_AamRecommend = i18n(_T("AamApm"), _T("AAM_RECOMMEND"));

	m_ApmLow = i18n(_T("AamApm"), _T("APM_LOW"));
	m_ApmHigh = i18n(_T("AamApm"), _T("APM_HIGH"));

	m_LabelAam = i18n(_T("AamApm"), _T("AUTOMATIC_ACOUSTIC_MANAGEMENT"));
	m_LabelApm = i18n(_T("AamApm"), _T("ADVANCED_POWER_MANAGEMENT"));

	m_EnableAam = i18n(_T("TrayMenu"), _T("ENABLE"));
	m_DisableAam = i18n(_T("TrayMenu"), _T("DISABLE"));
	m_EnableApm = i18n(_T("TrayMenu"), _T("ENABLE"));
	m_DisableApm = i18n(_T("TrayMenu"), _T("DISABLE"));

	UpdateData(FALSE);
}

void CSettingDlg::InitSelectDisk()
{
	CString select;
	CString cstr;

	cstr.Format(_T("<select id=\"SelectDisk\" title=\"%s\" onchange=\"this.click()\">"), i18n(_T("AamApm"), _T("PLEASE_SELECT_DISK")));
	select = cstr;

	for(int i = 0; i < p->m_Ata.vars.GetCount(); i++)
	{
		CString temp;
		if(p->m_Ata.vars.GetAt(i).IsAamSupported && p->m_Ata.vars.GetAt(i).IsApmSupported)
		{
			temp = _T("[AAM, APM]");	
		}
		else if(p->m_Ata.vars.GetAt(i).IsAamSupported)
		{
			temp = _T("[AAM]");	
		}
		else if(p->m_Ata.vars.GetAt(i).IsApmSupported)
		{
			temp = _T("[APM]");	
		}

		if(i == 0)
		{
			cstr.Format(_T("<option value=\"%d\" selected=\"selected\">(%d) %s %s</option>"), i, i + 1, p->m_Ata.vars.GetAt(i).Model, temp);
		}
		else
		{
			cstr.Format(_T("<option value=\"%d\">(%d) %s %s</option>"), i, i + 1, p->m_Ata.vars.GetAt(i).Model, temp);
		}
		select += cstr;
	}
	select += _T("</select>");
	SetElementOuterHtmlEx(_T("SelectDisk"), select);

	UpdateData(TRUE);
	m_DiskIndex = _tstoi(m_SelectDisk);
	UpdateSelectDisk(m_DiskIndex);
}

HRESULT CSettingDlg::OnSelectDisk(IHTMLElement* /*pElement*/)
{
	UpdateData(TRUE);

	if(m_DiskIndex != _tstoi(m_SelectDisk))
	{
		m_DiskIndex = _tstoi(m_SelectDisk);
		UpdateSelectDisk(m_DiskIndex);
	}

	return S_FALSE;
}

/* Memo 2008/11/8
AamStatus/ApmStatus
1 : Enabled
0 : Disabled
-1: Unsupported
*/

HRESULT CSettingDlg::OnEnableAam(IHTMLElement* /*pElement*/)
{
	if(! p->m_Ata.vars.GetAt(m_DiskIndex).IsAamSupported)
	{
		return S_FALSE;
	}

	int targetValue = m_AamScrollbar.GetScrollPos();
	p->m_Ata.EnableAam(m_DiskIndex, targetValue);
	p->m_Ata.UpdateIdInfo(m_DiskIndex);

	if(p->m_Ata.vars.GetAt(m_DiskIndex).IsAamEnabled)
	{
		SetElementPropertyEx(_T("LabelAam"), DISPID_IHTMLELEMENT_CLASSNAME, _T("labelEnable"));
		m_AamStatus = _T("ON");
	}
	m_AamScrollbar.SetScrollPos(p->m_Ata.GetAamValue(m_DiskIndex));
	m_CurrentAam.Format(_T("%02Xh"), p->m_Ata.GetAamValue(m_DiskIndex));
	m_RecommendAam.Format(_T("%02Xh"), p->m_Ata.GetRecommendAamValue(m_DiskIndex));
	UpdateData(FALSE);

	// Save Settings
	CString cstr;
	cstr.Format(_T("%d"), targetValue);
	WritePrivateProfileString(_T("AamStatus"), p->m_Ata.vars[m_DiskIndex].ModelSerial, _T("1"), m_Ini);
	WritePrivateProfileString(_T("AamValue"), p->m_Ata.vars[m_DiskIndex].ModelSerial, cstr, m_Ini);

	return S_FALSE;
}

HRESULT CSettingDlg::OnDisableAam(IHTMLElement* /*pElement*/)
{
	if(! p->m_Ata.vars.GetAt(m_DiskIndex).IsAamSupported)
	{
		return S_FALSE;
	}

	p->m_Ata.DisableAam(m_DiskIndex);
	p->m_Ata.UpdateIdInfo(m_DiskIndex);

	if(! p->m_Ata.vars.GetAt(m_DiskIndex).IsAamEnabled)
	{
		SetElementPropertyEx(_T("LabelAam"), DISPID_IHTMLELEMENT_CLASSNAME, _T("labelDisable"));
		m_AamStatus = _T("OFF");
	}
	m_AamScrollbar.SetScrollPos(p->m_Ata.GetAamValue(m_DiskIndex));
	m_CurrentAam.Format(_T("%02Xh"), p->m_Ata.GetAamValue(m_DiskIndex));
	m_RecommendAam.Format(_T("%02Xh"), p->m_Ata.GetRecommendAamValue(m_DiskIndex));
	UpdateData(FALSE);

	// Save Settings
//	CString cstr;
//	cstr.Format(_T("%d"), m_AamScrollbar.GetScrollPos());
	WritePrivateProfileString(_T("AamStatus"), p->m_Ata.vars[m_DiskIndex].ModelSerial, _T("0"), m_Ini);
//	WritePrivateProfileString(_T("AamValue"), p->m_Ata.vars[m_DiskIndex].ModelSerial, cstr, m_Ini);

	return S_FALSE;
}

HRESULT CSettingDlg::OnEnableApm(IHTMLElement* /*pElement*/)
{
	if(! p->m_Ata.vars.GetAt(m_DiskIndex).IsApmSupported)
	{
		return S_FALSE;
	}

	int targetValue = m_ApmScrollbar.GetScrollPos();
	p->m_Ata.EnableApm(m_DiskIndex, targetValue);
	p->m_Ata.UpdateIdInfo(m_DiskIndex);

	if(p->m_Ata.vars.GetAt(m_DiskIndex).IsApmEnabled)
	{
		SetElementPropertyEx(_T("LabelApm"), DISPID_IHTMLELEMENT_CLASSNAME, _T("labelEnable"));
		m_ApmStatus = _T("ON");
	}
	m_ApmScrollbar.SetScrollPos(p->m_Ata.GetApmValue(m_DiskIndex));
	m_CurrentApm.Format(_T("%02Xh"), p->m_Ata.GetApmValue(m_DiskIndex));
	UpdateData(FALSE);

	// Save Settings
	CString cstr;
	cstr.Format(_T("%d"), targetValue);
	WritePrivateProfileString(_T("ApmStatus"), p->m_Ata.vars[m_DiskIndex].ModelSerial, _T("1"), m_Ini);
	WritePrivateProfileString(_T("ApmValue"), p->m_Ata.vars[m_DiskIndex].ModelSerial, cstr, m_Ini);

	return S_FALSE;
}

HRESULT CSettingDlg::OnDisableApm(IHTMLElement* /*pElement*/)
{
	if(! p->m_Ata.vars.GetAt(m_DiskIndex).IsApmSupported)
	{
		return S_FALSE;
	}

	p->m_Ata.DisableApm(m_DiskIndex);
	p->m_Ata.UpdateIdInfo(m_DiskIndex);

	if(! p->m_Ata.vars.GetAt(m_DiskIndex).IsApmEnabled)
	{
		SetElementPropertyEx(_T("LabelApm"), DISPID_IHTMLELEMENT_CLASSNAME, _T("labelDisable"));
		m_ApmStatus = _T("OFF");
	}
	m_ApmScrollbar.SetScrollPos(p->m_Ata.GetApmValue(m_DiskIndex));
	m_CurrentApm.Format(_T("%02Xh"), p->m_Ata.GetApmValue(m_DiskIndex));
	UpdateData(FALSE);

	// Save Settings
//	CString cstr;
//	cstr.Format(_T("%d"), m_ApmScrollbar.GetScrollPos());
	WritePrivateProfileString(_T("ApmStatus"), p->m_Ata.vars[m_DiskIndex].ModelSerial, _T("0"), m_Ini);
//	WritePrivateProfileString(_T("ApmValue"), p->m_Ata.vars[m_DiskIndex].ModelSerial, cstr, m_Ini);

	return S_FALSE;
}

void CSettingDlg::UpdateSelectDisk(DWORD index)
{
	m_AamScrollbar.SetScrollPos(p->m_Ata.GetAamValue(index));
	m_CurrentAam.Format(_T("%02Xh"), p->m_Ata.GetAamValue(index));
	m_RecommendAam.Format(_T("%02Xh"), p->m_Ata.GetRecommendAamValue(index));

	m_ApmScrollbar.SetScrollPos(p->m_Ata.GetApmValue(index));
	m_CurrentApm.Format(_T("%02Xh"), p->m_Ata.GetApmValue(index));
//	m_RecommendApm.Format(_T("%02Xh"), p->m_Ata.GetRecommendApmValue(index));

	if(p->m_Ata.vars.GetAt(index).IsAamSupported 
		&& p->m_Ata.vars.GetAt(index).CommandType != p->m_Ata.CMD_TYPE_SCSI_MINIPORT
		&& p->m_Ata.vars.GetAt(index).CommandType != p->m_Ata.CMD_TYPE_SILICON_IMAGE
		&& p->m_Ata.vars.GetAt(index).CommandType != p->m_Ata.CMD_TYPE_CSMI
		)
	{
		m_AamScrollbar.EnableWindow(TRUE);
		if(p->m_Ata.vars.GetAt(index).IsAamEnabled)
		{
			SetElementPropertyEx(_T("LabelAam"), DISPID_IHTMLELEMENT_CLASSNAME, _T("labelEnable"));
			m_AamStatus = _T("ON");
		}
		else
		{
			SetElementPropertyEx(_T("LabelAam"), DISPID_IHTMLELEMENT_CLASSNAME, _T("labelDisable"));
			m_AamStatus = _T("OFF");
		}
		SetElementPropertyEx(_T("EnableAam"), DISPID_IHTMLELEMENT_CLASSNAME, _T("buttonEnable"));
		SetElementPropertyEx(_T("DisableAam"), DISPID_IHTMLELEMENT_CLASSNAME, _T("buttonEnable"));
	}
	else
	{
		m_AamScrollbar.EnableWindow(FALSE);
		m_AamStatus = _T("");
		SetElementPropertyEx(_T("LabelAam"), DISPID_IHTMLELEMENT_CLASSNAME, _T("labelUnsupported"));
		SetElementPropertyEx(_T("EnableAam"), DISPID_IHTMLELEMENT_CLASSNAME, _T("buttonDisable"));
		SetElementPropertyEx(_T("DisableAam"), DISPID_IHTMLELEMENT_CLASSNAME, _T("buttonDisable"));
	}

	if(p->m_Ata.vars.GetAt(index).IsApmSupported
		&& p->m_Ata.vars.GetAt(index).CommandType != p->m_Ata.CMD_TYPE_SCSI_MINIPORT
		&& p->m_Ata.vars.GetAt(index).CommandType != p->m_Ata.CMD_TYPE_SILICON_IMAGE
		&& p->m_Ata.vars.GetAt(index).CommandType != p->m_Ata.CMD_TYPE_CSMI
		)
	{
		m_ApmScrollbar.EnableWindow(TRUE);
		if(p->m_Ata.vars.GetAt(index).IsApmEnabled)
		{
			SetElementPropertyEx(_T("LabelApm"), DISPID_IHTMLELEMENT_CLASSNAME, _T("labelEnable"));
			m_ApmStatus = _T("ON");
		}
		else
		{
			SetElementPropertyEx(_T("LabelApm"), DISPID_IHTMLELEMENT_CLASSNAME, _T("labelDisable"));
			m_ApmStatus = _T("OFF");
		}
		SetElementPropertyEx(_T("EnableApm"), DISPID_IHTMLELEMENT_CLASSNAME, _T("buttonEnable"));
		SetElementPropertyEx(_T("DisableApm"), DISPID_IHTMLELEMENT_CLASSNAME, _T("buttonEnable"));
	}
	else
	{
		m_ApmScrollbar.EnableWindow(FALSE);
		m_ApmStatus = _T("");
		SetElementPropertyEx(_T("LabelApm"), DISPID_IHTMLELEMENT_CLASSNAME, _T("labelUnsupported"));
		SetElementPropertyEx(_T("EnableApm"), DISPID_IHTMLELEMENT_CLASSNAME, _T("buttonDisable"));
		SetElementPropertyEx(_T("DisableApm"), DISPID_IHTMLELEMENT_CLASSNAME, _T("buttonDisable"));
	}

	UpdateData(FALSE);
}

void CSettingDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	int position = pScrollBar->GetScrollPos();
	switch(nSBCode)
	{

	case SB_LINELEFT:
		position -= 1;
		break;
	case SB_LINERIGHT:
		position += 1;
		break;
	case SB_PAGELEFT:
		position -= 8;
		break;
	case SB_PAGERIGHT:
		position += 8;
		break;
	case SB_LEFT:
		break;
	case SB_RIGHT:
		break;
	case SB_THUMBTRACK:
		position = nPos;
		break;
	}
	pScrollBar->SetScrollPos(position);

	if(*pScrollBar == m_AamScrollbar)
	{
		m_CurrentAam.Format(_T("%02Xh"), m_AamScrollbar.GetScrollPos());
	}
	else if(*pScrollBar == m_ApmScrollbar)
	{
		m_CurrentApm.Format(_T("%02Xh"), m_ApmScrollbar.GetScrollPos());
	}

	UpdateData(FALSE);
	
	CDHtmlDialogEx::OnHScroll(nSBCode, nPos, pScrollBar);
}
