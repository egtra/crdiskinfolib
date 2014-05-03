/*---------------------------------------------------------------------------*/
//       Author : hiyohiyo
//         Mail : hiyohiyo@crystalmark.info
//          Web : http://crystalmark.info/
//      License : The modified BSD license
//
//                                Copyright 2008 hiyohiyo. All rights reserved.
/*---------------------------------------------------------------------------*/

#include "stdafx.h"
#include "DiskInfo.h"
#include "GraphDlg.h"
#include "GetOsInfo.h"

#include "DiskInfoDlg.h"


IMPLEMENT_DYNCREATE(CGraphDlg, CDHtmlDialog)

CGraphDlg::CGraphDlg(CWnd* pParent /*=NULL*/, int defaultDisk)
	: CDHtmlMainDialog(CGraphDlg::IDD, CGraphDlg::IDH,
	((CDiskInfoApp*)AfxGetApp())->m_ThemeDir,
	((CDiskInfoApp*)AfxGetApp())->m_ThemeIndex,
	((CDiskInfoApp*)AfxGetApp())->m_LangDir,
	((CDiskInfoApp*)AfxGetApp())->m_LangIndex,
	pParent)
{
	CString cstr;

	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_SmartDir = ((CDiskInfoApp*)AfxGetApp())->m_SmartDir;
	_tcscpy_s(m_Ini, MAX_PATH, ((CDiskInfoApp*)AfxGetApp())->m_Ini.GetString());

	InitVars(defaultDisk);
}

CGraphDlg::~CGraphDlg()
{
	CString index, value;
	for(int i = 0; i < m_DetectedDisk; i++)
	{
		index.Format(_T("Disk%d"), i);
		value.Format(_T("%d"), ! m_FlagGraph[i]);
		WritePrivateProfileString(_T("GraphHideDisk"), index, value, m_Ini);
	}
}

void CGraphDlg::DoDataExchange(CDataExchange* pDX)
{
	CDHtmlMainDialog::DoDataExchange(pDX);

	DDX_DHtml_ElementInnerHtml(pDX, _T("Title"),  m_Title);
	DDX_DHtml_ElementInnerHtml(pDX, _T("Disk0"),  m_LiDisk[0]);
	DDX_DHtml_ElementInnerHtml(pDX, _T("Disk1"),  m_LiDisk[1]);
	DDX_DHtml_ElementInnerHtml(pDX, _T("Disk2"),  m_LiDisk[2]);
	DDX_DHtml_ElementInnerHtml(pDX, _T("Disk3"),  m_LiDisk[3]);
	DDX_DHtml_ElementInnerHtml(pDX, _T("Disk4"),  m_LiDisk[4]);
	DDX_DHtml_ElementInnerHtml(pDX, _T("Disk5"),  m_LiDisk[5]);
	DDX_DHtml_ElementInnerHtml(pDX, _T("Disk6"),  m_LiDisk[6]);
	DDX_DHtml_ElementInnerHtml(pDX, _T("Disk7"),  m_LiDisk[7]);
	DDX_DHtml_ElementInnerHtml(pDX, _T("Disk8"),  m_LiDisk[8]);
	DDX_DHtml_ElementInnerHtml(pDX, _T("Disk9"),  m_LiDisk[9]);
	DDX_DHtml_ElementInnerHtml(pDX, _T("Disk10"), m_LiDisk[10]);
	DDX_DHtml_ElementInnerHtml(pDX, _T("Disk11"), m_LiDisk[11]);
	DDX_DHtml_ElementInnerHtml(pDX, _T("Disk12"), m_LiDisk[12]);
	DDX_DHtml_ElementInnerHtml(pDX, _T("Disk13"), m_LiDisk[13]);
	DDX_DHtml_ElementInnerHtml(pDX, _T("Disk14"), m_LiDisk[14]);
	DDX_DHtml_ElementInnerHtml(pDX, _T("Disk15"), m_LiDisk[15]);
	DDX_DHtml_ElementInnerHtml(pDX, _T("Disk16"), m_LiDisk[16]);
	DDX_DHtml_ElementInnerHtml(pDX, _T("Disk17"), m_LiDisk[17]);
	DDX_DHtml_ElementInnerHtml(pDX, _T("Disk18"), m_LiDisk[18]);
	DDX_DHtml_ElementInnerHtml(pDX, _T("Disk19"), m_LiDisk[19]);
	DDX_DHtml_ElementInnerHtml(pDX, _T("Disk20"), m_LiDisk[20]);
	DDX_DHtml_ElementInnerHtml(pDX, _T("Disk21"), m_LiDisk[21]);
	DDX_DHtml_ElementInnerHtml(pDX, _T("Disk22"), m_LiDisk[22]);
	DDX_DHtml_ElementInnerHtml(pDX, _T("Disk23"), m_LiDisk[23]);
	DDX_DHtml_ElementInnerHtml(pDX, _T("Disk24"), m_LiDisk[24]);
	DDX_DHtml_ElementInnerHtml(pDX, _T("Disk25"), m_LiDisk[25]);
	DDX_DHtml_ElementInnerHtml(pDX, _T("Disk26"), m_LiDisk[26]);
	DDX_DHtml_ElementInnerHtml(pDX, _T("Disk27"), m_LiDisk[27]);
	DDX_DHtml_ElementInnerHtml(pDX, _T("Disk28"), m_LiDisk[28]);
	DDX_DHtml_ElementInnerHtml(pDX, _T("Disk29"), m_LiDisk[29]);
	DDX_DHtml_ElementInnerHtml(pDX, _T("Disk30"), m_LiDisk[30]);
	DDX_DHtml_ElementInnerHtml(pDX, _T("Disk31"), m_LiDisk[31]);

	DDX_DHtml_SelectValue(pDX, _T("SelectAttributeId"), m_SelectAttributeId);
	DDX_DHtml_SelectIndex(pDX, _T("SelectAttributeId"), m_SelectAttributeIdCtrl);
}

BOOL CGraphDlg::OnInitDialog()
{
	CDHtmlMainDialog::OnInitDialog();

	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	InitThemeLang();
	InitMenu();

	SetWindowText(_T("CrystalDiskInfo - ") + i18n(_T("WindowTitle"), _T("GRAPH")) + _T(" - Powered by Flot"));

	m_IeVersion = GetIeVersion();

	InitDHtmlDialog(m_SizeX, m_SizeY, ((CDiskInfoApp*)AfxGetApp())->m_GraphDlgPath);

	return TRUE;
}

