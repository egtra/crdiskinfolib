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
#include "DiskInfoDlg.h"

#include "GetFileVersion.h"
#include "GetOsInfo.h"

#ifdef BENCHMARK
#include "Benchmark.h"
#endif

#include <complex>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// Global
	// Task Tray
	UINT gRegMessageId = ::RegisterWindowMessage(_T("CrystalDiskInfo"));
	UINT gRegIconId = ::RegisterWindowMessage(_T("CrystalDiskInfoIcon"));

	UINT gTempIcon0  = ::RegisterWindowMessage(_T("TempIcon0"));
	UINT gTempIcon1  = ::RegisterWindowMessage(_T("TempIcon1"));
	UINT gTempIcon2  = ::RegisterWindowMessage(_T("TempIcon2"));
	UINT gTempIcon3  = ::RegisterWindowMessage(_T("TempIcon3"));
	UINT gTempIcon4  = ::RegisterWindowMessage(_T("TempIcon4"));
	UINT gTempIcon5  = ::RegisterWindowMessage(_T("TempIcon5"));
	UINT gTempIcon6  = ::RegisterWindowMessage(_T("TempIcon6"));
	UINT gTempIcon7  = ::RegisterWindowMessage(_T("TempIcon7"));
	UINT gTempIcon8  = ::RegisterWindowMessage(_T("TempIcon8"));
	UINT gTempIcon9  = ::RegisterWindowMessage(_T("TempIcon9"));
	UINT gTempIcon10 = ::RegisterWindowMessage(_T("TempIcon10"));
	UINT gTempIcon11 = ::RegisterWindowMessage(_T("TempIcon11"));
	UINT gTempIcon12 = ::RegisterWindowMessage(_T("TempIcon12"));
	UINT gTempIcon13 = ::RegisterWindowMessage(_T("TempIcon13"));
	UINT gTempIcon14 = ::RegisterWindowMessage(_T("TempIcon14"));
	UINT gTempIcon15 = ::RegisterWindowMessage(_T("TempIcon15"));
	UINT gTempIcon16 = ::RegisterWindowMessage(_T("TempIcon16"));
	UINT gTempIcon17 = ::RegisterWindowMessage(_T("TempIcon17"));
	UINT gTempIcon18 = ::RegisterWindowMessage(_T("TempIcon18"));
	UINT gTempIcon19 = ::RegisterWindowMessage(_T("TempIcon19"));
	UINT gTempIcon20 = ::RegisterWindowMessage(_T("TempIcon20"));
	UINT gTempIcon21 = ::RegisterWindowMessage(_T("TempIcon21"));
	UINT gTempIcon22 = ::RegisterWindowMessage(_T("TempIcon22"));
	UINT gTempIcon23 = ::RegisterWindowMessage(_T("TempIcon23"));
	UINT gTempIcon24 = ::RegisterWindowMessage(_T("TempIcon24"));
	UINT gTempIcon25 = ::RegisterWindowMessage(_T("TempIcon25"));
	UINT gTempIcon26 = ::RegisterWindowMessage(_T("TempIcon26"));
	UINT gTempIcon27 = ::RegisterWindowMessage(_T("TempIcon27"));
	UINT gTempIcon28 = ::RegisterWindowMessage(_T("TempIcon28"));
	UINT gTempIcon29 = ::RegisterWindowMessage(_T("TempIcon29"));
	UINT gTempIcon30 = ::RegisterWindowMessage(_T("TempIcon30"));
	UINT gTempIcon31 = ::RegisterWindowMessage(_T("TempIcon31"));

	extern const GUID StrageGUID = { 0x53F56307, 0xB6BF, 0x11D0, 
                      0x94,0xF2,0x00,0xA0,0xC9,0x1E,0xFB,0x8B };

// CDiskInfoDlg dialog

BEGIN_DHTML_EVENT_MAP(CDiskInfoDlg)
	DHTML_EVENT_ONCLICK(_T("Disk0"), OnDisk0)
	DHTML_EVENT_ONCLICK(_T("Disk1"), OnDisk1)
	DHTML_EVENT_ONCLICK(_T("Disk2"), OnDisk2)
	DHTML_EVENT_ONCLICK(_T("Disk3"), OnDisk3)
	DHTML_EVENT_ONCLICK(_T("Disk4"), OnDisk4)
	DHTML_EVENT_ONCLICK(_T("Disk5"), OnDisk5)
	DHTML_EVENT_ONCLICK(_T("Disk6"), OnDisk6)
	DHTML_EVENT_ONCLICK(_T("Disk7"), OnDisk7)
	DHTML_EVENT_ONCLICK(_T("PreDisk"), OnPreDisk)
	DHTML_EVENT_ONCLICK(_T("NextDisk"), OnNextDisk)
	DHTML_EVENT_ONCLICK(_T("DiskStatus"), OnDiskStatus)
#ifdef BENCHMARK
	DHTML_EVENT_ONCLICK(_T("Benchmark"), OnBenchmark)
#endif
END_DHTML_EVENT_MAP()

