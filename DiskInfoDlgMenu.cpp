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
#ifdef BENCHMARK
#include "Benchmark.h"
#endif

#include <complex>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


CString CDiskInfoDlg::Encode10X(DWORD value)
{
	CString cstr;
	cstr.Format(_T("%010d%08X"), value, value);

	return cstr;
}

DWORD CDiskInfoDlg::Decode10X(CString cstr)
{
	DWORD d, x;
	TCHAR *endPtr;
	d = _tcstol(cstr.Left(10), &endPtr, 10);
	x = _tcstol(cstr.Mid(10, 8), &endPtr, 16);

	if(d == x)
	{
		return d;
	}
	else
	{
		return 0;
	}
}

#ifdef BENCHMARK
HRESULT CDiskInfoDlg::OnBenchmark(IHTMLElement* /*pElement*/)
{
	CWaitCursor wait;

	BENCHMARK_TARGET_INFO data;
	data.physicalDriveId = m_Ata.vars[m_SelectDisk].PhysicalDriveId;
	data.scsiPort = m_Ata.vars[m_SelectDisk].ScsiPort;
	data.scsiTargetId = m_Ata.vars[m_SelectDisk].ScsiTargetId;

	m_Ata.vars[m_SelectDisk].Speed = ExecDiskBenchAll(&data);
	SetMeter(m_Ata.vars[m_SelectDisk].Speed);

	WritePrivateProfileStringW(_T("Benchmark"),
		m_Ata.vars[m_SelectDisk].ModelSerial,
		Encode10X(DWORD(m_Ata.vars[m_SelectDisk].Speed * 1000)), m_Ini);

	return S_FALSE;
}

void CDiskInfoDlg::SetMeter(double score)
{
	CComPtr<IHTMLStyle> pHtmlStyle;
	CComPtr<IHTMLElement> pHtmlElement;
	HRESULT hr;
	CComBSTR bstr;
	CString cstr;
	VARIANT va;
	VariantInit(&va);

	hr = GetElementInterface(_T("BenchmarkMeter"), IID_IHTMLElement, (void **) &pHtmlElement);
	if(FAILED(hr)){ return ;}
	hr = pHtmlElement->get_style(&pHtmlStyle);
	if(FAILED(hr)){ return ;}

	int meterLength;
	meterLength = (int)(MAX_METER_LENGTH / 3 * log10(score));
	if(meterLength > MAX_METER_LENGTH)
	{
		meterLength = MAX_METER_LENGTH;
	}
	else if(meterLength < 1)
	{
		meterLength = 0;
	}

	cstr.Format(_T("%dpx"), -1 * (MAX_METER_LENGTH - meterLength));
	bstr = cstr;
	va.vt = VT_BSTR;
	va.bstrVal = bstr;
	pHtmlStyle->put_backgroundPositionX(va);

	VariantClear(&va);

	cstr.Format(_T("%.1f MB/s"), score);
	m_BenchmarkMeter = cstr;
	UpdateData(FALSE);
}
#endif

void CDiskInfoDlg::OnAbout()
{
	m_AboutDlg = new CAboutDlg(this);
	m_AboutDlg->Create(CAboutDlg::IDD, m_AboutDlg, ID_HELP_ABOUT, this);
}

void CDiskInfoDlg::OnCustomize()
{
	m_OptionDlg = new COptionDlg(this);
	m_OptionDlg->Create(COptionDlg::IDD, m_OptionDlg, ID_CUSTOMIZE, this);
}


void CDiskInfoDlg::OnAamApm()
{
	m_SettingDlg = new CSettingDlg(this);
	m_SettingDlg->Create(CSettingDlg::IDD, m_SettingDlg, ID_AAM_APM, this);
}

void CDiskInfoDlg::OnHealthStatus()
{
	m_HealthDlg = new CHealthDlg(this);
	m_HealthDlg->Create(CHealthDlg::IDD, m_HealthDlg, ID_HEALTH_STATUS, this);
}

void CDiskInfoDlg::OnGraph()
{
	ShowGraphDlg(-1);
}