BEGIN_MESSAGE_MAP(CGraphDlg, CDHtmlMainDialog)
	ON_WM_GETMINMAXINFO()
	ON_WM_SIZE()
	ON_COMMAND(ID_FILE_EXIT, &CGraphDlg::OnExit)
	ON_COMMAND(ID_NORTH_WEST, &CGraphDlg::OnNorthWest)
	ON_COMMAND(ID_NORTH_EAST, &CGraphDlg::OnNorthEast)
	ON_COMMAND(ID_SOUTH_WEST, &CGraphDlg::OnSouthWest)
	ON_COMMAND(ID_SOUTH_EAST, &CGraphDlg::OnSouthEast)
	ON_COMMAND(ID_PAINT_WEEKEND, &CGraphDlg::OnPaintWeekend)
	ON_COMMAND(ID_POINT_100, &CGraphDlg::OnPoint100)
	ON_COMMAND(ID_POINT_200, &CGraphDlg::OnPoint200)
	ON_COMMAND(ID_POINT_300, &CGraphDlg::OnPoint300)
	ON_COMMAND(ID_POINT_400, &CGraphDlg::OnPoint400)
	ON_COMMAND(ID_POINT_500, &CGraphDlg::OnPoint500)
	ON_COMMAND(ID_POINT_600, &CGraphDlg::OnPoint600)
	ON_COMMAND(ID_POINT_700, &CGraphDlg::OnPoint700)
	ON_COMMAND(ID_POINT_800, &CGraphDlg::OnPoint800)
	ON_COMMAND(ID_POINT_900, &CGraphDlg::OnPoint900)
	ON_COMMAND(ID_POINT_1000, &CGraphDlg::OnPoint1000)
	ON_COMMAND(ID_POINT_2000, &CGraphDlg::OnPoint2000)
	ON_COMMAND(ID_POINT_3000, &CGraphDlg::OnPoint3000)
	ON_COMMAND(ID_POINT_4000, &CGraphDlg::OnPoint4000)
	ON_COMMAND(ID_POINT_5000, &CGraphDlg::OnPoint5000)
	ON_COMMAND(ID_POINT_ALL, &CGraphDlg::OnPointAll)
	ON_COMMAND(ID_CUSTOMIZE, &CGraphDlg::OnCustomize)
	ON_MESSAGE(MY_UPDATE_BG_IMAGE, OnUpdateBgImage)
	ON_MESSAGE(MY_UPDATE_LINE_COLOR, OnUpdateLineColor)
	ON_COMMAND(ID_MDHM, &CGraphDlg::OnMdhm)
	ON_COMMAND(ID_MD, &CGraphDlg::OnMd)
	ON_COMMAND(ID_YMDHM, &CGraphDlg::OnYmdhm)
	ON_COMMAND(ID_YMD, &CGraphDlg::OnYmd)
	ON_COMMAND(ID_DMYHM, &CGraphDlg::OnDmyhm)
	ON_COMMAND(ID_DMY, &CGraphDlg::OnDmy)
	ON_COMMAND(ID_DMYHM2, &CGraphDlg::OnDmyhm2)
	ON_COMMAND(ID_DMY2, &CGraphDlg::OnDmy2)
END_MESSAGE_MAP()

BEGIN_DHTML_EVENT_MAP(CGraphDlg)
	DHTML_EVENT_ONCLICK(_T("SelectAttributeId"), OnSelectAttributeId)
	DHTML_EVENT_ONCLICK(_T("AllOn"), OnAllOn)
	DHTML_EVENT_ONCLICK(_T("AllOff"), OnAllOff)
	DHTML_EVENT_ONCLICK(_T("Reset"), OnReset)

	DHTML_EVENT_ONCLICK(_T("Disk0"),  OnDisk0)
	DHTML_EVENT_ONCLICK(_T("Disk1"),  OnDisk1)
	DHTML_EVENT_ONCLICK(_T("Disk2"),  OnDisk2)
	DHTML_EVENT_ONCLICK(_T("Disk3"),  OnDisk3)
	DHTML_EVENT_ONCLICK(_T("Disk4"),  OnDisk4)
	DHTML_EVENT_ONCLICK(_T("Disk5"),  OnDisk5)
	DHTML_EVENT_ONCLICK(_T("Disk6"),  OnDisk6)
	DHTML_EVENT_ONCLICK(_T("Disk7"),  OnDisk7)
	DHTML_EVENT_ONCLICK(_T("Disk8"),  OnDisk8)
	DHTML_EVENT_ONCLICK(_T("Disk9"),  OnDisk9)
	DHTML_EVENT_ONCLICK(_T("Disk10"), OnDisk10)
	DHTML_EVENT_ONCLICK(_T("Disk11"), OnDisk11)
	DHTML_EVENT_ONCLICK(_T("Disk12"), OnDisk12)
	DHTML_EVENT_ONCLICK(_T("Disk13"), OnDisk13)
	DHTML_EVENT_ONCLICK(_T("Disk14"), OnDisk14)
	DHTML_EVENT_ONCLICK(_T("Disk15"), OnDisk15)
	DHTML_EVENT_ONCLICK(_T("Disk16"), OnDisk16)
	DHTML_EVENT_ONCLICK(_T("Disk17"), OnDisk17)
	DHTML_EVENT_ONCLICK(_T("Disk18"), OnDisk18)
	DHTML_EVENT_ONCLICK(_T("Disk19"), OnDisk19)
	DHTML_EVENT_ONCLICK(_T("Disk20"), OnDisk20)
	DHTML_EVENT_ONCLICK(_T("Disk21"), OnDisk21)
	DHTML_EVENT_ONCLICK(_T("Disk22"), OnDisk22)
	DHTML_EVENT_ONCLICK(_T("Disk23"), OnDisk23)
	DHTML_EVENT_ONCLICK(_T("Disk24"), OnDisk24)
	DHTML_EVENT_ONCLICK(_T("Disk25"), OnDisk25)
	DHTML_EVENT_ONCLICK(_T("Disk26"), OnDisk26)
	DHTML_EVENT_ONCLICK(_T("Disk27"), OnDisk27)
	DHTML_EVENT_ONCLICK(_T("Disk28"), OnDisk28)
	DHTML_EVENT_ONCLICK(_T("Disk29"), OnDisk29)
	DHTML_EVENT_ONCLICK(_T("Disk30"), OnDisk30)
	DHTML_EVENT_ONCLICK(_T("Disk31"), OnDisk31)
END_DHTML_EVENT_MAP()

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CGraphDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDHtmlMainDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CGraphDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CGraphDlg::OnDocumentComplete(LPDISPATCH pDisp, LPCTSTR szUrl)
{
	CString cstr;
	cstr = szUrl;
	if(cstr.Find(_T("html")) != -1 || cstr.Find(_T("dlg")) != -1)
	{
		m_FlagShowWindow = TRUE;

		InitMenuBar();
		UpdateBgImage();

		for(int i = 0; i < m_DetectedDisk; i++)
		{
			if(m_DefaultDisk[i])
			{
				CheckDisk(i);
			}
		}
		m_FlagInitializing = FALSE;

		UpdateData(TRUE);
		m_AttributeId = _tstoi(m_SelectAttributeId);

		UpdateDialogSize();
		UpdateGraph();
		CenterWindow();

		ShowWindow(SW_SHOW);
	}
}

