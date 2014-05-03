/*---------------------------------------------------------------------------*/
//       Author : hiyohiyo
//         Mail : hiyohiyo@crystalmark.info
//          Web : http://crystalmark.info/
//      License : Simplified BSD license
/*---------------------------------------------------------------------------*/

#include "stdafx.h"
#include "DiskInfo.h"
#include "DiskInfoDlg.h"
#include "HealthDlg.h"

CDiskInfoDlg *h;

IMPLEMENT_DYNCREATE(CHealthDlg, CDHtmlDialog)

CHealthDlg::CHealthDlg(CWnd* pParent /*=NULL*/)
	: CDHtmlDialogEx(CHealthDlg::IDD, CHealthDlg::IDH, pParent)
{
	h = (CDiskInfoDlg*)pParent;

	_tcscpy_s(m_Ini, MAX_PATH, ((CDiskInfoApp*)AfxGetApp())->m_Ini);

	m_CurrentLangPath = ((CDHtmlMainDialog*)pParent)->m_CurrentLangPath;
	m_DefaultLangPath = ((CDHtmlMainDialog*)pParent)->m_DefaultLangPath;
	m_ZoomType = ((CDHtmlMainDialog*)pParent)->GetZoomType();
}

CHealthDlg::~CHealthDlg()
{
}

void CHealthDlg::DoDataExchange(CDataExchange* pDX)
{
	CDHtmlDialog::DoDataExchange(pDX);

	DDX_DHtml_ElementInnerText(pDX, _T("Value05"), m_Value05);
	DDX_DHtml_ElementInnerText(pDX, _T("ValueC5"), m_ValueC5);
	DDX_DHtml_ElementInnerText(pDX, _T("ValueC6"), m_ValueC6);

	DDX_DHtml_ElementInnerText(pDX, _T("Value05X"), m_Value05X);
	DDX_DHtml_ElementInnerText(pDX, _T("ValueC5X"), m_ValueC5X);
	DDX_DHtml_ElementInnerText(pDX, _T("ValueC6X"), m_ValueC6X);

	DDX_DHtml_ElementInnerText(pDX, _T("Label05"), m_Label05);
	DDX_DHtml_ElementInnerText(pDX, _T("LabelC5"), m_LabelC5);
	DDX_DHtml_ElementInnerText(pDX, _T("LabelC6"), m_LabelC6);

	DDX_DHtml_ElementInnerText(pDX, _T("Apply"), m_Apply);
	DDX_DHtml_ElementInnerText(pDX, _T("Default"), m_Default);

	DDX_Control(pDX, IDC_SCROLLBAR_05, m_Scrollbar05);
	DDX_Control(pDX, IDC_SCROLLBAR_C5, m_ScrollbarC5);
	DDX_Control(pDX, IDC_SCROLLBAR_C6, m_ScrollbarC6);

	DDX_DHtml_SelectValue(pDX, _T("SelectDisk"), m_SelectDisk);
}

BOOL CHealthDlg::OnInitDialog()
{
	CDHtmlDialogEx::OnInitDialog();

	SetWindowText(i18n(_T("WindowTitle"), _T("HEALTH_STATUS_SETTING"))
		+ _T(" - ") + i18n(_T("HealthStatus"), _T("THRESHOLD_OF_CAUTION"))
		+ _T(" (") + i18n(_T("Dialog"), _T("LIST_RAW_VALUES")) + _T(")"));

	m_Scrollbar05.SetScrollRange(0x00, 0xFF);
	m_ScrollbarC5.SetScrollRange(0x00, 0xFF);
	m_ScrollbarC6.SetScrollRange(0x00, 0xFF);

	EnableDpiAware();
	InitDHtmlDialog(SIZE_X, SIZE_Y, ((CDiskInfoApp*)AfxGetApp())->m_HealthDlgPath);

	return TRUE;
}

BEGIN_MESSAGE_MAP(CHealthDlg, CDHtmlDialogEx)
	ON_WM_HSCROLL()
END_MESSAGE_MAP()