CDiskInfoDlg::CDiskInfoDlg(CWnd* pParent /*=NULL*/, BOOL flagStartupExit)
	: CDHtmlMainDialog(CDiskInfoDlg::IDD, CDiskInfoDlg::IDH,
	((CDiskInfoApp*)AfxGetApp())->m_ThemeDir,
	((CDiskInfoApp*)AfxGetApp())->m_ThemeIndex,
	((CDiskInfoApp*)AfxGetApp())->m_LangDir,
	((CDiskInfoApp*)AfxGetApp())->m_LangIndex,
	pParent)
{
	m_hMenu = NULL;
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_hIconMini = AfxGetApp()->LoadIcon(IDI_TRAY_ICON);
	m_MainIconId = gRegIconId;

	m_SmartDir = ((CDiskInfoApp*)AfxGetApp())->m_SmartDir;
	m_ExeDir = ((CDiskInfoApp*)AfxGetApp())->m_ExeDir;
	_tcscpy_s(m_Ini, MAX_PATH, ((CDiskInfoApp*)AfxGetApp())->m_Ini);

	m_FlagStartupExit = flagStartupExit;

	m_AboutDlg = NULL;
	m_SettingDlg = NULL;
	m_HealthDlg = NULL;
	m_OptionDlg = NULL;

	for(int i = 0; i < 100; i++)
	{
		m_hTempIcon[0][i] = NULL;
		m_hTempIcon[1][i] = NULL;
	}

	m_TempIconIndex[0]  = gTempIcon0;
	m_TempIconIndex[1]  = gTempIcon1;
	m_TempIconIndex[2]  = gTempIcon2;
	m_TempIconIndex[3]  = gTempIcon3;
	m_TempIconIndex[4]  = gTempIcon4;
	m_TempIconIndex[5]  = gTempIcon5;
	m_TempIconIndex[6]  = gTempIcon6;
	m_TempIconIndex[7]  = gTempIcon7;
	m_TempIconIndex[8]  = gTempIcon8;
	m_TempIconIndex[9]  = gTempIcon9;
	m_TempIconIndex[10] = gTempIcon10;
	m_TempIconIndex[11] = gTempIcon11;
	m_TempIconIndex[12] = gTempIcon12;
	m_TempIconIndex[13] = gTempIcon13;
	m_TempIconIndex[14] = gTempIcon14;
	m_TempIconIndex[15] = gTempIcon15;
	m_TempIconIndex[16] = gTempIcon16;
	m_TempIconIndex[17] = gTempIcon17;
	m_TempIconIndex[18] = gTempIcon18;
	m_TempIconIndex[19] = gTempIcon19;
	m_TempIconIndex[20] = gTempIcon20;
	m_TempIconIndex[21] = gTempIcon21;
	m_TempIconIndex[22] = gTempIcon22;
	m_TempIconIndex[23] = gTempIcon23;
	m_TempIconIndex[24] = gTempIcon24;
	m_TempIconIndex[25] = gTempIcon25;
	m_TempIconIndex[26] = gTempIcon26;
	m_TempIconIndex[27] = gTempIcon27;
	m_TempIconIndex[28] = gTempIcon28;
	m_TempIconIndex[29] = gTempIcon29;
	m_TempIconIndex[30] = gTempIcon30;
	m_TempIconIndex[31] = gTempIcon31;

	m_FlagTrayMainIcon = FALSE;
	for(int i = 0; i < CAtaSmart::MAX_DISK; i++)
	{
		m_PreTemp[i] = 0;
		m_FlagTrayTemperatureIcon[i] = FALSE;
	}

	m_SelectDisk = 0;
	m_DriveMenuPage = 0;
	m_AutoRefreshStatus = 0;
	m_WaitTimeStatus = 0;
	m_AutoDetectionStatus = 0;
	m_RawValues = 0;
	
	m_NowDetectingUnitPowerOnHours = FALSE;
	m_FlagInitializing = TRUE;

	m_ImageList.Create(16, 16, ILC_COLOR32|ILC_MASK, 3, 1);
	m_ImageList.Add(AfxGetApp()->LoadIcon(IDI_GOOD));
	m_ImageList.Add(AfxGetApp()->LoadIcon(IDI_CAUTION));
	m_ImageList.Add(AfxGetApp()->LoadIcon(IDI_BAD));
	m_ImageList.Add(AfxGetApp()->LoadIcon(IDI_UNKNOWN));

	m_FlagAdvancedDiskSearch = (BOOL)GetPrivateProfileInt(_T("Setting"), _T("AdvancedDiskSearch"), 0, m_Ini);
	m_FlagEventLog = (BOOL)GetPrivateProfileInt(_T("Setting"), _T("EventLog"), 0, m_Ini);
	m_FlagAtaPassThroughSmart = (BOOL)GetPrivateProfileInt(_T("Setting"), _T("AtaPassThroughSmart"), 0, m_Ini);
	m_FlagAutoAamApm = (BOOL)GetPrivateProfileInt(_T("Setting"), _T("AutoAamApm"), 0, m_Ini);
	m_FlagDumpIdentifyDevice = (BOOL)GetPrivateProfileInt(_T("Setting"), _T("DumpIdentifyDevice"), 1, m_Ini);			// Default = Enabled
	m_FlagDumpSmartReadData = (BOOL)GetPrivateProfileInt(_T("Setting"), _T("DumpSmartReadData"), 0, m_Ini);
	m_FlagDumpSmartReadThreshold = (BOOL)GetPrivateProfileInt(_T("Setting"), _T("DumpSmartReadThreshold"), 0, m_Ini);
	m_FlagResidentMinimize = (BOOL)GetPrivateProfileInt(_T("Setting"), _T("ResidentMinimize"), 0, m_Ini);
	m_FlagShowTemperatureIconOnly = (BOOL)GetPrivateProfileInt(_T("Setting"), _T("ShowTemperatureIconOnly"), 0, m_Ini);
	m_FlagAsciiView = (BOOL)GetPrivateProfileInt(_T("Setting"), _T("AsciiView"), 0, m_Ini);
	m_FlagSidebar = IsSidebar();

	m_AutoDetectionStatus = GetPrivateProfileInt(_T("Setting"), _T("AutoDetection"), 0, m_Ini);
	if(m_AutoDetectionStatus < 0)
	{
		m_AutoDetectionStatus = 0;
	}

	m_RawValues = GetPrivateProfileInt(_T("Setting"), _T("RawValues"), 0, m_Ini);
	if(m_RawValues < 0 || m_RawValues > 3)
	{
		m_RawValues = 0;
	}

	m_ZoomType = (BOOL)GetPrivateProfileInt(_T("Setting"), _T("ZoomType"), ZOOM_TYPE_AUTO, m_Ini);

	// Setting
	SAVE_SMART_PERIOD = GetPrivateProfileInt(_T("Setting"), _T("SAVE_SMART_PERIOD"), 150, m_Ini);
	ALARM_TEMPERATURE_PERIOD = GetPrivateProfileInt(_T("Setting"), _T("ALARM_TEMPERATURE_PERIOD"), 60 * 60, m_Ini);


	if(m_FlagEventLog)
	{
		InstallEventSource();
	}
	else
	{
		UninstallEventSource();
	}

	if(m_FlagAtaPassThroughSmart)
	{
		m_Ata.SetAtaPassThroughSmart(TRUE);
	}
	else
	{
		m_Ata.SetAtaPassThroughSmart(FALSE);
	}

	/*
	CString cstr;
	TCHAR str[MAX_PATH];
	GetSystemDirectory(str, MAX_PATH);
	cstr.Format(_T("%s\\eventcreate.exe"), str);

	if(IsFileExist(cstr))
	{
		m_FlagUseEventCreate = TRUE;
	}
	else
	{
		m_FlagUseEventCreate = FALSE;
	}
	*/
}

CDiskInfoDlg::~CDiskInfoDlg()
{
	if(m_hMenu != NULL)
	{
		DestroyMenu(m_hMenu);
	}
	
	#ifdef GADGET_SUPPORT
	DeleteShareInfo();
	#endif
}

void CDiskInfoDlg::OnCancel()
{
	if(m_FlagResidentMinimize)
	{
		ShowWindow(SW_MINIMIZE);
	}
	else
	{
		ShowWindow(SW_HIDE);
	}
	if(! m_FlagResident)
	{
		KillGraphDlg();
		if(m_hDevNotify)
		{
			UnregisterDeviceNotification(m_hDevNotify);
		}
		CDHtmlMainDialog::OnCancel();
	}
}

void CDiskInfoDlg::OnExit()
{
	ShowWindow(SW_HIDE);
	RemoveTrayMainIcon();
	for(int i = 0; i < m_Ata.vars.GetCount(); i++)
	{
		RemoveTemperatureIcon(i);
	}
	KillGraphDlg();
	CDHtmlMainDialog::OnCancel();
}

void CDiskInfoDlg::OnOK()
{

}

BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam);

void CDiskInfoDlg::KillGraphDlg()
{
	static HWND hWnd;
	EnumWindows(EnumWindowsProc, (LPARAM)&m_GraphProcessId);
}

BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam)
{
	CArray<DWORD, DWORD> *id = (CArray<DWORD, DWORD>*)lParam;

	TCHAR str[1024];
	GetWindowText(hWnd, str, 1024);
	if(str[0] == 0)
	{
		return TRUE;
	}

	CString cstr;
	cstr = str;

	if(cstr.Find(_T("CrystalDiskInfo - ")) == 0 && cstr.Find(_T(" - Powered by Flot")) > 0)
	{
		for(int i = 0; i < id->GetCount(); i++)
		{
			DWORD processId = 0;
			::GetWindowThreadProcessId(hWnd, &processId);
			if(processId == id->GetAt(i))
			{
				PostMessage(hWnd, WM_QUIT, NULL, NULL);
			}
		}
	}

	return TRUE;
}

void CDiskInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDHtmlDialog::DoDataExchange(pDX);
	DDX_DHtml_ElementInnerHtml(pDX, _T("Disk0"), m_LiDisk[0]);
	DDX_DHtml_ElementInnerHtml(pDX, _T("Disk1"), m_LiDisk[1]);
	DDX_DHtml_ElementInnerHtml(pDX, _T("Disk2"), m_LiDisk[2]);
	DDX_DHtml_ElementInnerHtml(pDX, _T("Disk3"), m_LiDisk[3]);
	DDX_DHtml_ElementInnerHtml(pDX, _T("Disk4"), m_LiDisk[4]);
	DDX_DHtml_ElementInnerHtml(pDX, _T("Disk5"), m_LiDisk[5]);
	DDX_DHtml_ElementInnerHtml(pDX, _T("Disk6"), m_LiDisk[6]);
	DDX_DHtml_ElementInnerHtml(pDX, _T("Disk7"), m_LiDisk[7]);

	DDX_DHtml_ElementInnerText(pDX, _T("DiskStatus"), m_DiskStatus);
	DDX_DHtml_ElementInnerText(pDX, _T("Temperature"), m_Temperature);

	DDX_DHtml_ElementInnerText(pDX, _T("Model"), m_Model);
	DDX_DHtml_ElementInnerText(pDX, _T("Firmware"), m_Firmware);
	DDX_DHtml_ElementInnerText(pDX, _T("SerialNumber"), m_SerialNumber);
	DDX_DHtml_ElementInnerText(pDX, _T("Capacity"), m_Capacity);
	DDX_DHtml_ElementInnerText(pDX, _T("BufferSize"), m_BufferSize);
	DDX_DHtml_ElementInnerText(pDX, _T("NvCacheSzie"), m_NvCacheSize);
	DDX_DHtml_ElementInnerText(pDX, _T("RotationRate"), m_RotationRate);
	DDX_DHtml_ElementInnerText(pDX, _T("LbaSize"), m_LbaSize);

	DDX_DHtml_ElementInnerText(pDX, _T("DriveMap"), m_DriveMap);
	DDX_DHtml_ElementInnerText(pDX, _T("Interface"), m_Interface);
	DDX_DHtml_ElementInnerText(pDX, _T("TransferMode"), m_TransferMode);
	DDX_DHtml_ElementInnerText(pDX, _T("AtaAtapi"), m_AtaAtapi);
	DDX_DHtml_ElementInnerText(pDX, _T("PowerOnCount"), m_PowerOnCount);
	DDX_DHtml_ElementInnerText(pDX, _T("Feature"), m_Feature);

	DDX_DHtml_ElementInnerText(pDX, _T("PowerOnHours"), m_PowerOnHours);

	DDX_DHtml_ElementInnerText(pDX, _T("LabelFirmware"), m_LabelFirmware);
	DDX_DHtml_ElementInnerText(pDX, _T("LabelSerialNumber"), m_LabelSerialNumber);
	DDX_DHtml_ElementInnerText(pDX, _T("LabelCapacity"), m_LabelCapacity);
	DDX_DHtml_ElementInnerText(pDX, _T("LabelBufferSize"), m_LabelBufferSize);
	DDX_DHtml_ElementInnerText(pDX, _T("LabelNvCacheSize"), m_LabelNvCacheSize);
	DDX_DHtml_ElementInnerText(pDX, _T("LabelRotationRate"), m_LabelRotationRate);
	DDX_DHtml_ElementInnerText(pDX, _T("LabelLbaSize"), m_LabelLbaSize);
	DDX_DHtml_ElementInnerText(pDX, _T("LabelDriveMap"), m_LabelDriveMap);
	DDX_DHtml_ElementInnerText(pDX, _T("LabelInterface"), m_LabelInterface);
	DDX_DHtml_ElementInnerText(pDX, _T("LabelTransferMode"), m_LabelTransferMode);
	DDX_DHtml_ElementInnerText(pDX, _T("LabelAtaAtapi"), m_LabelAtaAtapi);
	DDX_DHtml_ElementInnerText(pDX, _T("LabelSmartStatus"), m_LabelSmartStatus);
	DDX_DHtml_ElementInnerText(pDX, _T("LabelTemperature"), m_LabelTemperature);
	DDX_DHtml_ElementInnerText(pDX, _T("LabelPowerOnHours"), m_LabelPowerOnHours);
	DDX_DHtml_ElementInnerText(pDX, _T("LabelPowerOnCount"), m_LabelPowerOnCount);
	DDX_DHtml_ElementInnerText(pDX, _T("LabelFeature"), m_LabelFeature);
	DDX_DHtml_ElementInnerText(pDX, _T("LabelHealthStatus"), m_LabelDiskStatus);
#ifdef BENCHMARK
	DDX_DHtml_ElementInnerText(pDX, _T("BenchmarkMeter"), m_BenchmarkMeter);
#endif
	DDX_Control(pDX, IDC_LIST, m_List);
}