void CGraphDlg::InitVars(int defaultDisk)
{
	TCHAR str[256];
	CString cstr;
	int temp;

	GetPrivateProfileString(_T("EXCHANGE"), _T("DetectedDisk"), _T("-1"), str, 256, m_SmartDir + _T("\\") + EXCHANGE_INI);
	temp = _tstoi(str);
	if(0 < temp && temp <= CAtaSmart::MAX_DISK)
	{
		m_DetectedDisk = temp;
	}
	else
	{
		m_DetectedDisk = 0;
	}

	for(int i = 0; i < CAtaSmart::MAX_DISK; i++)
	{
		m_FlagGraph[i] = FALSE;
		m_DefaultDisk[i] = FALSE;
	}

	if(0 <= defaultDisk && defaultDisk < m_DetectedDisk)
	{
		m_DefaultDisk[defaultDisk] = TRUE;
	}
	else
	{
		for(int i = 0; i < m_DetectedDisk; i++)
		{
			cstr.Format(_T("Disk%d"), i);
			if(GetPrivateProfileInt(_T("GraphHideDisk"), cstr, 0, m_Ini) == 0)
			{
				m_DefaultDisk[i] = TRUE;
			}
		}
	}

	for(int i = 0; i < m_DetectedDisk; i++)
	{
		cstr.Format(_T("%d"), i);
		GetPrivateProfileString(_T("MODEL"), cstr, _T(""), str, 256, m_SmartDir + _T("\\") + EXCHANGE_INI);
		m_Model[i] = str;
		cstr.Format(_T("%d"), i);
		GetPrivateProfileString(_T("SERIAL"), cstr, _T(""), str, 256, m_SmartDir + _T("\\") + EXCHANGE_INI);
		m_Serial[i] = str;
	}

	// LegendPosition
	GetPrivateProfileString(_T("Setting"), _T("LegendPosition"), _T("sw"), str, 256, m_Ini);
	cstr = str;
	if(cstr.Compare(_T("nw")) == 0 || cstr.Compare(_T("ne")) == 0 || cstr.Compare(_T("sw")) == 0 || cstr.Compare(_T("se")) == 0)
	{
		m_LegendPositon = str;
	}
	else
	{
		m_LegendPositon = _T("sw");
	}

	// Date Style
	GetPrivateProfileString(_T("Setting"), _T("TimeFormat"), _T("mdhm"), str, 256, m_Ini);
	cstr = str;
	if(cstr.Compare(_T("%m/%d %H:%M")) == 0
	|| cstr.Compare(_T("%m/%d")) == 0
	|| cstr.Compare(_T("%y/%m/%d %H:%M")) == 0
	|| cstr.Compare(_T("%y/%m/%d")) == 0
	|| cstr.Compare(_T("%d/%m/%y")) == 0
	|| cstr.Compare(_T("%d/%m/%y %H:%M")) == 0
	|| cstr.Compare(_T("%d.%m.%y")) == 0
	|| cstr.Compare(_T("%d.%m.%y %H:%M")) == 0)
	{
		m_TimeFormat = str;
	}
	else
	{
		m_TimeFormat = _T("%m/%d %H:%M");
	}

	// Paint Weekend
	GetPrivateProfileString(_T("Setting"), _T("PaintWeekend"), _T("0"), str, 256, m_Ini);
	if(_tstoi(str) > 0)
	{
		m_FlagPaintWeekend = TRUE;
	}
	else
	{
		m_FlagPaintWeekend = FALSE;
	}

	GetPrivateProfileString(_T("Setting"), _T("MaxPlotPoint"), _T("100"), str, 256, m_Ini);
	if(_tstoi(str) > 0)
	{
		m_MaxPlotPoint = _tstoi(str);
	}
	else
	{
		m_MaxPlotPoint = 0;
	}

	// Graph Color
	m_DefaultLineColor[0] = 0xff9797;
	m_DefaultLineColor[1] = 0x40c2ed;
	m_DefaultLineColor[2] = 0x4b4bcb;
	m_DefaultLineColor[3] = 0xc6ac8c;
	m_DefaultLineColor[4] = 0x4da74d;
	m_DefaultLineColor[5] = 0xaa1c4a;
	m_DefaultLineColor[6] = 0x1191bb;
	m_DefaultLineColor[7] = 0x47189e;
	m_DefaultLineColor[8] = 0xc17453;
	m_DefaultLineColor[9] = 0x1a7612;
	m_DefaultLineColor[10] = 0xff6c35;
	m_DefaultLineColor[11] = 0x24676f;
	m_DefaultLineColor[12] = 0x0b29a4;
	m_DefaultLineColor[13] = 0xd2958c;
	m_DefaultLineColor[14] = 0x575700;
	m_DefaultLineColor[15] = 0x808080;
	m_DefaultLineColor[16] = 0x787878;
	m_DefaultLineColor[17] = 0x707070;
	m_DefaultLineColor[18] = 0x686868;
	m_DefaultLineColor[19] = 0x606060;
	m_DefaultLineColor[20] = 0x585858;
	m_DefaultLineColor[21] = 0x505050;
	m_DefaultLineColor[22] = 0x484848;
	m_DefaultLineColor[23] = 0x404040;
	m_DefaultLineColor[24] = 0x383838;
	m_DefaultLineColor[25] = 0x303030;
	m_DefaultLineColor[26] = 0x282828;
	m_DefaultLineColor[27] = 0x202020;
	m_DefaultLineColor[28] = 0x181818;
	m_DefaultLineColor[29] = 0x101010;
	m_DefaultLineColor[30] = 0x080808;
	m_DefaultLineColor[31] = 0x000000;
	m_DefaultLineColor[CAtaSmart::MAX_DISK] = 0x4a24ff;	// Threshold

	switch(GetPrivateProfileInt(_T("Setting"), _T("Temperature"), 0, m_Ini))
	{
	case   1:	m_FlagFahrenheit = TRUE; break;
	default:	m_FlagFahrenheit = FALSE;break;
	}

	m_FlagInitializing = TRUE;

	UpdateColor();
}

void CGraphDlg::UpdateColor()
{
	for(int i = 0; i < m_DetectedDisk; i++)
	{
		m_LineColor[i] = GetLineColor(i);
	}
	// Threshold
	m_LineColor[CAtaSmart::MAX_DISK] = GetLineColor(CAtaSmart::MAX_DISK);
}

COLORREF CGraphDlg::GetLineColor(DWORD index)
{
	TCHAR str[256];
	CString cstr;
	cstr.Format(_T("%d"), index);
	GetPrivateProfileString(_T("LineColor"), cstr, _T("-1"), str, 256, m_Ini);

	if(str[0] == '#')
	{
		int r, g, b;
		cstr = str;
		r = _tcstol(cstr.Mid(1,2), NULL, 16);
		g = _tcstol(cstr.Mid(3,2), NULL, 16);
		b = _tcstol(cstr.Mid(5,2), NULL, 16);
		return RGB(r, g, b);
	}
	else if(_tstoi(str) <= -1 || _tstoi(str) > 0x00FFFFFF)
	{
		return m_DefaultLineColor[index];
	}
	else
	{
		return _tstoi(str);
	}
}

HRESULT CGraphDlg::OnSelectAttributeId(IHTMLElement* /*pElement*/)
{
	UpdateData(TRUE);

	if(m_AttributeId != _tstoi(m_SelectAttributeId))
	{
		m_AttributeId = _tstoi(m_SelectAttributeId);
		UpdateGraph();
		CString cstr;
		cstr.Format(_T("%d"), m_AttributeId);
		WritePrivateProfileString(_T("Setting"), _T("SelectedAttributeId"), cstr, m_Ini);
	}

	return S_FALSE;
}

HRESULT CGraphDlg::OnDisk0(IHTMLElement* /*pElement*/){CheckDisk(0);UpdateGraph();return S_FALSE;}
HRESULT CGraphDlg::OnDisk1(IHTMLElement* /*pElement*/){CheckDisk(1);UpdateGraph();return S_FALSE;}
HRESULT CGraphDlg::OnDisk2(IHTMLElement* /*pElement*/){CheckDisk(2);UpdateGraph();return S_FALSE;}
HRESULT CGraphDlg::OnDisk3(IHTMLElement* /*pElement*/){CheckDisk(3);UpdateGraph();return S_FALSE;}
HRESULT CGraphDlg::OnDisk4(IHTMLElement* /*pElement*/){CheckDisk(4);UpdateGraph();return S_FALSE;}
HRESULT CGraphDlg::OnDisk5(IHTMLElement* /*pElement*/){CheckDisk(5);UpdateGraph();return S_FALSE;}
HRESULT CGraphDlg::OnDisk6(IHTMLElement* /*pElement*/){CheckDisk(6);UpdateGraph();return S_FALSE;}
HRESULT CGraphDlg::OnDisk7(IHTMLElement* /*pElement*/){CheckDisk(7);UpdateGraph();return S_FALSE;}

HRESULT CGraphDlg::OnDisk8(IHTMLElement* /*pElement*/){CheckDisk(8);UpdateGraph();return S_FALSE;}
HRESULT CGraphDlg::OnDisk9(IHTMLElement* /*pElement*/){CheckDisk(9);UpdateGraph();return S_FALSE;}
HRESULT CGraphDlg::OnDisk10(IHTMLElement* /*pElement*/){CheckDisk(10);UpdateGraph();return S_FALSE;}
HRESULT CGraphDlg::OnDisk11(IHTMLElement* /*pElement*/){CheckDisk(11);UpdateGraph();return S_FALSE;}
HRESULT CGraphDlg::OnDisk12(IHTMLElement* /*pElement*/){CheckDisk(12);UpdateGraph();return S_FALSE;}
HRESULT CGraphDlg::OnDisk13(IHTMLElement* /*pElement*/){CheckDisk(13);UpdateGraph();return S_FALSE;}
HRESULT CGraphDlg::OnDisk14(IHTMLElement* /*pElement*/){CheckDisk(14);UpdateGraph();return S_FALSE;}
HRESULT CGraphDlg::OnDisk15(IHTMLElement* /*pElement*/){CheckDisk(15);UpdateGraph();return S_FALSE;}