BEGIN_DHTML_EVENT_MAP(CHealthDlg)
	DHTML_EVENT_ONCLICK(_T("SelectDisk"), OnSelectDisk)
	DHTML_EVENT_ONCLICK(_T("Apply"), OnApply)
	DHTML_EVENT_ONCLICK(_T("Default"), OnDefault)
END_DHTML_EVENT_MAP()

void CHealthDlg::OnDocumentComplete(LPDISPATCH pDisp, LPCTSTR szUrl)
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

		m_Scrollbar05.MoveWindow((DWORD)(17 * m_ZoomRatio), (DWORD)(81 * m_ZoomRatio), (DWORD)(328 * m_ZoomRatio), (DWORD)(20 * m_ZoomRatio));
		m_ScrollbarC5.MoveWindow((DWORD)(17 * m_ZoomRatio), (DWORD)(144 * m_ZoomRatio), (DWORD)(328 * m_ZoomRatio), (DWORD)(20 * m_ZoomRatio));
		m_ScrollbarC6.MoveWindow((DWORD)(17 * m_ZoomRatio), (DWORD)(207 * m_ZoomRatio), (DWORD)(328 * m_ZoomRatio), (DWORD)(20 * m_ZoomRatio));

	//	CenterWindow();
		ShowWindow(SW_SHOW);
	}
}

void CHealthDlg::InitLang()
{
	m_Label05 = _T("[05] ") + i18n(_T("Smart"), _T("05"));
	m_LabelC5 = _T("[C5] ") + i18n(_T("Smart"), _T("C5"));
	m_LabelC6 = _T("[C6] ") + i18n(_T("Smart"), _T("C6"));

	m_Apply =   i18n(_T("HealthStatus"), _T("APPLY"));
	m_Default = i18n(_T("HealthStatus"), _T("DEFAULT"));

	UpdateData(FALSE);
}

void CHealthDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
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

	if(*pScrollBar == m_Scrollbar05)
	{
		m_Value05X.Format(_T("%02Xh"), m_Scrollbar05.GetScrollPos());
		m_Value05.Format(_T("%d"), m_Scrollbar05.GetScrollPos());
		if(m_Scrollbar05.GetScrollPos() == 0)
		{
			SetElementPropertyEx(_T("Label05"), DISPID_IHTMLELEMENT_CLASSNAME, _T("labelDisable"));
		}
		else
		{
			SetElementPropertyEx(_T("Label05"), DISPID_IHTMLELEMENT_CLASSNAME, _T("labelEnable"));
		}
	}
	else if(*pScrollBar == m_ScrollbarC5)
	{
		m_ValueC5X.Format(_T("%02Xh"), m_ScrollbarC5.GetScrollPos());
		m_ValueC5.Format(_T("%d"), m_ScrollbarC5.GetScrollPos());
		if(m_ScrollbarC5.GetScrollPos() == 0)
		{
			SetElementPropertyEx(_T("LabelC5"), DISPID_IHTMLELEMENT_CLASSNAME, _T("labelDisable"));
		}
		else
		{
			SetElementPropertyEx(_T("LabelC5"), DISPID_IHTMLELEMENT_CLASSNAME, _T("labelEnable"));
		}
	}
	else if(*pScrollBar == m_ScrollbarC6)
	{
		m_ValueC6X.Format(_T("%02Xh"), m_ScrollbarC6.GetScrollPos());
		m_ValueC6.Format(_T("%d"), m_ScrollbarC6.GetScrollPos());
		if(m_ScrollbarC6.GetScrollPos() == 0)
		{
			SetElementPropertyEx(_T("LabelC6"), DISPID_IHTMLELEMENT_CLASSNAME, _T("labelDisable"));
		}
		else
		{
			SetElementPropertyEx(_T("LabelC6"), DISPID_IHTMLELEMENT_CLASSNAME, _T("labelEnable"));
		}
	}

	UpdateData(FALSE);

	CDHtmlDialogEx::OnHScroll(nSBCode, nPos, pScrollBar);
}