BEGIN_MESSAGE_MAP(CDiskInfoDlg, CDHtmlMainDialog)
	//}}AFX_MSG_MAP
	ON_WM_GETMINMAXINFO()
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_COMMAND(ID_FILE_EXIT, &CDiskInfoDlg::OnExit)
	ON_COMMAND(ID_HELP_ABOUT, &CDiskInfoDlg::OnAbout)
	ON_COMMAND(ID_HIDE_SMART_INFO, &CDiskInfoDlg::OnHideSmartInfo)
	ON_COMMAND(ID_HIDE_SERIAL_NUMBER, &CDiskInfoDlg::OnHideSerialNumber)
	ON_COMMAND(ID_EDIT_COPY, &CDiskInfoDlg::OnEditCopy)
	ON_COMMAND(ID_HELP_CRYSTALDEWWORLD, &CDiskInfoDlg::OnCrystalDewWorld)
	ON_COMMAND(ID_REFRESH, &CDiskInfoDlg::OnRefresh)
	ON_COMMAND(ID_HELP_ABOUT_SMART, &CDiskInfoDlg::OnHelpAboutSmart)
	ON_COMMAND(ID_AUTO_REFRESH_DISABLE, &CDiskInfoDlg::OnAutoRefreshDisable)
	ON_COMMAND(ID_AUTO_REFRESH_01_MIN, &CDiskInfoDlg::OnAutoRefresh01Min)
	ON_COMMAND(ID_AUTO_REFRESH_03_MIN, &CDiskInfoDlg::OnAutoRefresh03Min)
	ON_COMMAND(ID_AUTO_REFRESH_05_MIN, &CDiskInfoDlg::OnAutoRefresh05Min)
	ON_COMMAND(ID_AUTO_REFRESH_10_MIN, &CDiskInfoDlg::OnAutoRefresh10Min)
	ON_COMMAND(ID_AUTO_REFRESH_30_MIN, &CDiskInfoDlg::OnAutoRefresh30Min)
	ON_COMMAND(ID_AUTO_REFRESH_60_MIN, &CDiskInfoDlg::OnAutoRefresh60Min)
	ON_COMMAND(ID_OPEN_DISK_MANAGEMENT, &CDiskInfoDlg::OnOpenDiskManagement)
	ON_COMMAND(ID_OPEN_DEVICE_MANAGER, &CDiskInfoDlg::OnOpenDeviceManager)
	ON_COMMAND(ID_ADVANCED_DISK_SEARCH, &CDiskInfoDlg::OnAdvancedDiskSearch)
	ON_COMMAND(ID_RESIDENT, &CDiskInfoDlg::OnResident)

	ON_MESSAGE(WM_POWERBROADCAST, &CDiskInfoDlg::OnPowerBroadcast)
	ON_MESSAGE(WM_DEVICECHANGE, &CDiskInfoDlg::OnDeviceChange)

	// Task Tray
	ON_REGISTERED_MESSAGE(gRegMessageId, OnRegMessage)
	ON_REGISTERED_MESSAGE(wmTaskbarCreated, OnTaskbarCreated)

	ON_REGISTERED_MESSAGE(gTempIcon0,  OnTempIcon0)
	ON_REGISTERED_MESSAGE(gTempIcon1,  OnTempIcon1)
	ON_REGISTERED_MESSAGE(gTempIcon2,  OnTempIcon2)
	ON_REGISTERED_MESSAGE(gTempIcon3,  OnTempIcon3)
	ON_REGISTERED_MESSAGE(gTempIcon4,  OnTempIcon4)
	ON_REGISTERED_MESSAGE(gTempIcon5,  OnTempIcon5)
	ON_REGISTERED_MESSAGE(gTempIcon6,  OnTempIcon6)
	ON_REGISTERED_MESSAGE(gTempIcon7,  OnTempIcon7)
	ON_REGISTERED_MESSAGE(gTempIcon8,  OnTempIcon8)
	ON_REGISTERED_MESSAGE(gTempIcon9,  OnTempIcon9)
	ON_REGISTERED_MESSAGE(gTempIcon10, OnTempIcon10)
	ON_REGISTERED_MESSAGE(gTempIcon11, OnTempIcon11)
	ON_REGISTERED_MESSAGE(gTempIcon12, OnTempIcon12)
	ON_REGISTERED_MESSAGE(gTempIcon13, OnTempIcon13)
	ON_REGISTERED_MESSAGE(gTempIcon14, OnTempIcon14)
	ON_REGISTERED_MESSAGE(gTempIcon15, OnTempIcon15)
	ON_REGISTERED_MESSAGE(gTempIcon16, OnTempIcon16)
	ON_REGISTERED_MESSAGE(gTempIcon17, OnTempIcon17)
	ON_REGISTERED_MESSAGE(gTempIcon18, OnTempIcon18)
	ON_REGISTERED_MESSAGE(gTempIcon19, OnTempIcon19)
	ON_REGISTERED_MESSAGE(gTempIcon20, OnTempIcon20)
	ON_REGISTERED_MESSAGE(gTempIcon21, OnTempIcon21)
	ON_REGISTERED_MESSAGE(gTempIcon22, OnTempIcon22)
	ON_REGISTERED_MESSAGE(gTempIcon23, OnTempIcon23)
	ON_REGISTERED_MESSAGE(gTempIcon24, OnTempIcon24)
	ON_REGISTERED_MESSAGE(gTempIcon25, OnTempIcon25)
	ON_REGISTERED_MESSAGE(gTempIcon26, OnTempIcon26)
	ON_REGISTERED_MESSAGE(gTempIcon27, OnTempIcon27)
	ON_REGISTERED_MESSAGE(gTempIcon28, OnTempIcon28)
	ON_REGISTERED_MESSAGE(gTempIcon29, OnTempIcon29)
	ON_REGISTERED_MESSAGE(gTempIcon30, OnTempIcon30)
	ON_REGISTERED_MESSAGE(gTempIcon31, OnTempIcon31)

	ON_COMMAND(ID_GRAPH, &CDiskInfoDlg::OnGraph)
	ON_COMMAND(ID_HELP, &CDiskInfoDlg::OnHelp)
	ON_COMMAND(ID_CUSTOMIZE, &CDiskInfoDlg::OnCustomize)
	ON_COMMAND(ID_STARTUP, &CDiskInfoDlg::OnStartup)
	ON_COMMAND(ID_WAIT_0_SEC, &CDiskInfoDlg::OnWait0Sec)
	ON_COMMAND(ID_WAIT_5_SEC, &CDiskInfoDlg::OnWait5Sec)
	ON_COMMAND(ID_WAIT_10_SEC, &CDiskInfoDlg::OnWait10Sec)
	ON_COMMAND(ID_WAIT_15_SEC, &CDiskInfoDlg::OnWait15Sec)
	ON_COMMAND(ID_WAIT_20_SEC, &CDiskInfoDlg::OnWait20Sec)
	ON_COMMAND(ID_WAIT_30_SEC, &CDiskInfoDlg::OnWait30Sec)
	ON_COMMAND(ID_WAIT_40_SEC, &CDiskInfoDlg::OnWait40Sec)
	ON_COMMAND(ID_WAIT_50_SEC, &CDiskInfoDlg::OnWait50Sec)
	ON_COMMAND(ID_WAIT_60_SEC, &CDiskInfoDlg::OnWait60Sec)
	ON_COMMAND(ID_WAIT_90_SEC, &CDiskInfoDlg::OnWait90Sec)
	ON_COMMAND(ID_WAIT_120_SEC, &CDiskInfoDlg::OnWait120Sec)
	ON_COMMAND(ID_WAIT_150_SEC, &CDiskInfoDlg::OnWait150Sec)
	ON_COMMAND(ID_WAIT_180_SEC, &CDiskInfoDlg::OnWait180Sec)
	ON_COMMAND(ID_WAIT_210_SEC, &CDiskInfoDlg::OnWait210Sec)
	ON_COMMAND(ID_WAIT_240_SEC, &CDiskInfoDlg::OnWait240Sec)

	ON_COMMAND(ID_AUTO_DETECTION_05_SEC, &CDiskInfoDlg::OnAutoDetection05Sec)
	ON_COMMAND(ID_AUTO_DETECTION_10_SEC, &CDiskInfoDlg::OnAutoDetection10Sec)
	ON_COMMAND(ID_AUTO_DETECTION_20_SEC, &CDiskInfoDlg::OnAutoDetection20Sec)
	ON_COMMAND(ID_AUTO_DETECTION_30_SEC, &CDiskInfoDlg::OnAutoDetection30Sec)
	ON_COMMAND(ID_AUTO_DETECTION_DISABLE, &CDiskInfoDlg::OnAutoDetectionDisable)

	ON_COMMAND(ID_EVENT_LOG, &CDiskInfoDlg::OnEventLog)
	ON_COMMAND(ID_ATA_PASS_THROUGH_SMART, &CDiskInfoDlg::OnAtaPassThroughSmart)
	ON_COMMAND(ID_CELSIUS, &CDiskInfoDlg::OnCelsius)
	ON_COMMAND(ID_FAHRENHEIT, &CDiskInfoDlg::OnFahrenheit)
	ON_COMMAND(ID_AAM_APM, &CDiskInfoDlg::OnAamApm)
	ON_COMMAND(ID_AUTO_AAM_APM, &CDiskInfoDlg::OnAutoAamApm)
	ON_COMMAND(ID_RESCAN, &CDiskInfoDlg::OnRescan)
	ON_COMMAND(ID_USB_SAT, &CDiskInfoDlg::OnUsbSat)
	ON_COMMAND(ID_USB_IODATA, &CDiskInfoDlg::OnUsbIodata)
	ON_COMMAND(ID_USB_SUNPLUS, &CDiskInfoDlg::OnUsbSunplus)
	ON_COMMAND(ID_USB_LOGITEC, &CDiskInfoDlg::OnUsbLogitec)
	ON_COMMAND(ID_USB_JMICRON, &CDiskInfoDlg::OnUsbJmicron)
	ON_COMMAND(ID_USB_CYPRESS, &CDiskInfoDlg::OnUsbCypress)
	ON_COMMAND(ID_USB_ENABLE_ALL, &CDiskInfoDlg::OnUsbEnableAll)
	ON_COMMAND(ID_USB_DISABLE_ALL, &CDiskInfoDlg::OnUsbDisableAll)
	ON_COMMAND(ID_HEALTH_STATUS, &CDiskInfoDlg::OnHealthStatus)
	ON_COMMAND(ID_DUMP_IDENTIFY_DEVICE, &CDiskInfoDlg::OnDumpIdentifyDevice)
	ON_COMMAND(ID_DUMP_SMART_READ_DATA, &CDiskInfoDlg::OnDumpSmartReadData)
	ON_COMMAND(ID_DUMP_SMART_READ_THRESHOLD, &CDiskInfoDlg::OnDumpSmartReadThreshold)
	ON_COMMAND(ID_RESIDENT_HIDE, &CDiskInfoDlg::OnResidentHide)
	ON_COMMAND(ID_RESIDENT_MINIMIZE, &CDiskInfoDlg::OnResidentMinimize)
	ON_COMMAND(ID_ZOOM_100, &CDiskInfoDlg::OnZoom100)
	ON_COMMAND(ID_ZOOM_125, &CDiskInfoDlg::OnZoom125)
	ON_COMMAND(ID_ZOOM_150, &CDiskInfoDlg::OnZoom150)
	ON_COMMAND(ID_ZOOM_200, &CDiskInfoDlg::OnZoom200)
	ON_COMMAND(ID_ZOOM_AUTO, &CDiskInfoDlg::OnZoomAuto)
	ON_COMMAND(ID_RAW_VALUES_16, &CDiskInfoDlg::OnRawValues16)
	ON_COMMAND(ID_RAW_VALUES_10_ALL, &CDiskInfoDlg::OnRawValues10All)
	ON_COMMAND(ID_RAW_VALUES_2BYTE, &CDiskInfoDlg::OnRawValues2byte)
	ON_COMMAND(ID_RAW_VALUES_1BYTE, &CDiskInfoDlg::OnRawValues1byte)
	ON_COMMAND(ID_ASCII_VIEW, &CDiskInfoDlg::OnAsciiView)
	END_MESSAGE_MAP()

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.
void CDiskInfoDlg::OnPaint()
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
HCURSOR CDiskInfoDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