void CDiskInfoDlg::ShowGraphDlg(int index)
{
	CreateExchangeInfo();

	TCHAR path[MAX_PATH];
	CString cstr;
	GetModuleFileName(NULL, path, MAX_PATH);

	STARTUPINFO si = {0};
	PROCESS_INFORMATION pi = {0};
	si.cb			= sizeof(STARTUPINFO);
	si.dwFlags		= STARTF_USESHOWWINDOW;
	si.wShowWindow	= SW_SHOWNORMAL;
	cstr.Format(_T("\"%s\" /Earthlight %d"), path, index); 
	::CreateProcess(NULL, (LPWSTR)cstr.GetString(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);

	m_GraphProcessId.Add(pi.dwProcessId);
}

void CDiskInfoDlg::CreateExchangeInfo()
{
	CString cstr;
	cstr.Format(_T("%d"), m_Ata.vars.GetCount());
	WritePrivateProfileString(_T("EXCHANGE"), _T("DetectedDisk"), cstr, m_SmartDir + _T("\\") + EXCHANGE_INI);

	for(int i = 0; i < m_Ata.vars.GetCount(); i++)
	{
		cstr.Format(_T("%d"), i);
		WritePrivateProfileString(_T("MODEL"), cstr, m_Ata.vars[i].Model, m_SmartDir + _T("\\") + EXCHANGE_INI);
		WritePrivateProfileString(_T("SERIAL"), cstr, m_Ata.vars[i].SerialNumber, m_SmartDir + _T("\\") + EXCHANGE_INI);
	}
}

void CDiskInfoDlg::OnHideSmartInfo()
{
	CMenu *menu = GetMenu();		
	if(m_FlagHideSmartInfo)
	{
		m_SizeX = SIZE_SMART_X;
		m_SizeY = SIZE_SMART_Y;
		SetClientRect((DWORD)(m_SizeX * m_ZoomRatio), (DWORD)(m_SizeY * m_ZoomRatio), 1);
		menu->CheckMenuItem(ID_HIDE_SMART_INFO, MF_UNCHECKED);
		m_FlagHideSmartInfo = FALSE;
		WritePrivateProfileStringW(_T("Setting"), _T("HideSmartInfo"), _T("0"), m_Ini);
	}
	else
	{
		m_SizeX = SIZE_X;
		m_SizeY = SIZE_Y;
		SetClientRect((DWORD)(m_SizeX * m_ZoomRatio), (DWORD)(m_SizeY * m_ZoomRatio), 1);
		menu->CheckMenuItem(ID_HIDE_SMART_INFO, MF_CHECKED);
		m_FlagHideSmartInfo = TRUE;
		WritePrivateProfileStringW(_T("Setting"), _T("HideSmartInfo"), _T("1"), m_Ini);
	}

	CString cstr;
	cstr.Format(_T("%d"), m_SizeY);
	WritePrivateProfileString(_T("Setting"), _T("Height"), cstr, m_Ini);

	SetMenu(menu);
	DrawMenuBar();
}

void CDiskInfoDlg::CheckHideSerialNumber()
{
	CMenu *menu = GetMenu();
	if(GetPrivateProfileInt(_T("Setting"), _T("HideSerialNumber"), 0, m_Ini))
	{
		m_FlagHideSerialNumber = TRUE;
		menu->CheckMenuItem(ID_HIDE_SERIAL_NUMBER, MF_CHECKED);
	}
	else
	{
		m_FlagHideSerialNumber = FALSE;
		menu->CheckMenuItem(ID_HIDE_SERIAL_NUMBER, MF_UNCHECKED);
	}
	SetMenu(menu);
	DrawMenuBar();
}

void CDiskInfoDlg::OnHideSerialNumber()
{
	CMenu *menu = GetMenu();		
	if(m_FlagHideSerialNumber)
	{
		m_FlagHideSerialNumber = FALSE;
		menu->CheckMenuItem(ID_HIDE_SERIAL_NUMBER, MF_UNCHECKED);
		WritePrivateProfileStringW(_T("Setting"), _T("HideSerialNumber"), _T("0"), m_Ini);
	}
	else
	{
		m_FlagHideSerialNumber = TRUE;
		menu->CheckMenuItem(ID_HIDE_SERIAL_NUMBER, MF_CHECKED);
		WritePrivateProfileStringW(_T("Setting"), _T("HideSerialNumber"), _T("1"), m_Ini);
	}
	SetMenu(menu);
	DrawMenuBar();

	ChangeDisk(m_SelectDisk);
}

void CDiskInfoDlg::OnCrystalDewWorld()
{
	if(GetUserDefaultLCID() == 0x0411) // Japanese
	{
		ShellExecute(NULL, NULL, URL_CRYSTAL_DEW_WORLD_JA, NULL, NULL, SW_SHOWNORMAL);
	}
	else // Other Language
	{
		ShellExecute(NULL, NULL, URL_CRYSTAL_DEW_WORLD_EN, NULL, NULL, SW_SHOWNORMAL);
	}	
}

void CDiskInfoDlg::OnHelp()
{
	CString cstr;
	if(GetUserDefaultLCID() == 0x0411) // Japanese
	{
		cstr.Format(_T("%s%s"), m_ExeDir, HTML_HELP_JA);
		ShellExecute(NULL, NULL, cstr, NULL, NULL, SW_SHOWNORMAL);
	}
	else // Other Language
	{
		cstr.Format(_T("%s%s"), m_ExeDir, HTML_HELP_EN);
		ShellExecute(NULL, NULL, cstr, NULL, NULL, SW_SHOWNORMAL);
	}	
}

void CDiskInfoDlg::OnRefresh()
{
	Refresh(TRUE);
}

void CDiskInfoDlg::OnRescan()
{
	CWaitCursor wait;
	BOOL flagChangeDisk = FALSE;

	InitAta(TRUE, m_FlagAdvancedDiskSearch, &flagChangeDisk);

	if(flagChangeDisk)
	{
		// Update Menu and Dialog
		m_SelectDisk = 0;
		m_DriveMenuPage = 0;
		ChangeLang(m_CurrentLang);
	}
	else
	{
		Refresh(TRUE);
	}

	if(m_FlagResident && flagChangeDisk)
	{
		for(int i = 0; i < CAtaSmart::MAX_DISK; i++)
		{
			RemoveTemperatureIcon(i);
		}
		CheckTrayTemperatureIcon();
	}
}

void CDiskInfoDlg::OnHelpAboutSmart()
{
	TCHAR str[256];
	GetPrivateProfileString(_T("Url"), _T("WIKIPEDIA_SMART"), _T("http://en.wikipedia.org/wiki/Self-Monitoring%2C_Analysis%2C_and_Reporting_Technology"), str, 256, m_CurrentLangPath);

	ShellExecute(NULL, NULL, str, NULL, NULL, SW_SHOWNORMAL);
}

void CDiskInfoDlg::OnAutoRefreshDisable(){	CheckRadioAutoRefresh(ID_AUTO_REFRESH_DISABLE, 0);}
void CDiskInfoDlg::OnAutoRefresh01Min(){	CheckRadioAutoRefresh(ID_AUTO_REFRESH_01_MIN, 1);}
void CDiskInfoDlg::OnAutoRefresh03Min(){	CheckRadioAutoRefresh(ID_AUTO_REFRESH_03_MIN, 3);}
void CDiskInfoDlg::OnAutoRefresh05Min(){	CheckRadioAutoRefresh(ID_AUTO_REFRESH_05_MIN, 5);}
void CDiskInfoDlg::OnAutoRefresh10Min(){	CheckRadioAutoRefresh(ID_AUTO_REFRESH_10_MIN, 10);}
void CDiskInfoDlg::OnAutoRefresh30Min(){	CheckRadioAutoRefresh(ID_AUTO_REFRESH_30_MIN, 30);}
void CDiskInfoDlg::OnAutoRefresh60Min(){	CheckRadioAutoRefresh(ID_AUTO_REFRESH_60_MIN, 60);}

void CDiskInfoDlg::CheckRadioAutoRefresh(int id, int value)
{
	CMenu *menu = GetMenu();
	menu->CheckMenuRadioItem(ID_AUTO_REFRESH_DISABLE, ID_AUTO_REFRESH_60_MIN, id, MF_BYCOMMAND);
	SetMenu(menu);
	DrawMenuBar();

	m_AutoRefreshStatus = value;

	CString cstr;
	cstr.Format(_T("%d"), value);
	WritePrivateProfileString(_T("Setting"), _T("AutoRefresh"), cstr, m_Ini);

	if(value == 0)
	{
		KillTimer(TIMER_AUTO_REFRESH);
	}
	else
	{
		KillTimer(TIMER_AUTO_REFRESH);
		SetTimer(TIMER_AUTO_REFRESH, 1000 * 60 * value, 0);
	}
}

void CDiskInfoDlg::CheckRadioAutoRefresh()
{
	int id = ID_AUTO_REFRESH_DISABLE;

	switch(m_AutoRefreshStatus)
	{
	case  1: id = ID_AUTO_REFRESH_01_MIN; break;
	case  3: id = ID_AUTO_REFRESH_03_MIN; break;
	case  5: id = ID_AUTO_REFRESH_05_MIN; break;
	case 10: id = ID_AUTO_REFRESH_10_MIN; break;
	case 30: id = ID_AUTO_REFRESH_30_MIN; break;
	case 60: id = ID_AUTO_REFRESH_60_MIN; break;
	default: id = ID_AUTO_REFRESH_DISABLE; break;
	}

	CMenu *menu = GetMenu();
	menu->CheckMenuRadioItem(ID_AUTO_REFRESH_DISABLE, ID_AUTO_REFRESH_60_MIN, id, MF_BYCOMMAND);
	SetMenu(menu);
	DrawMenuBar();
}


void CDiskInfoDlg::OnWait0Sec()
{
	CheckRadioWaitTime(ID_WAIT_0_SEC, 0);
}

void CDiskInfoDlg::OnWait5Sec(){	CheckRadioWaitTime(ID_WAIT_5_SEC, 5);	}
void CDiskInfoDlg::OnWait10Sec(){	CheckRadioWaitTime(ID_WAIT_10_SEC, 10);	}
void CDiskInfoDlg::OnWait15Sec(){	CheckRadioWaitTime(ID_WAIT_15_SEC, 15);	}
void CDiskInfoDlg::OnWait20Sec(){	CheckRadioWaitTime(ID_WAIT_20_SEC, 20);	}
void CDiskInfoDlg::OnWait30Sec(){	CheckRadioWaitTime(ID_WAIT_30_SEC, 30);	}
void CDiskInfoDlg::OnWait40Sec(){	CheckRadioWaitTime(ID_WAIT_40_SEC, 40);	}
void CDiskInfoDlg::OnWait50Sec(){	CheckRadioWaitTime(ID_WAIT_50_SEC, 50);	}
void CDiskInfoDlg::OnWait60Sec(){	CheckRadioWaitTime(ID_WAIT_60_SEC, 60);	}
void CDiskInfoDlg::OnWait90Sec(){	CheckRadioWaitTime(ID_WAIT_90_SEC, 90);	}
void CDiskInfoDlg::OnWait120Sec(){	CheckRadioWaitTime(ID_WAIT_120_SEC, 120);}
void CDiskInfoDlg::OnWait150Sec(){	CheckRadioWaitTime(ID_WAIT_150_SEC, 150);}
void CDiskInfoDlg::OnWait180Sec(){	CheckRadioWaitTime(ID_WAIT_180_SEC, 180);}
void CDiskInfoDlg::OnWait210Sec(){	CheckRadioWaitTime(ID_WAIT_210_SEC, 210);}
void CDiskInfoDlg::OnWait240Sec(){	CheckRadioWaitTime(ID_WAIT_240_SEC, 240);}

void CDiskInfoDlg::CheckRadioWaitTime(int id, int value)
{
	CMenu *menu = GetMenu();
	menu->CheckMenuRadioItem(ID_WAIT_0_SEC, ID_WAIT_240_SEC, id, MF_BYCOMMAND);
	SetMenu(menu);
	DrawMenuBar();

	m_WaitTimeStatus = value;

	CString cstr;
	cstr.Format(_T("%d"), value);
	WritePrivateProfileString(_T("Setting"), _T("StartupWaitTime"), cstr, m_Ini);
}

void CDiskInfoDlg::CheckRadioWaitTime()
{
	int id = ID_WAIT_0_SEC;

	switch(m_WaitTimeStatus)
	{
	case   0: id = ID_WAIT_0_SEC;	break;
	case   5: id = ID_WAIT_5_SEC;	break;
	case  10: id = ID_WAIT_10_SEC;	break;
	case  15: id = ID_WAIT_15_SEC;	break;
	case  20: id = ID_WAIT_20_SEC;	break;
	case  30: id = ID_WAIT_30_SEC;	break;
	case  40: id = ID_WAIT_40_SEC;	break;
	case  50: id = ID_WAIT_50_SEC;	break;
	case  60: id = ID_WAIT_60_SEC;	break;
	case  90: id = ID_WAIT_90_SEC;	break;
	case 120: id = ID_WAIT_120_SEC;	break;
	case 150: id = ID_WAIT_150_SEC;	break;
	case 180: id = ID_WAIT_180_SEC;	break;
	case 210: id = ID_WAIT_210_SEC;	break;
	case 240: id = ID_WAIT_240_SEC;	break;
	default:  id = ID_WAIT_0_SEC;	break;
	}

	CMenu *menu = GetMenu();
	menu->CheckMenuRadioItem(ID_WAIT_0_SEC, ID_WAIT_240_SEC, id, MF_BYCOMMAND);
	SetMenu(menu);
	DrawMenuBar();
}

void CDiskInfoDlg::OnRawValues16()
{
	CheckRadioRawValues(ID_RAW_VALUES_16, 0);
}

void CDiskInfoDlg::OnRawValues10All()
{
	CheckRadioRawValues(ID_RAW_VALUES_10_ALL, 1);
}

void CDiskInfoDlg::OnRawValues2byte()
{
	CheckRadioRawValues(ID_RAW_VALUES_2BYTE, 2);
}

void CDiskInfoDlg::OnRawValues1byte()
{
	CheckRadioRawValues(ID_RAW_VALUES_1BYTE, 3);
}

void CDiskInfoDlg::CheckRadioRawValues(int id, int value)
{
	CMenu *menu = GetMenu();
	menu->CheckMenuRadioItem(ID_RAW_VALUES_16, ID_RAW_VALUES_1BYTE, id, MF_BYCOMMAND);
	SetMenu(menu);
	DrawMenuBar();

	m_RawValues = value;

	CString cstr;
	cstr.Format(_T("%d"), value);
	WritePrivateProfileString(_T("Setting"), _T("RawVlues"), cstr, m_Ini);

	Refresh(TRUE);
}

void CDiskInfoDlg::CheckRadioRawValues()
{
	int id = ID_RAW_VALUES_16;

	switch(m_RawValues)
	{
	case   0: id = ID_RAW_VALUES_16;	break;
	case   1: id = ID_RAW_VALUES_10_ALL;break;
	case   2: id = ID_RAW_VALUES_2BYTE;	break;
	case   3: id = ID_RAW_VALUES_1BYTE;	break;
	default:  id = ID_RAW_VALUES_16;	break;
	}

	CMenu *menu = GetMenu();
	menu->CheckMenuRadioItem(ID_RAW_VALUES_16, ID_RAW_VALUES_1BYTE, id, MF_BYCOMMAND);
	SetMenu(menu);
	DrawMenuBar();
}

void CDiskInfoDlg::OnOpenDiskManagement()
{
	ShellExecute(NULL, NULL, _T("diskmgmt.msc"), NULL, NULL, SW_SHOWNORMAL);	
}

void CDiskInfoDlg::OnOpenDeviceManager()
{
	ShellExecute(NULL, NULL, _T("devmgmt.msc"), NULL, NULL, SW_SHOWNORMAL);	
}

void CDiskInfoDlg::OnAdvancedDiskSearch()
{
	CWaitCursor wait;
	BOOL flagChangeDisk = FALSE;

	if(m_FlagAdvancedDiskSearch)
	{
		m_FlagAdvancedDiskSearch = FALSE;
		InitAta(TRUE, m_FlagAdvancedDiskSearch, &flagChangeDisk);

		if(flagChangeDisk)
		{
			// Update Menu and Dialog
			ChangeLang(m_CurrentLang);
		}
		WritePrivateProfileString(_T("Setting"), _T("AdvancedDiskSearch"), _T("0"), m_Ini);

		CMenu *menu = GetMenu();
		menu->CheckMenuItem(ID_ADVANCED_DISK_SEARCH, MF_UNCHECKED);
		SetMenu(menu);
		DrawMenuBar();
	}
	else
	{
		m_FlagAdvancedDiskSearch = TRUE;
		InitAta(TRUE, m_FlagAdvancedDiskSearch, &flagChangeDisk);

		if(flagChangeDisk)
		{
			// Update Menu and Dialog
			ChangeLang(m_CurrentLang);
		}
		WritePrivateProfileString(_T("Setting"), _T("AdvancedDiskSearch"), _T("1"), m_Ini);

		CMenu *menu = GetMenu();
		menu->CheckMenuItem(ID_ADVANCED_DISK_SEARCH, MF_CHECKED);
		SetMenu(menu);
		DrawMenuBar();
	}

	if(m_FlagResident && flagChangeDisk)
	{
		for(int i = 0; i < CAtaSmart::MAX_DISK; i++)
		{
			RemoveTemperatureIcon(i);
		}
		CheckTrayTemperatureIcon();
	}
}

void CDiskInfoDlg::OnEventLog()
{
	CMenu *menu = GetMenu();
	if(m_FlagEventLog)
	{
		m_FlagEventLog = FALSE;
		menu->CheckMenuItem(ID_EVENT_LOG, MF_UNCHECKED);
		WritePrivateProfileString(_T("Setting"), _T("EventLog"), _T("0"), m_Ini);

	//	if(! m_FlagUseEventCreate)
	//	{
			UninstallEventSource();
	//	}
	}
	else
	{
		m_FlagEventLog = TRUE;
		menu->CheckMenuItem(ID_EVENT_LOG, MF_CHECKED);
		WritePrivateProfileString(_T("Setting"), _T("EventLog"), _T("1"), m_Ini);

	//	if(m_FlagUseEventCreate)
	//	{
			InstallEventSource();
	//	}
	}
	SetMenu(menu);
	DrawMenuBar();
}

void CDiskInfoDlg::CheckStartup()
{
	if(GetPrivateProfileInt(_T("Setting"), _T("Startup"), 0, m_Ini) == 1)
	{
		m_FlagStartup = TRUE;
		CMenu *menu = GetMenu();
		menu->CheckMenuItem(ID_STARTUP, MF_CHECKED);
		SetMenu(menu);
		DrawMenuBar();
	}
	else
	{
		m_FlagStartup = FALSE;
	}
}

void CDiskInfoDlg::OnStartup()
{
	CMenu *menu = GetMenu();
	if(m_FlagStartup)
	{
		UnregisterStartup();
		m_FlagStartup = FALSE;
		menu->CheckMenuItem(ID_STARTUP, MF_UNCHECKED);
		WritePrivateProfileString(_T("Setting"), _T("Startup"), _T("0"), m_Ini);
	}
	else
	{
		RegisterStartup();
		m_FlagStartup = TRUE;
		menu->CheckMenuItem(ID_STARTUP, MF_CHECKED);
		WritePrivateProfileString(_T("Setting"), _T("Startup"), _T("1"), m_Ini);
	}
	SetMenu(menu);
	DrawMenuBar();
}

BOOL CDiskInfoDlg::RegisterStartup()
{
	OSVERSIONINFOEX osvi;
	BOOL bosVersionInfoEx;

	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	if(!(bosVersionInfoEx = GetVersionEx((OSVERSIONINFO *)&osvi)))
	{
		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		GetVersionEx((OSVERSIONINFO *)&osvi);
	}

	TCHAR path[MAX_PATH];
	GetModuleFileName(NULL, path, MAX_PATH);
	CString cstr;

	if(osvi.dwMajorVersion >= 6)
	{
		STARTUPINFO si = {0};
		PROCESS_INFORMATION pi = {0};
		si.cb			= sizeof(STARTUPINFO);
		si.dwFlags		= STARTF_USESHOWWINDOW;
		si.wShowWindow	= SW_HIDE;
		/*
		if(osvi.dwMajorVersion >= 7 || (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion >= 1))
		{
			cstr.Format(_T("schtasks.exe /Create /tn CrystalDiskInfo /tr \"\\\"%s\\\" \"/Startup\"\" /sc ONLOGON /RL HIGHEST /F /ru \"Administrators\""), path);
		}
		else
		{
			cstr.Format(_T("schtasks.exe /Create /tn CrystalDiskInfo /tr \"\\\"%s\\\" \"/Startup\"\" /sc ONLOGON /RL HIGHEST /F"), path);
		}
		*/
		cstr.Format(_T("schtasks.exe /Create /tn CrystalDiskInfo /tr \"\\\"%s\\\" \"/Startup\"\" /sc ONLOGON /RL HIGHEST /F"), path);
		::CreateProcess(NULL, (LPWSTR)cstr.GetString(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
		WaitForSingleObject(pi.hProcess, 1000);
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
	}
	else
	{
		HKEY hKey;
		DWORD disposition;
		LONG result;
		result = ::RegCreateKeyEx(HKEY_CURRENT_USER, 
			_T("Software\\Microsoft\\Windows\\CurrentVersion\\Run"), 0, _T(""),
			REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &disposition);
		if(result == ERROR_SUCCESS)
		{
			cstr.Format(_T("\"%s\" /Startup"), path);
			::RegSetValueEx(hKey, _T("CrystalDiskInfo"), 0, REG_SZ,
				(CONST BYTE*)(LPCTSTR)cstr,
				((DWORD)_tcslen(cstr) + 3) * 2);
			::RegCloseKey(hKey);
		}
	}

	return TRUE;
}

BOOL CDiskInfoDlg::UnregisterStartup()
{
	OSVERSIONINFOEX osvi;
	BOOL bosVersionInfoEx;

	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	if(!(bosVersionInfoEx = GetVersionEx((OSVERSIONINFO *)&osvi)))
	{
		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		GetVersionEx((OSVERSIONINFO *)&osvi);
	}

	if(osvi.dwMajorVersion >= 6)
	{
		CString cstr;

		STARTUPINFO si = {0};
		PROCESS_INFORMATION pi = {0};
		si.cb			= sizeof(STARTUPINFO);
		si.dwFlags		= STARTF_USESHOWWINDOW;
		si.wShowWindow	= SW_HIDE;
		cstr.Format(_T("schtasks.exe /Delete /tn CrystalDiskInfo /F")); 
		::CreateProcess(NULL, (LPWSTR)cstr.GetString(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
		WaitForSingleObject(pi.hProcess, 1000);
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
	}
	else
	{
		HKEY hKey;
		DWORD disposition;
		LONG result;
		result = ::RegCreateKeyEx(HKEY_CURRENT_USER, 
			_T("Software\\Microsoft\\Windows\\CurrentVersion\\Run"), 0, _T(""),
			REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &disposition);
		if(result == ERROR_SUCCESS)
		{
			::RegDeleteValue(hKey, _T("CrystalDiskInfo"));
			::RegCloseKey(hKey);
		}
	}

	return TRUE;
}

void CDiskInfoDlg::OnCelsius()
{
	CMenu *menu = GetMenu();
	menu->CheckMenuRadioItem(ID_CELSIUS, ID_FAHRENHEIT, ID_CELSIUS, MF_BYCOMMAND);
	SetMenu(menu);
	DrawMenuBar();
	m_FlagFahrenheit = FALSE;
	WritePrivateProfileString(_T("Setting"), _T("Temperature"), _T("0"), m_Ini);

	SelectDrive(m_SelectDisk);
}

void CDiskInfoDlg::OnFahrenheit()
{
	CMenu *menu = GetMenu();
	menu->CheckMenuRadioItem(ID_CELSIUS, ID_FAHRENHEIT, ID_FAHRENHEIT, MF_BYCOMMAND);
	SetMenu(menu);
	DrawMenuBar();
	m_FlagFahrenheit = TRUE;
	WritePrivateProfileString(_T("Setting"), _T("Temperature"), _T("1"), m_Ini);

	SelectDrive(m_SelectDisk);
}

void CDiskInfoDlg::OnAutoAamApm()
{
	CMenu *menu = GetMenu();
	if(m_FlagAutoAamApm)
	{
		m_FlagAutoAamApm = FALSE;
		menu->CheckMenuItem(ID_AUTO_AAM_APM, MF_UNCHECKED);
		WritePrivateProfileString(_T("Setting"), _T("AutoAamApm"), _T("0"), m_Ini);
	}
	else
	{
		m_FlagAutoAamApm = TRUE;
		menu->CheckMenuItem(ID_AUTO_AAM_APM, MF_CHECKED);
		WritePrivateProfileString(_T("Setting"), _T("AutoAamApm"), _T("1"), m_Ini);
	}
	SetMenu(menu);
	DrawMenuBar();
}

void CDiskInfoDlg::OnUsbSat()
{
	CMenu *menu = GetMenu();
	if(m_Ata.FlagUsbSat)
	{
		m_Ata.FlagUsbSat = FALSE;
		menu->CheckMenuItem(ID_USB_SAT, MF_UNCHECKED);
		WritePrivateProfileString(_T("USB"), _T("SAT"), _T("0"), m_Ini);
	}
	else
	{
		m_Ata.FlagUsbSat = TRUE;
		menu->CheckMenuItem(ID_USB_SAT, MF_CHECKED);
		WritePrivateProfileString(_T("USB"), _T("SAT"), _T("1"), m_Ini);
	}
	SetMenu(menu);
	DrawMenuBar();
}

void CDiskInfoDlg::OnUsbIodata()
{
	CMenu *menu = GetMenu();
	if(m_Ata.FlagUsbIodata)
	{
		m_Ata.FlagUsbIodata = FALSE;
		menu->CheckMenuItem(ID_USB_IODATA, MF_UNCHECKED);
		WritePrivateProfileString(_T("USB"), _T("IODATA"), _T("0"), m_Ini);
	}
	else
	{
		m_Ata.FlagUsbIodata = TRUE;
		menu->CheckMenuItem(ID_USB_IODATA, MF_CHECKED);
		WritePrivateProfileString(_T("USB"), _T("IODATA"), _T("1"), m_Ini);
	}
	SetMenu(menu);
	DrawMenuBar();
}

void CDiskInfoDlg::OnUsbSunplus()
{
	CMenu *menu = GetMenu();
	if(m_Ata.FlagUsbSunplus)
	{
		m_Ata.FlagUsbSunplus = FALSE;
		menu->CheckMenuItem(ID_USB_SUNPLUS, MF_UNCHECKED);
		WritePrivateProfileString(_T("USB"), _T("Sunplus"), _T("0"), m_Ini);
	}
	else
	{
		m_Ata.FlagUsbSunplus = TRUE;
		menu->CheckMenuItem(ID_USB_SUNPLUS, MF_CHECKED);
		WritePrivateProfileString(_T("USB"), _T("Sunplus"), _T("1"), m_Ini);
	}
	SetMenu(menu);
	DrawMenuBar();
}

void CDiskInfoDlg::OnUsbLogitec()
{
	CMenu *menu = GetMenu();
	if(m_Ata.FlagUsbLogitec)
	{
		m_Ata.FlagUsbLogitec = FALSE;
		menu->CheckMenuItem(ID_USB_LOGITEC, MF_UNCHECKED);
		WritePrivateProfileString(_T("USB"), _T("Logitec"), _T("0"), m_Ini);
	}
	else
	{
		m_Ata.FlagUsbLogitec = TRUE;
		menu->CheckMenuItem(ID_USB_LOGITEC, MF_CHECKED);
		WritePrivateProfileString(_T("USB"), _T("Logitec"), _T("1"), m_Ini);
	}
	SetMenu(menu);
	DrawMenuBar();
}

void CDiskInfoDlg::OnUsbJmicron()
{
		CMenu *menu = GetMenu();
	if(m_Ata.FlagUsbJmicron)
	{
		m_Ata.FlagUsbJmicron = FALSE;
		menu->CheckMenuItem(ID_USB_JMICRON, MF_UNCHECKED);
		WritePrivateProfileString(_T("USB"), _T("JMicron"), _T("0"), m_Ini);
	}
	else
	{
		m_Ata.FlagUsbJmicron = TRUE;
		menu->CheckMenuItem(ID_USB_JMICRON, MF_CHECKED);
		WritePrivateProfileString(_T("USB"), _T("JMicron"), _T("1"), m_Ini);
	}
	SetMenu(menu);
	DrawMenuBar();
}

void CDiskInfoDlg::OnUsbCypress()
{
	CMenu *menu = GetMenu();
	if(m_Ata.FlagUsbCypress)
	{
		m_Ata.FlagUsbCypress = FALSE;
		menu->CheckMenuItem(ID_USB_CYPRESS, MF_UNCHECKED);
		WritePrivateProfileString(_T("USB"), _T("Cypress"), _T("0"), m_Ini);
	}
	else
	{
		m_Ata.FlagUsbCypress = TRUE;
		menu->CheckMenuItem(ID_USB_CYPRESS, MF_CHECKED);
		WritePrivateProfileString(_T("USB"), _T("Cypress"), _T("1"), m_Ini);
	}
	SetMenu(menu);
	DrawMenuBar();
}

void CDiskInfoDlg::OnUsbEnableAll()
{
	m_Ata.FlagUsbSat     = FALSE;
	m_Ata.FlagUsbIodata  = FALSE;
	m_Ata.FlagUsbSunplus = FALSE;
	m_Ata.FlagUsbLogitec = FALSE;
	m_Ata.FlagUsbJmicron = FALSE;
	m_Ata.FlagUsbCypress = FALSE;

	OnUsbSat();
	OnUsbIodata();
	OnUsbSunplus();
	OnUsbLogitec();
	OnUsbJmicron();
	OnUsbCypress();
}

void CDiskInfoDlg::OnUsbDisableAll()
{
	m_Ata.FlagUsbSat     = TRUE;
	m_Ata.FlagUsbIodata  = TRUE;
	m_Ata.FlagUsbSunplus = TRUE;
	m_Ata.FlagUsbLogitec = TRUE;
	m_Ata.FlagUsbJmicron = TRUE;
	m_Ata.FlagUsbCypress = TRUE;

	OnUsbSat();
	OnUsbIodata();
	OnUsbSunplus();
	OnUsbLogitec();
	OnUsbJmicron();
	OnUsbCypress();
}

void CDiskInfoDlg::OnDumpIdentifyDevice()
{
	CMenu *menu = GetMenu();
	if(m_FlagDumpIdentifyDevice)
	{
		m_FlagDumpIdentifyDevice = FALSE;
		menu->CheckMenuItem(ID_DUMP_IDENTIFY_DEVICE, MF_UNCHECKED);
		WritePrivateProfileString(_T("Setting"), _T("DumpIdentifyDevice"), _T("0"), m_Ini);
	}
	else
	{
		m_FlagDumpIdentifyDevice = TRUE;
		menu->CheckMenuItem(ID_DUMP_IDENTIFY_DEVICE, MF_CHECKED);
		WritePrivateProfileString(_T("Setting"), _T("DumpIdentifyDevice"), _T("1"), m_Ini);
	}
	SetMenu(menu);
	DrawMenuBar();
}

void CDiskInfoDlg::OnDumpSmartReadData()
{
	CMenu *menu = GetMenu();
	if(m_FlagDumpSmartReadData)
	{
		m_FlagDumpSmartReadData = FALSE;
		menu->CheckMenuItem(ID_DUMP_SMART_READ_DATA, MF_UNCHECKED);
		WritePrivateProfileString(_T("Setting"), _T("DumpSmartReadData"), _T("0"), m_Ini);
	}
	else
	{
		m_FlagDumpSmartReadData = TRUE;
		menu->CheckMenuItem(ID_DUMP_SMART_READ_DATA, MF_CHECKED);
		WritePrivateProfileString(_T("Setting"), _T("DumpSmartReadData"), _T("1"), m_Ini);
	}
	SetMenu(menu);
	DrawMenuBar();
}

void CDiskInfoDlg::OnDumpSmartReadThreshold()
{
	CMenu *menu = GetMenu();
	if(m_FlagDumpSmartReadThreshold)
	{
		m_FlagDumpSmartReadThreshold = FALSE;
		menu->CheckMenuItem(ID_DUMP_SMART_READ_THRESHOLD, MF_UNCHECKED);
		WritePrivateProfileString(_T("Setting"), _T("DumpSmartReadThreshold"), _T("0"), m_Ini);
	}
	else
	{
		m_FlagDumpSmartReadThreshold = TRUE;
		menu->CheckMenuItem(ID_DUMP_SMART_READ_THRESHOLD, MF_CHECKED);
		WritePrivateProfileString(_T("Setting"), _T("DumpSmartReadThreshold"), _T("1"), m_Ini);
	}
	SetMenu(menu);
	DrawMenuBar();
}

void CDiskInfoDlg::OnResidentHide()
{
	CMenu *menu = GetMenu();
	menu->CheckMenuRadioItem(ID_RESIDENT_HIDE, ID_RESIDENT_MINIMIZE, ID_RESIDENT_HIDE, MF_BYCOMMAND);
	SetMenu(menu);
	DrawMenuBar();
	m_FlagResidentMinimize = FALSE;
	WritePrivateProfileString(_T("Setting"), _T("ResidentMinimize"), _T("0"), m_Ini);
}

void CDiskInfoDlg::OnResidentMinimize()
{
	CMenu *menu = GetMenu();
	menu->CheckMenuRadioItem(ID_RESIDENT_HIDE, ID_RESIDENT_MINIMIZE, ID_RESIDENT_MINIMIZE, MF_BYCOMMAND);
	SetMenu(menu);
	DrawMenuBar();
	m_FlagResidentMinimize = TRUE;
	WritePrivateProfileString(_T("Setting"), _T("ResidentMinimize"), _T("1"), m_Ini);
}


void CDiskInfoDlg::OnZoom100()
{
	if(CheckRadioZoomType(ID_ZOOM_100, 100))
	{
		ReExecute();
	}
}

void CDiskInfoDlg::OnZoom125()
{
	if(CheckRadioZoomType(ID_ZOOM_125, 125))
	{
		ReExecute();
	}
}

void CDiskInfoDlg::OnZoom150()
{
	if(CheckRadioZoomType(ID_ZOOM_150, 150))
	{
		ReExecute();
	}
}

void CDiskInfoDlg::OnZoom200()
{
	if(CheckRadioZoomType(ID_ZOOM_200, 200))
	{
		ReExecute();
	}
}

void CDiskInfoDlg::OnZoomAuto()
{
	if(CheckRadioZoomType(ID_ZOOM_AUTO, 0))
	{
		ReExecute();
	}
}

BOOL CDiskInfoDlg::CheckRadioZoomType(int id, int value)
{
	if(m_ZoomType == value)
	{
		return FALSE;
	}

	CMenu *menu = GetMenu();
	menu->CheckMenuRadioItem(ID_ZOOM_100, ID_ZOOM_AUTO, id, MF_BYCOMMAND);
	SetMenu(menu);
	DrawMenuBar();

	m_ZoomType = value;

	CString cstr;
	cstr.Format(_T("%d"), value);
	WritePrivateProfileString(_T("Setting"), _T("ZoomType"), cstr, m_Ini);

	return TRUE;
}

void CDiskInfoDlg::CheckRadioZoomType()
{
	int id = ID_ZOOM_AUTO;

	switch(m_ZoomType)
	{
	case 100: id = ID_ZOOM_100;	break;
	case 125: id = ID_ZOOM_125;	break;
	case 150: id = ID_ZOOM_150;	break;
	case 200: id = ID_ZOOM_200;	break;
	default:  id = ID_ZOOM_AUTO;	break;
	}

	CMenu *menu = GetMenu();
	menu->CheckMenuRadioItem(ID_ZOOM_100, ID_ZOOM_AUTO, id, MF_BYCOMMAND);
	SetMenu(menu);
	DrawMenuBar();
}