HRESULT CGraphDlg::OnDisk16(IHTMLElement* /*pElement*/){CheckDisk(16);UpdateGraph();return S_FALSE;}
HRESULT CGraphDlg::OnDisk17(IHTMLElement* /*pElement*/){CheckDisk(17);UpdateGraph();return S_FALSE;}
HRESULT CGraphDlg::OnDisk18(IHTMLElement* /*pElement*/){CheckDisk(18);UpdateGraph();return S_FALSE;}
HRESULT CGraphDlg::OnDisk19(IHTMLElement* /*pElement*/){CheckDisk(19);UpdateGraph();return S_FALSE;}
HRESULT CGraphDlg::OnDisk20(IHTMLElement* /*pElement*/){CheckDisk(20);UpdateGraph();return S_FALSE;}
HRESULT CGraphDlg::OnDisk21(IHTMLElement* /*pElement*/){CheckDisk(21);UpdateGraph();return S_FALSE;}
HRESULT CGraphDlg::OnDisk22(IHTMLElement* /*pElement*/){CheckDisk(22);UpdateGraph();return S_FALSE;}
HRESULT CGraphDlg::OnDisk23(IHTMLElement* /*pElement*/){CheckDisk(23);UpdateGraph();return S_FALSE;}

HRESULT CGraphDlg::OnDisk24(IHTMLElement* /*pElement*/){CheckDisk(24);UpdateGraph();return S_FALSE;}
HRESULT CGraphDlg::OnDisk25(IHTMLElement* /*pElement*/){CheckDisk(25);UpdateGraph();return S_FALSE;}
HRESULT CGraphDlg::OnDisk26(IHTMLElement* /*pElement*/){CheckDisk(26);UpdateGraph();return S_FALSE;}
HRESULT CGraphDlg::OnDisk27(IHTMLElement* /*pElement*/){CheckDisk(27);UpdateGraph();return S_FALSE;}
HRESULT CGraphDlg::OnDisk28(IHTMLElement* /*pElement*/){CheckDisk(28);UpdateGraph();return S_FALSE;}
HRESULT CGraphDlg::OnDisk29(IHTMLElement* /*pElement*/){CheckDisk(29);UpdateGraph();return S_FALSE;}
HRESULT CGraphDlg::OnDisk30(IHTMLElement* /*pElement*/){CheckDisk(30);UpdateGraph();return S_FALSE;}
HRESULT CGraphDlg::OnDisk31(IHTMLElement* /*pElement*/){CheckDisk(31);UpdateGraph();return S_FALSE;}

HRESULT CGraphDlg::OnAllOn(IHTMLElement* /*pElement*/)
{
	VARIANT dummy;
	VariantInit(&dummy);
	dummy.vt = VT_BSTR;
	CString cstr;

	for(int i = 0; i < m_DetectedDisk; i++)
	{
		cstr.Format(_T("Disk%d"), i);
		m_FlagGraph[i] = TRUE;
		SetElementPropertyEx(cstr, DISPID_IHTMLELEMENT_CLASSNAME, &dummy, _T("selected"));
	}

	UpdateGraph();

	return S_FALSE;
}

HRESULT CGraphDlg::OnAllOff(IHTMLElement* /*pElement*/)
{
	VARIANT dummy;
	VariantInit(&dummy);
	dummy.vt = VT_BSTR;
	CString cstr;

	for(int i = 0; i < m_DetectedDisk; i++)
	{
		cstr.Format(_T("Disk%d"), i);
		m_FlagGraph[i] = FALSE;
		SetElementPropertyEx(cstr, DISPID_IHTMLELEMENT_CLASSNAME, &dummy, _T(""));
	}
	UpdateGraph();

	return S_FALSE;
}

HRESULT CGraphDlg::OnReset(IHTMLElement* /*pElement*/)
{
	UpdateGraph();
	return S_FALSE;
}

BOOL CGraphDlg::CheckDisk(DWORD disk)
{
	VARIANT dummy;
	VariantInit(&dummy);
	dummy.vt = VT_BSTR;
	CString cstr;

	m_FlagGraph[disk] = ! m_FlagGraph[disk];

	cstr.Format(_T("Disk%d"), disk);
	if(m_FlagGraph[disk])
	{
		SetElementPropertyEx(cstr, DISPID_IHTMLELEMENT_CLASSNAME, &dummy, _T("selected"));
	}
	else
	{
		SetElementPropertyEx(cstr, DISPID_IHTMLELEMENT_CLASSNAME, &dummy, _T(""));
	}

	return TRUE;
}