BOOL CDiskInfoDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	if(WM_LANGUAGE_ID <= wParam && wParam < WM_LANGUAGE_ID + (UINT)m_MenuArrayLang.GetSize())
	{
#ifdef _UNICODE
		CMenu menu;
		CMenu subMenu;
		CMenu subMenuAN;
		CMenu subMenuOZ;
		menu.Attach(GetMenu()->GetSafeHmenu());
		subMenu.Attach(menu.GetSubMenu(m_LangIndex)->GetSafeHmenu());
		subMenuAN.Attach(subMenu.GetSubMenu(0)->GetSafeHmenu());
		subMenuOZ.Attach(subMenu.GetSubMenu(1)->GetSafeHmenu());

		m_CurrentLang = m_MenuArrayLang.GetAt(wParam - WM_LANGUAGE_ID);
		ChangeLang(m_MenuArrayLang.GetAt(wParam - WM_LANGUAGE_ID));
		subMenuAN.CheckMenuRadioItem(WM_LANGUAGE_ID, WM_LANGUAGE_ID + (UINT)m_MenuArrayLang.GetSize(),
									(UINT)wParam, MF_BYCOMMAND);
		subMenuOZ.CheckMenuRadioItem(WM_LANGUAGE_ID, WM_LANGUAGE_ID + (UINT)m_MenuArrayLang.GetSize(),
									(UINT)wParam, MF_BYCOMMAND);

		subMenuOZ.Detach();
		subMenuAN.Detach();
		subMenu.Detach();
		menu.Detach();
#else
		CMenu menu;
		CMenu subMenu;
		menu.Attach(GetMenu()->GetSafeHmenu());
		subMenu.Attach(menu.GetSubMenu(m_LangIndex)->GetSafeHmenu());

		m_CurrentLang = m_MenuArrayLang.GetAt(wParam - WM_LANGUAGE_ID);
		ChangeLang(m_MenuArrayLang.GetAt(wParam - WM_LANGUAGE_ID));
		subMenu.CheckMenuRadioItem(WM_LANGUAGE_ID, WM_LANGUAGE_ID + (UINT)m_MenuArrayLang.GetSize(),
									(UINT)wParam, MF_BYCOMMAND);
		subMenu.Detach();
		menu.Detach();
#endif
	}
	// Task Tray Menu
	else if(wParam == MY_EXIT)
	{
		RemoveTrayMainIcon();
		for(int i = 0; i < m_Ata.vars.GetCount(); i++)
		{
			RemoveTemperatureIcon(i);
		}
		KillGraphDlg();
		CDHtmlMainDialog::OnCancel();
	}
	else if(wParam == MY_SHOW_MAIN_DIALOG && m_FlagResidentMinimize)
	{
		if(! IsIconic())
		{
			ShowWindowEx(SW_MINIMIZE);
		}
		else
		{
			ShowWindowEx(SW_RESTORE);
		}
	}
	else if(wParam == MY_SHOW_MAIN_DIALOG)
	{
		if(IsWindowVisible())
		{
			ShowWindowEx(SW_HIDE);
		}
		else
		{
			ShowWindowEx(SW_RESTORE);
		}
	}
	else if(wParam == MY_SHOW_TEMPERATURE_ICON_ONLY)
	{
		ShowTemperatureIconOnly();
	}
	else if(wParam == SHOW_GRAPH_BASE + CAtaSmart::MAX_DISK)
	{
		ShowGraphDlg(-1); // Using "GraphHideDisk" option
	}
	else if(SHOW_GRAPH_BASE <= wParam && wParam < SHOW_GRAPH_BASE + CAtaSmart::MAX_DISK)
	{
		ShowGraphDlg((int)wParam - SHOW_GRAPH_BASE);
	}
	else if(ALARM_SETTING_HEALTH_STATUS_BASE <= wParam && wParam <= ALARM_SETTING_HEALTH_STATUS_BASE + CAtaSmart::MAX_DISK + 1)
	{
		int i = (int)(wParam - ALARM_SETTING_HEALTH_STATUS_BASE);

		if(i == CAtaSmart::MAX_DISK + 1) // Disable All
		{
			for(int j = 0; j < m_Ata.vars.GetCount(); j++)
			{
				m_Ata.vars[j].AlarmHealthStatus = FALSE;
				WritePrivateProfileString(_T("AlarmHealthStatus"), m_Ata.vars[j].ModelSerial, _T("0"), m_Ini);
			}
		}
		else if(i == CAtaSmart::MAX_DISK) // Enable All
		{
			for(int j = 0; j < m_Ata.vars.GetCount(); j++)
			{
				m_Ata.vars[j].AlarmHealthStatus = TRUE;
				WritePrivateProfileString(_T("AlarmHealthStatus"), m_Ata.vars[j].ModelSerial, _T("1"), m_Ini);
			}
		}
		else
		{
			CString alarm;
			if(m_Ata.vars[i].AlarmHealthStatus)
			{
				m_Ata.vars[i].AlarmHealthStatus = FALSE;
				alarm.Format(_T("%d"), FALSE);
			}
			else
			{
				m_Ata.vars[i].AlarmHealthStatus = TRUE;
				alarm.Format(_T("%d"), TRUE);
			}
			WritePrivateProfileString(_T("AlarmHealthStatus"), m_Ata.vars[i].ModelSerial, alarm, m_Ini);
		}
	}
	else if(ALARM_SETTING_TEMPERATURE_BASE <= wParam && wParam <= ALARM_SETTING_TEMPERATURE_BASE + (CAtaSmart::MAX_DISK + 1) * 100)
	{
		int i = (int)(wParam - ALARM_SETTING_TEMPERATURE_BASE) / 100;
		int j = (int)(wParam - ALARM_SETTING_TEMPERATURE_BASE) % 100;

		if(i == CAtaSmart::MAX_DISK)
		{
			for(int k = 0; k < m_Ata.vars.GetCount(); k++)
			{
				m_Ata.vars[k].AlarmTemperature = j;
				CString temperature;
				temperature.Format(_T("%d"), j);
				WritePrivateProfileString(_T("AlarmTemperature"), m_Ata.vars[k].Model + m_Ata.vars[k].SerialNumber, temperature, m_Ini);
			}
		}
		else
		{
			m_Ata.vars[i].AlarmTemperature = j;
			CString temperature;
			temperature.Format(_T("%d"), j);
			WritePrivateProfileString(_T("AlarmTemperature"), m_Ata.vars[i].ModelSerial, temperature, m_Ini);
		}
	}
	else if(TRAY_TEMPERATURE_ICON_BASE <= wParam && wParam <= TRAY_TEMPERATURE_ICON_BASE + CAtaSmart::MAX_DISK + 1)
	{
		int i = (int)(wParam - TRAY_TEMPERATURE_ICON_BASE);

		if(i == CAtaSmart::MAX_DISK + 1) // Hide All
		{
			for(int j = 0; j < m_Ata.vars.GetCount(); j++)
			{
				if(m_FlagTrayTemperatureIcon[j])
				{
					if(RemoveTemperatureIcon(j))
					{
						CString cstr;
						cstr.Format(_T("%d"), 0);
						WritePrivateProfileString(_T("TemperatureIcon"), m_Ata.vars[j].ModelSerial, cstr, m_Ini);
					}
				}
			}
			if(m_FlagShowTemperatureIconOnly && ! IsTemperatureIconExist())
			{
				AddTrayMainIcon();
			}
		}
		else if(i == CAtaSmart::MAX_DISK) // Show All
		{
			int max = gRegIconId;
			for(int j = (int)m_Ata.vars.GetCount() -1; j >= 0; j--)
			{
				if(! m_FlagTrayTemperatureIcon[j])
				{
					if(AddTemperatureIcon(j))
					{
						CString cstr;
						cstr.Format(_T("%d"), 1);
						WritePrivateProfileString(_T("TemperatureIcon"), m_Ata.vars[j].ModelSerial, cstr, m_Ini);
						max = TRAY_TEMPERATURE_ICON_BASE + j;
					}
				}
			}
			if(m_FlagShowTemperatureIconOnly && IsTemperatureIconExist())
			{
				if(RemoveTrayMainIcon())
				{
					m_MainIconId = max;
				}
			}
			else
			{
				AddTrayMainIcon();
			}
		}
		else
		{
			if(m_FlagTrayTemperatureIcon[i])
			{
				if(RemoveTemperatureIcon(i))
				{
					CString cstr;
					cstr.Format(_T("%d"), 0);
					WritePrivateProfileString(_T("TemperatureIcon"), m_Ata.vars[i].ModelSerial, cstr, m_Ini);
					
					if(! IsTemperatureIconExist())
					{
						AddTrayMainIcon();
					}
				}
			}
			else if(AddTemperatureIcon(i))
			{
				CString cstr;
				cstr.Format(_T("%d"), 1);
				WritePrivateProfileString(_T("TemperatureIcon"), m_Ata.vars[i].ModelSerial, cstr, m_Ini);

				if(m_FlagShowTemperatureIconOnly && IsTemperatureIconExist())
				{
					if(RemoveTrayMainIcon())
					{
						m_MainIconId = TRAY_TEMPERATURE_ICON_BASE + i;
					}
				}
			}
		}
	}
	else if(SELECT_DISK_BASE <= wParam && wParam < SELECT_DISK_BASE + CAtaSmart::MAX_DISK)
	{
		int i = (int)(wParam - SELECT_DISK_BASE);
		m_DriveMenuPage = i / 8;
		SelectDrive(i);
	}
	else if(AUTO_REFRESH_TARGET_BASE <= wParam && wParam <= AUTO_REFRESH_TARGET_BASE + CAtaSmart::MAX_DISK + 1)
	{
		int i = (int)(wParam - AUTO_REFRESH_TARGET_BASE);
		// Target All Disk : AUTO_REFRESH_TARGET_BASE + CAtaSmart::MAX_DISK
		// Unarget All Disk : AUTO_REFRESH_TARGET_BASE + CAtaSmart::MAX_DISK
		CMenu *menu = GetMenu();
		if(i == CAtaSmart::MAX_DISK) // Target All Disk
		{
			for(int j = 0; j < m_Ata.vars.GetCount(); j++)
			{
				m_FlagAutoRefreshTarget[j] = TRUE;
				menu->CheckMenuItem(AUTO_REFRESH_TARGET_BASE + j, MF_CHECKED);
				WritePrivateProfileString(_T("AutoRefreshTarget"), m_Ata.vars[j].ModelSerial, _T("1"), m_Ini);
			}
		}
		else if(i == CAtaSmart::MAX_DISK + 1) // Unarget All Disk
		{
			for(int j = 0; j < m_Ata.vars.GetCount(); j++)
			{
				m_FlagAutoRefreshTarget[j] = FALSE;
				menu->CheckMenuItem(AUTO_REFRESH_TARGET_BASE + j, MF_UNCHECKED);
				WritePrivateProfileString(_T("AutoRefreshTarget"), m_Ata.vars[j].ModelSerial, _T("0"), m_Ini);
			}
		}
		else
		{
			if(m_FlagAutoRefreshTarget[i])
			{
				m_FlagAutoRefreshTarget[i] = FALSE;
				menu->CheckMenuItem(AUTO_REFRESH_TARGET_BASE + i, MF_UNCHECKED);
				WritePrivateProfileString(_T("AutoRefreshTarget"), m_Ata.vars[i].ModelSerial, _T("0"), m_Ini);
			}
			else
			{
				m_FlagAutoRefreshTarget[i] = TRUE;
				menu->CheckMenuItem(AUTO_REFRESH_TARGET_BASE + i, MF_CHECKED);
				WritePrivateProfileString(_T("AutoRefreshTarget"), m_Ata.vars[i].ModelSerial, _T("1"), m_Ini);
			}
		}
		SetMenu(menu);
		DrawMenuBar();
	}

	return CDHtmlMainDialog::OnCommand(wParam, lParam);
}