HRESULT CHealthDlg::OnDefault(IHTMLElement* /*pElement*/)
{
	if(! h->m_Ata.vars[m_DiskIndex].IsSsd)
	{
		m_Scrollbar05.SetScrollPos(1);
		m_ScrollbarC5.SetScrollPos(1);
		m_ScrollbarC6.SetScrollPos(1);

		m_Value05.Format(_T("%d"), m_Scrollbar05.GetScrollPos());
		m_ValueC5.Format(_T("%d"), m_ScrollbarC5.GetScrollPos());
		m_ValueC6.Format(_T("%d"), m_ScrollbarC6.GetScrollPos());
		m_Value05X.Format(_T("%02Xh"), m_Scrollbar05.GetScrollPos());
		m_ValueC5X.Format(_T("%02Xh"), m_ScrollbarC5.GetScrollPos());
		m_ValueC6X.Format(_T("%02Xh"), m_ScrollbarC6.GetScrollPos());

		SetElementPropertyEx(_T("Label05"), DISPID_IHTMLELEMENT_CLASSNAME, _T("labelEnable"));
		SetElementPropertyEx(_T("LabelC5"), DISPID_IHTMLELEMENT_CLASSNAME, _T("labelEnable"));
		SetElementPropertyEx(_T("LabelC6"), DISPID_IHTMLELEMENT_CLASSNAME, _T("labelEnable"));

		UpdateData(FALSE);
	}

	return S_FALSE;
}

HRESULT CHealthDlg::OnApply(IHTMLElement* /*pElement*/)
{
	if(! h->m_Ata.vars[m_DiskIndex].IsSsd)
	{
		WritePrivateProfileString(_T("ThreasholdOfCaution05"), h->m_Ata.vars[m_DiskIndex].ModelSerial, m_Value05, m_Ini);
		WritePrivateProfileString(_T("ThreasholdOfCautionC5"), h->m_Ata.vars[m_DiskIndex].ModelSerial, m_ValueC5, m_Ini);
		WritePrivateProfileString(_T("ThreasholdOfCautionC6"), h->m_Ata.vars[m_DiskIndex].ModelSerial, m_ValueC6, m_Ini);
		h->m_Ata.vars[m_DiskIndex].Threshold05 = _tstoi(m_Value05);
		h->m_Ata.vars[m_DiskIndex].ThresholdC5 = _tstoi(m_ValueC5);
		h->m_Ata.vars[m_DiskIndex].ThresholdC6 = _tstoi(m_ValueC6);

		h->SendMessage(WM_COMMAND, ID_REFRESH);
	}

	return S_FALSE;
}

void CHealthDlg::InitSelectDisk()
{
	CString select;
	CString cstr;

	select = _T("<select id=\"SelectDisk\" onchange=\"this.click()\">");

	for(int i = 0; i < h->m_Ata.vars.GetCount(); i++)
	{
		CString temp;
		if(h->m_Ata.vars[i].IsSsd)
		{
			temp = _T("[SSD]");	
		}

		if(i == h->GetSelectDisk())
		{
			cstr.Format(_T("<option value=\"%d\" selected=\"selected\">(%d) %s %s</option>"), i, i + 1, h->m_Ata.vars[i].Model, temp);
		}
		else
		{
			cstr.Format(_T("<option value=\"%d\">(%d) %s %s</option>"), i, i + 1, h->m_Ata.vars[i].Model, temp);
		}
		select += cstr;
	}
	select += _T("</select>");
	SetElementOuterHtmlEx(_T("SelectDisk"), select);

	UpdateData(TRUE);
	m_DiskIndex = _tstoi(m_SelectDisk);
	UpdateSelectDisk(m_DiskIndex);
}

HRESULT CHealthDlg::OnSelectDisk(IHTMLElement* /*pElement*/)
{
	UpdateData(TRUE);

	if(m_DiskIndex != _tstoi(m_SelectDisk))
	{
		m_DiskIndex = _tstoi(m_SelectDisk);
		UpdateSelectDisk(m_DiskIndex);
	}

	return S_FALSE;
}