void CGraphDlg::InitMenuBar()
{
	VARIANT dummy;
	VariantInit(&dummy);
	dummy.vt = VT_BSTR;
	CString cstr, temp, space;

	int counter = 0, index = 0;
	m_SelectAttributeIdCtrl = 0;
	DWORD SelectedAttributeId = GetPrivateProfileInt(_T("Setting"), _T("SelectedAttributeId"), 0, m_Ini);

	for(int i = 0; i < CAtaSmart::MAX_DISK; i++)
	{
		m_LiDisk[i] = _T("");
		cstr.Format(_T("Disk%d"), i);
		SetElementPropertyEx(cstr, DISPID_IHTMLELEMENT_CLASSNAME, &dummy, _T("hidden"));
	}

	for(int i = 0; i < m_DetectedDisk; i++)
	{
		cstr.Format(_T("%d"), i + 1);
		m_LiDisk[i] = cstr;
		cstr.Format(_T("Disk%d"), i);
		SetElementPropertyEx(cstr, DISPID_IHTMLELEMENT_CLASSNAME, &dummy, _T("visible"));
		
		temp.Format(_T("%s"), m_Model[i]);
		SetElementPropertyEx(cstr, DISPID_IHTMLELEMENT_TITLE, &dummy, temp);
	}

	SetElementPropertyEx(_T("AllOn"), DISPID_IHTMLELEMENT_TITLE, &dummy, i18n(_T("Graph"), _T("ALL_ON")));
	SetElementPropertyEx(_T("AllOff"), DISPID_IHTMLELEMENT_TITLE, &dummy, i18n(_T("Graph"), _T("ALL_OFF")));
	SetElementPropertyEx(_T("Reset"), DISPID_IHTMLELEMENT_TITLE, &dummy, i18n(_T("Graph"), _T("RESET")));

	CString select;

	cstr.Format(_T("<select id=\"SelectAttributeId\" title=\"%s\" onchange=\"this.click()\">"), i18n(_T("Graph"), _T("PLEASE_SELECT_ITEM")));
	select = cstr;

	if(m_IeVersion >= 700)
	{
		cstr.Format(_T("<optgroup label=\"%s\">"), i18n(_T("Graph"), _T("ACTUAL_VALUE")));
		select += cstr;
	}
	else
	{
		cstr.Format(_T("<option value=\"513\">%s</option>"), i18n(_T("Graph"), _T("ACTUAL_VALUE")));
		select += cstr;
		space = _T("&nbsp;&nbsp;");
		if(SelectedAttributeId == 513)
		{
			index = counter;
		}
		counter++;
	}
	
	BOOL flagAttribute[256] = {0};
	CString dir, disk;
	TCHAR str[256];
	for(int i = 0; i < m_DetectedDisk; i++)
	{
		dir = m_SmartDir;
		disk = m_Model[i] + m_Serial[i];
		dir += disk;
		for(int j = 1; j < 255; j++)
		{
			cstr.Format(_T("%02X"), j);
			GetPrivateProfileString(disk, cstr, _T("-1"), str, 256, dir + _T("\\") + SMART_INI);
			if(_tstoi(str) >= 0)
			{
				flagAttribute[j] = TRUE;
			}
		}
	}
	
	// Reallocated Sectors Count
	if(flagAttribute[0x05])
	{
		cstr.Format(_T("<option value=\"261\" selected=\"selected\">%s[05] %s</option>"), space, i18n(_T("Smart"), _T("05")));
		select += cstr;
		if(SelectedAttributeId == 261)
		{
			index = counter;
		}
		counter++;
	}
	// PowerOnHours
	if(flagAttribute[0x09])
	{
		cstr.Format(_T("<option value=\"265\">%s[09] %s</option>"), space, i18n(_T("Smart"), _T("09")));
		select += cstr;
		if(SelectedAttributeId == 265)
		{
			index = counter;
		}
		counter++;
	}
	// PowerOnCount
	if(flagAttribute[0x0C])
	{
		cstr.Format(_T("<option value=\"268\">%s[0C] %s</option>"), space, i18n(_T("Smart"), _T("0C")));
		select += cstr;
		if(SelectedAttributeId == 268)
		{
			index = counter;
		}
		counter++;
	}
	// Temperature
	if(flagAttribute[0xC2])
	{
		if(m_FlagFahrenheit)
		{
			cstr.Format(_T("<option value=\"451\">%s[C2] %s (ÅãF)</option>"), space, i18n(_T("Smart"), _T("C2")));
			if(SelectedAttributeId == 451)
			{
				index = counter;
			}
		}
		else
		{
			cstr.Format(_T("<option value=\"450\">%s[C2] %s (ÅãC)</option>"), space, i18n(_T("Smart"), _T("C2")));
			if(SelectedAttributeId == 450)
			{
				index = counter;
			}
		}
		select += cstr;
		counter++;
	}
	// Reallocation Event Count
	if(flagAttribute[0xC4])
	{
		cstr.Format(_T("<option value=\"452\">%s[C4] %s</option>"), space, i18n(_T("Smart"), _T("C4")));
		select += cstr;
		if(SelectedAttributeId == 452)
		{
			index = counter;
		}
		counter++;
	}
	// Current Pending Sector Count
	if(flagAttribute[0xC5])
	{
		cstr.Format(_T("<option value=\"453\">%s[C5] %s</option>"), space, i18n(_T("Smart"), _T("C5")));
		select += cstr;
		if(SelectedAttributeId == 453)
		{
			index = counter;
		}
		counter++;
	}
	// Off-Line Scan Uncorrectable Sector Count
	if(flagAttribute[0xC6])
	{
		cstr.Format(_T("<option value=\"454\">%s[C6] %s</option>"), space, i18n(_T("Smart"), _T("C6")));
		select += cstr;
		if(SelectedAttributeId == 454)
		{
			index = counter;
		}
		counter++;
	}

	if(m_IeVersion >= 700)
	{
		cstr.Format(_T("<optgroup label=\"%s\">"), i18n(_T("Graph"), _T("NORMALIZED_VALUE")));
		select += cstr;
	}
	else
	{
		cstr.Format(_T("<option value=\"514\">%s</option>"), i18n(_T("Graph"), _T("NORMALIZED_VALUE")));
		select += cstr;
		if(SelectedAttributeId == 514)
		{
			index = counter;
		}
		counter++;
	}

	for(int i = 1; i < 255; i++)
	{
		if(flagAttribute[i])
		{
			cstr.Format(_T("%02X"), i);
			if(i18n(_T("Smart"), cstr).IsEmpty())
			{
				cstr.Format(_T("<option value=\"%d\">%s(%02X) %s</option>"), i, space, i, i18n(_T("Smart"), _T("UNKNOWN")));
			}
			else
			{
				cstr.Format(_T("<option value=\"%d\">%s(%02X) %s</option>"), i, space, i, i18n(_T("Smart"), cstr));
			}
			if(SelectedAttributeId == i)
			{
				index = counter;;
			}
			select += cstr;
			counter++;
		}
	}

	select += _T("</select>");
	SetElementOuterHtmlEx(_T("SelectAttributeId"), select);

	m_SelectAttributeIdCtrl = index;
	UpdateData(FALSE);
}