void CDiskInfoDlg::UpdateDialogSize()
{
	if(GetPrivateProfileInt(_T("Setting"), _T("HideSmartInfo"), 0, m_Ini))
	{
		m_SizeX = SIZE_X;
		m_SizeY = SIZE_Y;
		m_FlagHideSmartInfo = TRUE;
		SetClientRect((DWORD)(m_SizeX * m_ZoomRatio), (DWORD)(m_SizeY * m_ZoomRatio), 0);
		m_List.SetFontSize(m_ZoomRatio);

		CMenu *menu = GetMenu();
		menu->CheckMenuItem(ID_HIDE_SMART_INFO, MF_CHECKED);
		SetMenu(menu);
		DrawMenuBar();
	}
	else
	{
		m_SizeX = SIZE_SMART_X;
		if(GetPrivateProfileInt(_T("Setting"), _T("Height"), 0, m_Ini) > 0)
		{
			m_SizeY = GetPrivateProfileInt(_T("Setting"), _T("Height"), 0, m_Ini);
		}
		else
		{
			m_SizeY = SIZE_SMART_Y;
		}
		SetClientRect((DWORD)(m_SizeX * m_ZoomRatio), (DWORD)(m_SizeY * m_ZoomRatio), 1);
		m_List.SetFontSize(m_ZoomRatio);
		m_FlagHideSmartInfo = FALSE;
		CMenu *menu = GetMenu();
		menu->CheckMenuItem(ID_HIDE_SMART_INFO, MF_UNCHECKED);
		SetMenu(menu);
		DrawMenuBar();
	}
}

void CDiskInfoDlg::OnSize(UINT nType, int cx, int cy)
{
	CDHtmlMainDialog::OnSize(nType, cx, cy);

	static BOOL flag = FALSE;

	if(flag)
	{
		m_List.SetWindowPos(NULL, (int)(8 * m_ZoomRatio), (int)(SIZE_Y * m_ZoomRatio),
		(int)(cx - 16 * m_ZoomRatio), (int)(cy - SIZE_Y * m_ZoomRatio - 8 * m_ZoomRatio), SWP_SHOWWINDOW);
	}
	flag = TRUE;
	
	if(m_FlagHideSmartInfo == FALSE && m_FlagInitializing == FALSE)
	{
		RECT rect;
		CString cstr;
		GetClientRect(&rect);
		if(rect.bottom - rect.top > 0)
		{
			cstr.Format(_T("%d"), (DWORD)((rect.bottom - rect.top) / m_ZoomRatio));
			WritePrivateProfileString(_T("Setting"), _T("Height"), cstr, m_Ini);
		}
	}
}

void CDiskInfoDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	lpMMI->ptMinTrackSize.x = (LONG)(640 * m_ZoomRatio + GetSystemMetrics(SM_CXFRAME) * 2);
	lpMMI->ptMinTrackSize.y = (LONG)(48  * m_ZoomRatio + GetSystemMetrics(SM_CYMENU)
							+ GetSystemMetrics(SM_CYSIZEFRAME) * 2 + GetSystemMetrics(SM_CYCAPTION));
	lpMMI->ptMaxTrackSize.x = (LONG)(640 * m_ZoomRatio + GetSystemMetrics(SM_CXFRAME) * 2);
	lpMMI->ptMaxTrackSize.y = (LONG)(1200 * m_ZoomRatio);

	CDHtmlMainDialog::OnGetMinMaxInfo(lpMMI);
}