void CHealthDlg::UpdateSelectDisk(DWORD index)
{
	if(h->m_Ata.vars[index].IsSsd)
	{
		m_Scrollbar05.SetScrollPos(0);
		m_ScrollbarC5.SetScrollPos(0);
		m_ScrollbarC6.SetScrollPos(0);

		m_Scrollbar05.EnableWindow(FALSE);
		m_ScrollbarC5.EnableWindow(FALSE);
		m_ScrollbarC6.EnableWindow(FALSE);

		SetElementPropertyEx(_T("Label05"), DISPID_IHTMLELEMENT_CLASSNAME, _T("labelUnsupported"));
		SetElementPropertyEx(_T("LabelC5"), DISPID_IHTMLELEMENT_CLASSNAME, _T("labelUnsupported"));
		SetElementPropertyEx(_T("LabelC6"), DISPID_IHTMLELEMENT_CLASSNAME, _T("labelUnsupported"));

		SetElementPropertyEx(_T("Default"), DISPID_IHTMLELEMENT_CLASSNAME, _T("buttonDisable"));
		SetElementPropertyEx(_T("Apply"), DISPID_IHTMLELEMENT_CLASSNAME, _T("buttonDisable"));
	}
	else
	{
		m_Scrollbar05.SetScrollPos(GetPrivateProfileInt(_T("ThreasholdOfCaution05"), h->m_Ata.vars[index].ModelSerial, 1, m_Ini));
		m_ScrollbarC5.SetScrollPos(GetPrivateProfileInt(_T("ThreasholdOfCautionC5"), h->m_Ata.vars[index].ModelSerial, 1, m_Ini));
		m_ScrollbarC6.SetScrollPos(GetPrivateProfileInt(_T("ThreasholdOfCautionC6"), h->m_Ata.vars[index].ModelSerial, 1, m_Ini));

		m_Scrollbar05.EnableWindow(TRUE);
		m_ScrollbarC5.EnableWindow(TRUE);
		m_ScrollbarC6.EnableWindow(TRUE);

		if(m_Scrollbar05.GetScrollPos() == 0)
		{
			SetElementPropertyEx(_T("Label05"), DISPID_IHTMLELEMENT_CLASSNAME, _T("labelDisable"));
		}
		else
		{
			SetElementPropertyEx(_T("Label05"), DISPID_IHTMLELEMENT_CLASSNAME, _T("labelEnable"));
		}
		if(m_ScrollbarC5.GetScrollPos() == 0)
		{
			SetElementPropertyEx(_T("LabelC5"), DISPID_IHTMLELEMENT_CLASSNAME, _T("labelDisable"));
		}
		else
		{
			SetElementPropertyEx(_T("LabelC5"), DISPID_IHTMLELEMENT_CLASSNAME, _T("labelEnable"));
		}
		if(m_ScrollbarC6.GetScrollPos() == 0)
		{
			SetElementPropertyEx(_T("LabelC6"), DISPID_IHTMLELEMENT_CLASSNAME, _T("labelDisable"));
		}
		else
		{
			SetElementPropertyEx(_T("LabelC6"), DISPID_IHTMLELEMENT_CLASSNAME, _T("labelEnable"));
		}
		SetElementPropertyEx(_T("Default"), DISPID_IHTMLELEMENT_CLASSNAME, _T("buttonEnable"));
		SetElementPropertyEx(_T("Apply"), DISPID_IHTMLELEMENT_CLASSNAME, _T("buttonEnable"));
	}

	m_Value05.Format(_T("%d"), m_Scrollbar05.GetScrollPos());
	m_ValueC5.Format(_T("%d"), m_ScrollbarC5.GetScrollPos());
	m_ValueC6.Format(_T("%d"), m_ScrollbarC6.GetScrollPos());
	m_Value05X.Format(_T("%02Xh"), m_Scrollbar05.GetScrollPos());
	m_ValueC5X.Format(_T("%02Xh"), m_ScrollbarC5.GetScrollPos());
	m_ValueC6X.Format(_T("%02Xh"), m_ScrollbarC6.GetScrollPos());

	UpdateData(FALSE);
}