BOOL CGraphDlg::UpdateGraph()
{
	CString cstr, line, values, thresholds, maxMin, points;
	CString smartFile, fileName;
	CStdioFile inFile;
	DWORD threshold = 0;
	int max = -1, min = -1;

	if(m_AttributeId > 0x200)
	{
		CallScript(_T("updateData"), _T("[]"));
	//	CallScript(_T("reDraw"), NULL);
		CallScript(_T("changeSize"), NULL); // Redraw
		return FALSE;
	}
	else if(m_AttributeId < 0x100)
	{
		fileName.Format(_T("%02X"), m_AttributeId);
		max = 255;
		min = 0;
	}
	else if(m_AttributeId >= 0x100)
	{
		switch(m_AttributeId)
		{
		case 0x1C2:fileName = _T("Temperature");				min = 0;break;
		case 0x1C3:fileName = _T("Temperature");				min = 0;break;
		case 0x109:fileName = _T("PowerOnHours");				break;
		case 0x10C:fileName = _T("PowerOnCount");				break;
		case 0x105:fileName = _T("ReallocatedSectorsCount");	min = 0; break;
		case 0x1C4:fileName = _T("ReallocationEventCount");		min = 0; break;
		case 0x1C5:fileName = _T("CurrentPendingSectorCount");	min = 0; break;
		case 0x1C6:fileName = _T("UncorrectableSectorCount");	min = 0; break;
		default:
			return FALSE;
			break;
		}
	}
	else
	{
		return FALSE;
	}

	// Update Time Zone Information
	GetTimeZoneInformation(&m_TimeZoneInformation);

	UpdateColor();

	DWORD value = 0;
	time_t dateTime;
	ULONGLONG startTime = 0;
	int count = 0;
	int drive = 0;
	int index = 0; // Only One Disk
	CString arg;

	arg = _T("[");

	for(int i = 0; i < m_DetectedDisk; i++)
	{
		if(m_FlagGraph[i] == FALSE)
		{
			continue;
		}
		count = 0;
		values = _T("");

		smartFile = m_SmartDir + _T("\\");
		smartFile += m_Model[i] + m_Serial[i];
		smartFile.Replace(_T("/"), _T(""));
		smartFile += _T("\\") + fileName + _T(".csv");
		if(! inFile.Open(smartFile, CFile::modeRead | CFile::typeText))
		{
			continue;
		}
		else
		{
			CStringArray lines;
			while(inFile.ReadString(line) != NULL)
			{
				if(line.GetLength() > 0)
				{
					lines.Add(line);
				}
			}
			inFile.Close();

			int end = (int)lines.GetCount();
			int start = 0;

			if(m_MaxPlotPoint == 0)
			{
				start = 0;
			}
			else if(end > (int)m_MaxPlotPoint)
			{
				start = end - m_MaxPlotPoint;
			}

			if(m_AttributeId == 0x1C3) // Fahrenheit
			{
				for(int i = start; i < end; i++)
				{
					line = lines.GetAt(i);
					dateTime = GetTimeT(line.Left(19));
					if(count != 0)
					{
						cstr.Format(_T("[%I64d, %d], "), dateTime * 1000, value * 9 / 5 + 32); 
						values += cstr;
					}
					if(count == 0)
					{
						startTime = dateTime * 1000;
					}

					value = _tstoi(line.Mid(20));
					cstr.Format(_T("[%I64d, %d], "), dateTime * 1000, value * 9 / 5 + 32); 
					values += cstr;
					count++;
				}
			}
			else
			{
				for(int i = start; i < end; i++)
				{
					line = lines.GetAt(i);
					dateTime = GetTimeT(line.Left(19));
					if(count != 0)
					{
						cstr.Format(_T("[%I64d, %d], "), dateTime * 1000, value); 
						values += cstr;
					}
					if(count == 0)
					{
						startTime = dateTime * 1000;
					}

					value = _tstoi(line.Mid(20));
					cstr.Format(_T("[%I64d, %d], "), dateTime * 1000, value); 
					values += cstr;
					count++;
				}
			}
		}
		drive++;
		index = i;

		// Latest
		_tzset();
		if(m_AttributeId == 0x1C3) // Fahrenheit
		{
			cstr.Format(_T("[%I64d, %d]"), (ULONGLONG)(time(&dateTime) - m_TimeZoneInformation.Bias * 60) * 1000, value * 9 / 5 + 32);
		}
		else
		{
			cstr.Format(_T("[%I64d, %d]"), (ULONGLONG)(time(&dateTime) - m_TimeZoneInformation.Bias * 60) * 1000, value);
		}
		values += cstr;
/*		cstr.Format(_T("{label: \"(%d) %s\", data:[%s]}, "),
			i + 1, m_Model[i],
			values);
*/
		cstr.Format(_T("{label: \"(%d) %s\", color: \"rgb(%d, %d, %d)\", data:[%s]}, "),
			i + 1, m_Model[i],
			GetRValue(m_LineColor[i]), GetGValue(m_LineColor[i]), GetBValue(m_LineColor[i]),
			values);

	//	cstr.Format(_T("{label: \"%s\", color: \"rgb(%d, %d, %d)\", data:[%s]}, "),
	//		m_Ata->vars[i].Model, rand() % 256, rand() % 256, rand() % 256, values);
		arg += cstr;
	}

	if(m_AttributeId < 0x100 && drive == 1)
	{
		CString dir, disk;
		dir = m_SmartDir;
		disk = m_Model[index] + m_Serial[index];
		dir += disk;
		TCHAR str[256];
		cstr.Format(_T("%02X"), m_AttributeId);
		GetPrivateProfileString(disk + _T("THRESHOLD"), cstr, _T("-1"), str, 256, dir + _T("\\") + SMART_INI);
		threshold = _tstoi(str);
		if(threshold >= 0)
		{
			cstr.Format(_T("{label: \"%s\", color: \"rgb(%d, %d, %d)\", data:["),
				i18n(_T("Dialog"), _T("LIST_THRESHOLD")),
				GetRValue(m_LineColor[CAtaSmart::MAX_DISK]),
				GetGValue(m_LineColor[CAtaSmart::MAX_DISK]),
				GetBValue(m_LineColor[CAtaSmart::MAX_DISK])
				);
			arg += cstr;
			cstr.Format(_T("[%I64d, %d], "), startTime, threshold); 
			arg += cstr; 
			cstr.Format(_T("[%I64d, %d]"), (ULONGLONG)(time(&dateTime) - m_TimeZoneInformation.Bias * 60) * 1000, threshold); 
			arg += cstr;
			arg += _T("]}, ");
		}
	}

	if(arg.GetLength() > 3)
	{
		arg.Delete(arg.GetLength() - 2, 2);
	}
	arg += _T("]");

	CString options, overViewOptions, grid;

	if(m_FlagPaintWeekend)
	{
		grid = _T("grid: { hoverable: true, clickable: true, coloredAreas: weekendAreas },");
	}
	else
	{
		grid = _T("grid: { hoverable: true, clickable: true },");
	}

	// Options
	if(max == -1 && min == -1) // Auto
	{
	options.Format(_T("{\
xaxis: { mode: \"time\", ticks: 4, timeformat: \"%s\", minTickSize: [60, \"minute\"]},\
yaxis: { minTickSize: 1, min: null, max: null },\
selection: { mode: \"xy\" },\
%s\
legend: { position: \"%s\", margin: 20 },\
lines: { show: true }\
}"), m_TimeFormat, grid, m_LegendPositon);
	}
	else if(max == -1)
	{
	options.Format(_T("{\
xaxis: { mode: \"time\", ticks: 4, timeformat: \"%s\", minTickSize: [60, \"minute\"]},\
yaxis: { minTickSize: 1, min: %d, max: null },\
selection: { mode: \"xy\" },\
%s\
legend: { position: \"%s\", margin: 20 },\
lines: { show: true }\
%s\
}"), m_TimeFormat, min, grid, m_LegendPositon, points);
	}
	else
	{
	options.Format(_T("{\
xaxis: { mode: \"time\", ticks: 4, timeformat: \"%s\", minTickSize: [60, \"minute\"]},\
yaxis: { minTickSize: 1, min: %d, max: %d },\
selection: { mode: \"xy\" },\
%s\
legend: { position: \"%s\", margin: 20 },\
lines: { show: true }\
%s\
}"), m_TimeFormat, min, max, grid, m_LegendPositon, points);
	}

	// overViewOptions
	if(max == -1 && min == -1)
	{
	overViewOptions.Format(_T("{\
xaxis: { ticks: [], mode: \"time\" },\
yaxis: { ticks: [], min: 0, max: null  },\
selection: { mode: \"x\" },\
%s\
legend: { show: false },\
lines: { show: true }\
}"), grid);
	}
	else if(max == -1)
	{
	overViewOptions.Format(_T("{\
xaxis: { ticks: [], mode: \"time\" },\
yaxis: { ticks: [], min: 0, max: null  },\
selection: { mode: \"x\" },\
%s\
legend: { show: false },\
lines: { show: true }\
}"), grid);
	}
	else
	{
	overViewOptions.Format(_T("{\
xaxis: { ticks: [], mode: \"time\" },\
yaxis: { ticks: [], min: %d, max: %d },\
selection: { mode: \"x\" },\
%s\
legend: { show: false },\
lines: { show: true }\
}"), min, max, grid);
	}
	CallScript(_T("updateData"), arg);
	CallScript(_T("updateMainViewOptions"), options);
	CallScript(_T("updateOverViewOptions"), overViewOptions);
//	CallScript(_T("reDraw"), NULL);
	CallScript(_T("changeSize"), NULL); // Redraw

	if(m_AttributeId >= 0x100)
	{
		cstr.Format(_T("%02X"), m_AttributeId % 256);
		m_Title.Format(_T("%s"), i18n(_T("Smart"), cstr));
	}
	else
	{
		cstr.Format(_T("%02X"), m_AttributeId);
		m_Title.Format(_T("(%02X) %s"), m_AttributeId, i18n(_T("Smart"), cstr));
	}

	UpdateData(FALSE);
	return TRUE;
}