void CDiskInfoDlg::AlarmHealthStatus(DWORD i, CString dir, CString disk)
{
	TCHAR str[256];
	CString cstr, alarm, name, id;
	DWORD niifType = NIIF_INFO;
	int pre = -1;
	
	name.Format(_T("(%d) %s / %s / %s\r\n"), i + 1, m_Ata.vars[i].Model, m_Ata.vars[i].SerialNumber, m_Ata.vars[i].DriveMap);

	GetPrivateProfileString(disk, _T("HealthStatus"), _T("0"), str, 256, dir + _T("\\") + SMART_INI);
	pre = _tstoi(str);
	if(m_Ata.vars[i].DiskStatus > (DWORD)pre && pre != 0)
	{
		cstr.Format(_T("%s: [%s] -> [%s]\r\n"), i18n(_T("Dialog"), _T("HEALTH_STATUS")),
					GetDiskStatus(pre), GetDiskStatus(m_Ata.vars[i].DiskStatus));
		alarm += cstr;
		niifType = NIIF_WARNING;
		AddEventLog(601, 2, name + cstr);
	}
	else if(m_Ata.vars[i].DiskStatus < (DWORD)pre && pre != 0)
	{
		cstr.Format(_T("%s: [%s] -> [%s]\r\n"), i18n(_T("Dialog"), _T("HEALTH_STATUS")),
					GetDiskStatus(pre), GetDiskStatus(m_Ata.vars[i].DiskStatus));
		alarm += cstr;
		niifType = NIIF_INFO;
		AddEventLog(701, 4, name + cstr);
	}

	for(DWORD j = 0; j < m_Ata.vars[i].AttributeCount; j++)
	{
		if(( m_Ata.vars[i].Attribute[j].Id == 0x05 // Reallocated Sectors Count
		||  m_Ata.vars[i].Attribute[j].Id == 0xC4 // Reallocation Event Count
		||  m_Ata.vars[i].Attribute[j].Id == 0xC5 // Current Pending Sector Count
		||  m_Ata.vars[i].Attribute[j].Id == 0xC6 // Off-Line Scan Uncorrectable Sector Count
		) && ! m_Ata.vars[i].IsSsd)
		{
			CString target;
			DWORD eventId = 0;
			if(m_Ata.vars[i].Attribute[j].Id == 0x05)
			{
				target = _T("ReallocatedSectorsCount");
				eventId = 602;
			}
			else if(m_Ata.vars[i].Attribute[j].Id == 0xC4)
			{
				target = _T("ReallocationEventCount");
				eventId = 603;
			}
			else if(m_Ata.vars[i].Attribute[j].Id == 0xC5)
			{
				target = _T("CurrentPendingSectorCount");
				eventId = 604;
			}
			else if(m_Ata.vars[i].Attribute[j].Id == 0xC6)
			{
				target = _T("UncorrectableSectorCount");
				eventId = 605;
			}

			GetPrivateProfileString(disk, target, _T("-1"), str, 256, dir + _T("\\") + SMART_INI);
			pre = _tstoi(str);
			id.Format(_T("%02X"), m_Ata.vars[i].Attribute[j].Id);
			int rawValue = MAKEWORD(m_Ata.vars[i].Attribute[j].RawValue[0], m_Ata.vars[i].Attribute[j].RawValue[1]);
			if(rawValue > pre && pre != -1)
			{
				cstr.Format(_T("%s: (%02X) %s [%d->%d]\r\n"), i18n(_T("Alarm"), _T("DEGRADATION")),
					m_Ata.vars[i].Attribute[j].Id, i18n(_T("Smart"), id), pre, rawValue);
				alarm += cstr;
				niifType = NIIF_WARNING;
				AddEventLog(eventId, 2, name + cstr);
			}
			else if(rawValue < pre && pre != -1)
			{
				cstr.Format(_T("%s: (%02X) %s [%d->%d]\r\n"), i18n(_T("Alarm"), _T("RECOVERY")),
					m_Ata.vars[i].Attribute[j].Id, i18n(_T("Smart"), id), pre, rawValue);
				alarm += cstr;
				niifType = NIIF_INFO;
				AddEventLog(eventId + 100, 4, name + cstr);
			}
		}

		static CTime preTime[CAtaSmart::MAX_DISK] = {0};
		if(m_Ata.vars[i].AlarmTemperature > 0 && m_Ata.vars[i].Temperature >= m_Ata.vars[i].AlarmTemperature)
		{
			if(CTime::GetTickCount() > preTime[i] + ALARM_TEMPERATURE_PERIOD)
			{
				cstr.Format(_T("%s: %d C\r\n"), i18n(_T("Alarm"), _T("ALARM_TEMPERATURE")), m_Ata.vars[i].Temperature);
				AddEventLog(606, 2, name + cstr);
				niifType = NIIF_WARNING;
				preTime[i] = CTime::GetTickCount();
			}
		}
	}

	if(! alarm.IsEmpty())
	{
		cstr.Format(_T("(%d) %s\n"), i + 1, m_Ata.vars[i].Model);
		if(niifType == NIIF_WARNING)
		{
			ShowBalloon(m_MainIconId, niifType, i18n(_T("Alarm"), _T("ALARM_HEALTH_STATUS")), cstr + alarm);
		}
		else
		{
			ShowBalloon(m_MainIconId, niifType, i18n(_T("Alarm"), _T("INFO_HEALTH_STATUS")), cstr + alarm);
		}
	}
}

BOOL CDiskInfoDlg::AddEventLog(DWORD eventId, WORD eventType, CString message)
{
	if(! m_FlagEventLog)
	{
		return FALSE;
	}

//	BOOL result = FALSE;
	/*
	CString type;

	switch(eventType)
	{
	case 0: type = _T("SUCCESS");		break;
	case 1: type = _T("ERROR");			break;
	case 2: type = _T("WARNING");		break;
	case 4: type = _T("INFORMATION");	break;
	default:
		return FALSE;
		break;
	}

	if(m_FlagUseEventCreate)
	{
		CString cstr; 
		STARTUPINFO si = {0};
		PROCESS_INFORMATION pi = {0};
		si.cb			= sizeof(STARTUPINFO);
		si.dwFlags		= STARTF_USESHOWWINDOW;
		si.wShowWindow	= SW_HIDE;
		cstr.Format(_T("eventcreate /ID %d /L APPLICATION /SO CrystalDiskinfo /T %s /D \"%s\""), eventId, type, message); 
		result = ::CreateProcess(NULL, (LPWSTR)cstr.GetString(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
		WaitForSingleObject(pi.hProcess, 5000);
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
	}
	*/

	return WriteEventLog(eventId, eventType, _T("CrystalDiskInfo"), message);
}

void CDiskInfoDlg::AlarmOverheat()
{
	CString cstr;
	CString overheat;
	CString diskStatus;
	for(int i = 0; i < m_Ata.vars.GetCount(); i++)
	{
		if(m_Ata.vars[i].AlarmTemperature > 0 && m_Ata.vars[i].Temperature >= m_Ata.vars[i].AlarmTemperature)
		{
			diskStatus = GetDiskStatus(m_Ata.vars[i].DiskStatus);
			cstr.Format(_T("(%d) %s [%s] %d C\r\n"), i + 1, m_Ata.vars[i].Model, diskStatus, m_Ata.vars[i].Temperature);
			overheat += cstr;
		}
	}
	overheat.Trim();
	if(! overheat.IsEmpty())
	{
		ShowBalloon(m_MainIconId, NIIF_WARNING, i18n(_T("Alarm"), _T("ALARM_TEMPERATURE")), overheat);
	}
}

void CDiskInfoDlg::OnTimer(UINT_PTR nIDEvent)
{
	if(nIDEvent == TIMER_SET_POWER_ON_UNIT)
	{
		KillTimer(TIMER_SET_POWER_ON_UNIT);

		SetWindowTitle(_T(""));
		m_NowDetectingUnitPowerOnHours = FALSE;
		if(m_Ata.MeasuredTimeUnit())
		{
			if(m_SelectDisk < (DWORD)m_Ata.vars.GetCount() && m_Ata.vars[m_SelectDisk].IsSmartCorrect)
			{
				m_PowerOnHoursClass = _T("valueR");
				SetElementPropertyEx(_T("PowerOnHours"), DISPID_IHTMLELEMENT_CLASSNAME, m_PowerOnHoursClass);
			}
			for(int i = 0; i < m_Ata.vars.GetCount(); i++)
			{
				m_Ata.vars[i].MeasuredPowerOnHours = m_Ata.GetPowerOnHoursEx(i, m_Ata.vars[i].MeasuredTimeUnitType);
				
				CString cstr;
				cstr.Format(_T("%d"), m_Ata.vars[i].MeasuredTimeUnitType);
				WritePrivateProfileString(_T("PowerOnUnit"), m_Ata.vars[i].ModelSerial, cstr, m_Ini);
				SaveSmartInfo(i);
			}
		}
	}
	else if(nIDEvent == TIMER_AUTO_REFRESH)
	{
		Refresh(FALSE);
	}
	else if(nIDEvent == TIMER_FORCE_REFRESH)
	{
		KillTimer(TIMER_FORCE_REFRESH);
		Refresh(FALSE);

		AutoAamApmAdaption();
	}
	else if(nIDEvent == TIMER_AUTO_DETECT)
	{
		CWaitCursor wait;
		BOOL flagChangeDisk = FALSE;
		KillTimer(TIMER_AUTO_DETECT);

		InitAta(TRUE, m_FlagAdvancedDiskSearch, &flagChangeDisk);

		if(flagChangeDisk)
		{
			// Update Menu and Dialog
			m_SelectDisk = 0;
			m_DriveMenuPage = 0;
			ChangeLang(m_CurrentLang);
			if(m_FlagResident)
			{
				for(int i = 0; i < CAtaSmart::MAX_DISK; i++)
				{
					RemoveTemperatureIcon(i);
				}
				CheckTrayTemperatureIcon();
			}

			if(m_SettingDlg != NULL)
			{
				::SendMessage(m_SettingDlg->m_hWnd, WM_CLOSE, 0, 0);
			}
			if(m_HealthDlg != NULL)
			{
				::SendMessage(m_HealthDlg->m_hWnd, WM_CLOSE, 0, 0);
			}
		}
		else
		{
			Refresh(TRUE);
		}

/*
		InitAta(TRUE, m_FlagAdvancedDiskSearch, &flagChangeDisk);

		if(flagChangeDisk)
		{
			ChangeLang(m_CurrentLang);

			if(m_FlagResident)
			{
				SetTimer(TIMER_UPDATE_TRAY_ICON, 1000 * 1, 0);
			}
			if(m_SettingDlg != NULL)
			{
				::SendMessage(m_SettingDlg->m_hWnd, WM_CLOSE, 0, 0);
			}
		}
*/
	}
	else if(nIDEvent == TIMER_UPDATE_TRAY_ICON)
	{
		KillTimer(TIMER_UPDATE_TRAY_ICON);
		for(int i = 0; i < CAtaSmart::MAX_DISK; i++)
		{
			RemoveTemperatureIcon(i);
		}
		CheckTrayTemperatureIcon();
		ShowWindow(SW_SHOW);
	}

	CDHtmlMainDialog::OnTimer(nIDEvent);
}