void CGraphDlg::InitMenu()
{
	CString cstr, temp;
	CMenu *menu = GetMenu();
	CMenu subMenu;

	cstr = i18n(_T("Menu"), _T("FILE"));
	menu->ModifyMenu(0, MF_BYPOSITION | MF_STRING, 0, cstr);
	cstr = i18n(_T("Menu"), _T("OPTION"));
	menu->ModifyMenu(1, MF_BYPOSITION | MF_STRING, 1, cstr);

	cstr = i18n(_T("Menu"), _T("CUSTOMIZE"));
	menu->ModifyMenu(ID_CUSTOMIZE, MF_STRING, ID_CUSTOMIZE, cstr);
 	cstr = i18n(_T("Menu"), _T("EXIT"));
	menu->ModifyMenu(ID_FILE_EXIT, MF_STRING, ID_FILE_EXIT, cstr);
	cstr = i18n(_T("Menu"), _T("PAINT_WEEKEND"));
	menu->ModifyMenu(ID_PAINT_WEEKEND, MF_STRING, ID_PAINT_WEEKEND, cstr);
	cstr = i18n(_T("Menu"), _T("ALL"));
	menu->ModifyMenu(ID_POINT_ALL, MF_STRING, ID_POINT_ALL, cstr);
	cstr = i18n(_T("Menu"), _T("NORTH_WEST"));
	menu->ModifyMenu(ID_NORTH_WEST, MF_STRING, ID_NORTH_WEST, cstr);
	cstr = i18n(_T("Menu"), _T("NORTH_EAST"));
	menu->ModifyMenu(ID_NORTH_EAST, MF_STRING, ID_NORTH_EAST, cstr);
	cstr = i18n(_T("Menu"), _T("SOUTH_WEST"));
	menu->ModifyMenu(ID_SOUTH_WEST, MF_STRING, ID_SOUTH_WEST, cstr);
	cstr = i18n(_T("Menu"), _T("SOUTH_EAST"));
	menu->ModifyMenu(ID_SOUTH_EAST, MF_STRING, ID_SOUTH_EAST, cstr);

	subMenu.Attach(menu->GetSubMenu(1)->GetSafeHmenu());
	cstr = i18n(_T("Menu"), _T("LEGEND_POSITION"));
	subMenu.ModifyMenu(0, MF_BYPOSITION, 0, cstr);
	cstr = i18n(_T("Menu"), _T("MAX_PLOT_POINT"));
	subMenu.ModifyMenu(1, MF_BYPOSITION, 1, cstr);
	cstr = i18n(_T("Menu"), _T("TIME_FORMAT"));
	subMenu.ModifyMenu(2, MF_BYPOSITION, 2, cstr);

	subMenu.Detach();

	if(m_FlagPaintWeekend)
	{
		menu->CheckMenuItem(ID_PAINT_WEEKEND, MF_CHECKED);
	}

	if(m_LegendPositon.Compare(_T("nw")) == 0)
	{
		menu->CheckMenuRadioItem(ID_NORTH_WEST, ID_SOUTH_EAST, ID_NORTH_WEST, MF_BYCOMMAND);
	}
	else if(m_LegendPositon.Compare(_T("ne")) == 0)
	{
		menu->CheckMenuRadioItem(ID_NORTH_WEST, ID_SOUTH_EAST, ID_NORTH_EAST, MF_BYCOMMAND);
	}
	else if(m_LegendPositon.Compare(_T("sw")) == 0)
	{
		menu->CheckMenuRadioItem(ID_NORTH_WEST, ID_SOUTH_EAST, ID_SOUTH_WEST, MF_BYCOMMAND);
	}
	else
	{
		menu->CheckMenuRadioItem(ID_NORTH_WEST, ID_SOUTH_EAST, ID_SOUTH_EAST, MF_BYCOMMAND);
	}

	if(m_TimeFormat.Compare(_T("%m/%d")) == 0)
	{
		menu->CheckMenuRadioItem(ID_MDHM, ID_DMY2, ID_MD, MF_BYCOMMAND);
	}
	else if(m_TimeFormat.Compare(_T("%y/%m/%d %H:%M")) == 0)
	{
		menu->CheckMenuRadioItem(ID_MDHM, ID_DMY2, ID_YMDHM, MF_BYCOMMAND);
	}
	else if(m_TimeFormat.Compare(_T("%y/%m/%d")) == 0)
	{
		menu->CheckMenuRadioItem(ID_MDHM, ID_DMY2, ID_YMD, MF_BYCOMMAND);
	}
	else if(m_TimeFormat.Compare(_T("%d/%m/%y")) == 0)
	{
		menu->CheckMenuRadioItem(ID_MDHM, ID_DMY2, ID_DMY, MF_BYCOMMAND);
	}
	else if(m_TimeFormat.Compare(_T("%d/%m/%y %H:%M")) == 0)
	{
		menu->CheckMenuRadioItem(ID_MDHM, ID_DMY2, ID_DMYHM, MF_BYCOMMAND);
	}
	else if(m_TimeFormat.Compare(_T("%d/%m/%y")) == 0)
	{
		menu->CheckMenuRadioItem(ID_MDHM, ID_DMY2, ID_DMY, MF_BYCOMMAND);
	}
	else if(m_TimeFormat.Compare(_T("%d.%m.%y %H:%M")) == 0)
	{
		menu->CheckMenuRadioItem(ID_MDHM, ID_DMY2, ID_DMYHM2, MF_BYCOMMAND);
	}
	else if(m_TimeFormat.Compare(_T("%d.%m.%y")) == 0)
	{
		menu->CheckMenuRadioItem(ID_MDHM, ID_DMY2, ID_DMY2, MF_BYCOMMAND);
	}
	else
	{
		menu->CheckMenuRadioItem(ID_MDHM, ID_DMY2, ID_MDHM, MF_BYCOMMAND);
	}

	switch(m_MaxPlotPoint)
	{
	case 100:menu->CheckMenuRadioItem(ID_POINT_100, ID_POINT_ALL, ID_POINT_100, MF_BYCOMMAND);break;
	case 200:menu->CheckMenuRadioItem(ID_POINT_100, ID_POINT_ALL, ID_POINT_200, MF_BYCOMMAND);break;
	case 300:menu->CheckMenuRadioItem(ID_POINT_100, ID_POINT_ALL, ID_POINT_300, MF_BYCOMMAND);break;
	case 400:menu->CheckMenuRadioItem(ID_POINT_100, ID_POINT_ALL, ID_POINT_400, MF_BYCOMMAND);break;
	case 500:menu->CheckMenuRadioItem(ID_POINT_100, ID_POINT_ALL, ID_POINT_500, MF_BYCOMMAND);break;
	case 600:menu->CheckMenuRadioItem(ID_POINT_100, ID_POINT_ALL, ID_POINT_600, MF_BYCOMMAND);break;
	case 700:menu->CheckMenuRadioItem(ID_POINT_100, ID_POINT_ALL, ID_POINT_700, MF_BYCOMMAND);break;
	case 800:menu->CheckMenuRadioItem(ID_POINT_100, ID_POINT_ALL, ID_POINT_800, MF_BYCOMMAND);break;
	case 900:menu->CheckMenuRadioItem(ID_POINT_100, ID_POINT_ALL, ID_POINT_900, MF_BYCOMMAND);break;
	case 1000:menu->CheckMenuRadioItem(ID_POINT_100, ID_POINT_ALL, ID_POINT_1000, MF_BYCOMMAND);break;
	case 2000:menu->CheckMenuRadioItem(ID_POINT_100, ID_POINT_ALL, ID_POINT_2000, MF_BYCOMMAND);break;
	case 3000:menu->CheckMenuRadioItem(ID_POINT_100, ID_POINT_ALL, ID_POINT_3000, MF_BYCOMMAND);break;
	case 4000:menu->CheckMenuRadioItem(ID_POINT_100, ID_POINT_ALL, ID_POINT_4000, MF_BYCOMMAND);break;
	case 5000:menu->CheckMenuRadioItem(ID_POINT_100, ID_POINT_ALL, ID_POINT_5000, MF_BYCOMMAND);break;
	default:
		m_MaxPlotPoint = 0;
		menu->CheckMenuRadioItem(ID_POINT_100, ID_POINT_ALL, ID_POINT_ALL, MF_BYCOMMAND);
		break;
	}

	SetMenu(menu);
	DrawMenuBar();
}

BOOL CGraphDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if(GRAPH_DISK_INDEX <= wParam && wParam <= GRAPH_DISK_INDEX + (CAtaSmart::MAX_DISK + 1) * 100)
	{
		int i = (int)(wParam - GRAPH_DISK_INDEX) / 100;
		int j = (int)(wParam - GRAPH_DISK_INDEX) % 100;

		UpdateGraph();
	}

	return CDHtmlDialogEx::OnCommand(wParam, lParam);
}

time_t CGraphDlg::GetTimeT(CString time)
{
	CString token;
	int currentPosition = 0;
	int index = 0;
	tm t = {0};

	token = time.Tokenize(_T(" :/"), currentPosition);
	while(token != _T(""))
	{
		switch(index)
		{
		case 0:
			t.tm_year = _ttoi(token) - 1900;
			break;
		case 1:
			t.tm_mon = _ttoi(token) - 1;
			break;
		case 2:
			t.tm_mday = _ttoi(token);
			break;
		case 3:
			t.tm_hour = _ttoi(token);
			break;
		case 4:
			t.tm_min = _ttoi(token);
			break;
		case 5:
			t.tm_sec = _ttoi(token);
			break;
		default:
			break;
		}
		token = time.Tokenize(_T(" :/"), currentPosition);
		index++;
	}

	return mktime(&t) - m_TimeZoneInformation.Bias * 60;
}

void CGraphDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	lpMMI->ptMinTrackSize.x = SIZE_X + GetSystemMetrics(SM_CXFRAME) * 2;
	lpMMI->ptMinTrackSize.y = SIZE_Y + GetSystemMetrics(SM_CYMENU) + GetSystemMetrics(SM_CYSIZEFRAME) * 2 + GetSystemMetrics(SM_CYCAPTION);

	CDHtmlDialogEx::OnGetMinMaxInfo(lpMMI);
}

void CGraphDlg::UpdateDialogSize()
{
	if(GetPrivateProfileInt(_T("Setting"), _T("GraphHeight"), 0, m_Ini) > 0)
	{
		m_SizeY = GetPrivateProfileInt(_T("Setting"), _T("GraphHeight"), 0, m_Ini);
	}
	else
	{
		m_SizeY = SIZE_Y;
	}
	if(GetPrivateProfileInt(_T("Setting"), _T("GraphWidth"), 0, m_Ini) > 0)
	{
		m_SizeX = GetPrivateProfileInt(_T("Setting"), _T("GraphWidth"), 0, m_Ini);
	}
	else
	{
		m_SizeX = SIZE_X;
	}

	SetClientRect(m_SizeX, m_SizeY, 1);
}

void CGraphDlg::OnSize(UINT nType, int cx, int cy)
{
	CDHtmlMainDialog::OnSize(nType, cx, cy);

	if(m_FlagInitializing == FALSE)
	{
		RECT rect;
		CString cstr;
		GetClientRect(&rect);
		cstr.Format(_T("%d"), rect.bottom - rect.top);
		WritePrivateProfileString(_T("Setting"), _T("GraphHeight"), cstr, m_Ini);	
		cstr.Format(_T("%d"), rect.right - rect.left);
		WritePrivateProfileString(_T("Setting"), _T("GraphWidth"), cstr, m_Ini);
	}
}

void CGraphDlg::OnExit()
{
	OnCancel();
}

// Legend Position
void CGraphDlg::OnNorthWest(){SetLegendPosition(ID_NORTH_WEST, _T("nw"));}
void CGraphDlg::OnNorthEast(){SetLegendPosition(ID_NORTH_EAST, _T("ne"));}
void CGraphDlg::OnSouthWest(){SetLegendPosition(ID_SOUTH_WEST, _T("sw"));}
void CGraphDlg::OnSouthEast(){SetLegendPosition(ID_SOUTH_EAST, _T("se"));}

void CGraphDlg::SetLegendPosition(DWORD id, CString position)
{
	WritePrivateProfileString(_T("Setting"), _T("LegendPosition"), position, m_Ini);
	m_LegendPositon = position;
	UpdateGraph();

	CMenu *menu = GetMenu();
	menu->CheckMenuRadioItem(ID_NORTH_WEST, ID_SOUTH_EAST, id, MF_BYCOMMAND);
	SetMenu(menu);
	DrawMenuBar();
}

void CGraphDlg::OnMdhm()  {SetTimeFormat(ID_MDHM,   _T("%m/%d %H:%M"));}
void CGraphDlg::OnMd()    {SetTimeFormat(ID_MD,     _T("%m/%d"));}
void CGraphDlg::OnYmdhm() {SetTimeFormat(ID_YMDHM,  _T("%y/%m/%d %H:%M"));}
void CGraphDlg::OnYmd()   {SetTimeFormat(ID_YMD,    _T("%y/%m/%d"));}
void CGraphDlg::OnDmyhm() {SetTimeFormat(ID_DMYHM,  _T("%d/%m/%y %H:%M"));}
void CGraphDlg::OnDmy()   {SetTimeFormat(ID_DMY,    _T("%d/%m/%y"));}
void CGraphDlg::OnDmyhm2(){SetTimeFormat(ID_DMYHM2, _T("%d.%m.%y %H:%M"));}
void CGraphDlg::OnDmy2()  {SetTimeFormat(ID_DMY2,   _T("%d.%m.%y"));}

void CGraphDlg::SetTimeFormat(DWORD id, CString format)
{
	WritePrivateProfileString(_T("Setting"), _T("TimeFormat"), format, m_Ini);
	m_TimeFormat = format;
	UpdateGraph();

	CMenu *menu = GetMenu();
	menu->CheckMenuRadioItem(ID_MDHM, ID_DMY2, id, MF_BYCOMMAND);
	SetMenu(menu);
	DrawMenuBar();
}

void CGraphDlg::OnPaintWeekend()
{
	if(m_FlagPaintWeekend)
	{
		m_FlagPaintWeekend = FALSE;
		WritePrivateProfileString(_T("Setting"), _T("PaintWeekend"), _T("0"), m_Ini);

		CMenu *menu = GetMenu();
		menu->CheckMenuItem(ID_PAINT_WEEKEND, MF_UNCHECKED);
		SetMenu(menu);
		DrawMenuBar();
	}
	else
	{
		m_FlagPaintWeekend = TRUE;
		WritePrivateProfileString(_T("Setting"), _T("PaintWeekend"), _T("1"), m_Ini);

		CMenu *menu = GetMenu();
		menu->CheckMenuItem(ID_PAINT_WEEKEND, MF_CHECKED);
		SetMenu(menu);
		DrawMenuBar();
	}
	UpdateGraph();
}

void CGraphDlg::OnPoint100(){SetPlotPoint(ID_POINT_100, 100);}
void CGraphDlg::OnPoint200(){SetPlotPoint(ID_POINT_200, 200);}
void CGraphDlg::OnPoint300(){SetPlotPoint(ID_POINT_300, 300);}
void CGraphDlg::OnPoint400(){SetPlotPoint(ID_POINT_400, 400);}
void CGraphDlg::OnPoint500(){SetPlotPoint(ID_POINT_500, 500);}
void CGraphDlg::OnPoint600(){SetPlotPoint(ID_POINT_600, 600);}
void CGraphDlg::OnPoint700(){SetPlotPoint(ID_POINT_700, 700);}
void CGraphDlg::OnPoint800(){SetPlotPoint(ID_POINT_800, 800);}
void CGraphDlg::OnPoint900(){SetPlotPoint(ID_POINT_900, 900);}
void CGraphDlg::OnPoint1000(){SetPlotPoint(ID_POINT_1000, 1000);}
void CGraphDlg::OnPoint2000(){SetPlotPoint(ID_POINT_2000, 2000);}
void CGraphDlg::OnPoint3000(){SetPlotPoint(ID_POINT_3000, 3000);}
void CGraphDlg::OnPoint4000(){SetPlotPoint(ID_POINT_4000, 4000);}
void CGraphDlg::OnPoint5000(){SetPlotPoint(ID_POINT_5000, 5000);}
void CGraphDlg::OnPointAll(){SetPlotPoint(ID_POINT_ALL, 0);}

void CGraphDlg::SetPlotPoint(DWORD id, DWORD point)
{
	CString cstr;
	cstr.Format(_T("%d"), point);
	WritePrivateProfileString(_T("Setting"), _T("MaxPlotPoint"), cstr, m_Ini);
	m_MaxPlotPoint = point;
	UpdateGraph();

	CMenu *menu = GetMenu();
	menu->CheckMenuRadioItem(ID_POINT_100, ID_POINT_ALL, id, MF_BYCOMMAND);
	SetMenu(menu);
	DrawMenuBar();
}

void CGraphDlg::OnCustomize()
{
	m_OptionDlg = new COptionDlg(this);
	m_OptionDlg->Create(COptionDlg::IDD, m_OptionDlg, ID_CUSTOMIZE, this);
}

void CGraphDlg::UpdateBgImage()
{
	TCHAR str[256];
	CString cstr;
	GetPrivateProfileString(_T("Customize"), _T("GraphBgImage"), _T(""), str, 256, m_Ini);
	cstr = str;
	if(cstr.IsEmpty())
	{
		cstr = _T("image\\background.png");
	}
	CallScript(_T("changeBackgroundImage"), _T("url(") + cstr + _T(")"));
}

LRESULT CGraphDlg::OnUpdateBgImage(WPARAM wParam, LPARAM lParam)
{
	UpdateBgImage();
	return 0;
}

LRESULT CGraphDlg::OnUpdateLineColor(WPARAM wParam, LPARAM lParam)
{
	UpdateGraph();
	return 0;
}