LRESULT CDiskInfoDlg::OnPowerBroadcast(WPARAM wParam, LPARAM lParam)
{
	if(m_AutoDetectionStatus <= 0)
	{
		return FALSE;
	}

	switch(wParam)
	{
	case PBT_APMRESUMESUSPEND:
		SetTimer(TIMER_FORCE_REFRESH, m_AutoDetectionStatus * 1000, 0);
//		MessageBox(_T("PBT_APMRESUMESUSPEND"));
		break;
//	case PBT_APMSUSPEND:
//		MessageBox(_T("PBT_APMSUSPEND"));
//		break;
	default:
		break;
	}

	return TRUE;
}

LRESULT CDiskInfoDlg::OnDeviceChange(WPARAM wParam, LPARAM lParam)
{
	if(m_AutoDetectionStatus <= 0)
	{
		return FALSE;
	}

	//CString cstr;
	switch(wParam)
	{
	case DBT_DEVICEARRIVAL:
		{
			//cstr.Format(_T("DBT_DEVICEARRIVAL: LPARAM=%08X\n"), lParam);
			PDEV_BROADCAST_HDR pdbh = (PDEV_BROADCAST_HDR)lParam;
			switch(pdbh->dbch_devicetype)
			{
			case DBT_DEVTYP_DEVICEINTERFACE:
				{
					//cstr += _T("DBT_DEVTYP_DEVICEINTERFACE");
					PDEV_BROADCAST_DEVICEINTERFACE pdbd = (PDEV_BROADCAST_DEVICEINTERFACE)lParam;
					if(pdbd->dbcc_classguid == StrageGUID)
					{
					//	AfxMessageBox(pdbd->dbcc_name);
					
					// Disabled 2008/11/20 (This feature will be enabled on future release...)
						KillTimer(TIMER_AUTO_DETECT);
						SetTimer(TIMER_AUTO_DETECT, m_AutoDetectionStatus * 1000, 0);
					}
				}
				break;
			case DBT_DEVTYP_HANDLE:
				// cstr += _T("DBT_DEVTYP_HANDLE");
				break;
			case DBT_DEVTYP_OEM:
				// cstr += _T("DBT_DEVTYP_OEM");
				break;
			case DBT_DEVTYP_PORT:
				// cstr += _T("DBT_DEVTYP_PORT");
				break;
			case DBT_DEVTYP_VOLUME:
				// cstr += _T("DBT_DEVTYP_VOLUME\n");
				// PDEV_BROADCAST_VOLUME pdbv = (PDEV_BROADCAST_VOLUME)lParam;
				// CString temp;
				// temp.Format(_T("Flags=%d, UnitMask=%08X"), pdbv->dbcv_flags, pdbv->dbcv_unitmask);
				// cstr += temp;
				break;
			}
		//	AfxMessageBox(cstr);
		}
		break;
	case DBT_DEVICEREMOVECOMPLETE:
		{
			// cstr.Format(_T("DBT_DEVICEREMOVECOMPLETE: LPARAM=%08X\n"), lParam);
			PDEV_BROADCAST_HDR pdbh = (PDEV_BROADCAST_HDR)lParam;
			switch(pdbh->dbch_devicetype)
			{
			case DBT_DEVTYP_DEVICEINTERFACE:
				{
				//	cstr += _T("DBT_DEVTYP_DEVICEINTERFACE");
					PDEV_BROADCAST_DEVICEINTERFACE pdbd = (PDEV_BROADCAST_DEVICEINTERFACE)lParam;
					if(pdbd->dbcc_classguid == StrageGUID)
					{
					//	AfxMessageBox(pdbd->dbcc_name);
					// Disabled 2008/11/20 (This feature will be enabled on future release...)
						KillTimer(TIMER_AUTO_DETECT);
						if(m_AutoDetectionStatus > 0)
						{
							SetTimer(TIMER_AUTO_DETECT, m_AutoDetectionStatus * 1000, 0);
						}
					}
				}
				break;
			case DBT_DEVTYP_HANDLE:
			//	cstr += _T("DBT_DEVTYP_HANDLE");
				break;
			case DBT_DEVTYP_OEM:
			//	cstr += _T("DBT_DEVTYP_OEM");
				break;
			case DBT_DEVTYP_PORT:
			//	cstr += _T("DBT_DEVTYP_PORT");
				break;
			case DBT_DEVTYP_VOLUME:
			//	cstr += _T("DBT_DEVTYP_VOLUME\n");
			//	PDEV_BROADCAST_VOLUME pdbv = (PDEV_BROADCAST_VOLUME)lParam;
			//	CString temp;
			//	temp.Format(_T("Flags=%d, UnitMask=%08X"), pdbv->dbcv_flags, pdbv->dbcv_unitmask);
			//	cstr += temp;
				break;
			}
		//	AfxMessageBox(cstr);
		}
		break;
	default:
	//	cstr.Format(_T("WPARAM=%08X, LPARAM=%08X"), wParam, lParam);
	//	AfxMessageBox(cstr);
		break;
	}
	return TRUE;
}

void CDiskInfoDlg::AutoAamApmAdaption()
{
	if(! m_FlagAutoAamApm)
	{
		return ;
	}

	int status = 0;
	int value = 0;

	// AAM
	for(int i = 0; i < m_Ata.vars.GetCount(); i++)
	{
		if(! m_Ata.vars[i].IsAamSupported)
		{
			continue;
		}

		status = GetPrivateProfileInt(_T("AamStatus"), m_Ata.vars[i].ModelSerial, -1, m_Ini);
		value = GetPrivateProfileInt(_T("AamValue"), m_Ata.vars[i].ModelSerial, -1, m_Ini);
		if(status == 1 /* Enabled */
		&& (! m_Ata.vars[i].IsAamEnabled || value != m_Ata.GetAamValue(i)))
		{
			m_Ata.EnableAam(i, value);
			m_Ata.UpdateIdInfo(i);
		}
		if(status == 0 /* Disabled */
		&& m_Ata.vars[i].IsAamEnabled)
		{
			m_Ata.DisableAam(i);
			m_Ata.UpdateIdInfo(i);
		}
	}

	// APM
	for(int i = 0; i < m_Ata.vars.GetCount(); i++)
	{
		if(! m_Ata.vars[i].IsApmSupported)
		{
			continue;
		}

		status = GetPrivateProfileInt(_T("ApmStatus"), m_Ata.vars[i].ModelSerial, -1, m_Ini);
		value = GetPrivateProfileInt(_T("ApmValue"), m_Ata.vars[i].ModelSerial, -1, m_Ini);
		if(status == 1 /* Enabled */
		&& (! m_Ata.vars[i].IsApmEnabled || value != m_Ata.GetApmValue(i)))
		{
			m_Ata.EnableApm(i, value);
			m_Ata.UpdateIdInfo(i);
		}
		if(status == 0 /* Disabled */
		&& m_Ata.vars[i].IsApmEnabled)
		{
			m_Ata.DisableApm(i);
			m_Ata.UpdateIdInfo(i);
		}
	}
}

void CDiskInfoDlg::ReExecute()
{
	ShowWindow(SW_HIDE);
	RemoveTrayMainIcon();
	for(int i = 0; i < m_Ata.vars.GetCount(); i++)
	{
		RemoveTemperatureIcon(i);
	}
	KillGraphDlg();
	EndDialog(RE_EXEC);
}

DWORD CDiskInfoDlg::GetSelectDisk()
{
	return m_SelectDisk;
}
