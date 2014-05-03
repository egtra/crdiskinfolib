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



#ifdef _DEBUG
#define new DEBUG_NEW
#endif

extern const GUID StrageGUID;

int CALLBACK EnumFontFamExProcMeiryo(ENUMLOGFONTEX *lpelfe, NEWTEXTMETRICEX *lpntme, int FontType, LPARAM lParam)
{
	if(_tcscmp(lpelfe->elfLogFont.lfFaceName, _T("メイリオ")) == 0)
	{
		BOOL *flag = (BOOL*)lParam;
		*flag = TRUE;
	}
    return TRUE;
}

BOOL CDiskInfoDlg::OnInitDialog()
{
	CDHtmlMainDialog::OnInitDialog();

	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIconMini, FALSE);		// Set small icon

	InitThemeLang();
	InitMenu();

	TCHAR str[256];
	
	CClientDC dc(this);
    LOGFONT logfont;
	CString defaultFontFace;
	BOOL hasMeiryo = FALSE;
    ZeroMemory(&logfont, sizeof(LOGFONT)); 
    logfont.lfCharSet = ANSI_CHARSET;
    ::EnumFontFamiliesEx(dc.m_hDC, &logfont, (FONTENUMPROC)EnumFontFamExProcMeiryo, (long)&hasMeiryo, 0);
	
	if(hasMeiryo)
	{
		defaultFontFace = _T("メイリオ");
	}
	else
	{
		defaultFontFace = _T("Tahoma");
	}

	GetPrivateProfileString(_T("Setting"), _T("FontFace"), defaultFontFace, str, 256, m_Ini);
	m_FontFace = str;

	switch(GetPrivateProfileInt(_T("Setting"), _T("AutoRefresh"), 10, m_Ini))
	{
	case  1: OnAutoRefresh01Min(); break;
	case  3: OnAutoRefresh03Min(); break;
	case  5: OnAutoRefresh05Min(); break;
	case 10: OnAutoRefresh10Min(); break;
	case 30: OnAutoRefresh30Min(); break;
	case 60: OnAutoRefresh60Min(); break;
	default: OnAutoRefreshDisable(); break;
	}

	switch(GetPrivateProfileInt(_T("Setting"), _T("StartupWaitTime"), 30, m_Ini))
	{
	case   0: OnWait0Sec();   break;
	case   5: OnWait5Sec();   break;
	case  10: OnWait10Sec();  break;
	case  15: OnWait15Sec();  break;
	case  20: OnWait20Sec();  break;
	case  30: OnWait30Sec();  break;
	case  40: OnWait40Sec();  break;
	case  50: OnWait50Sec();  break;
	case  60: OnWait60Sec();  break;
	case  90: OnWait90Sec();  break;
	case 120: OnWait120Sec(); break;
	case 150: OnWait150Sec(); break;
	case 180: OnWait180Sec(); break;
	case 210: OnWait210Sec(); break;
	case 240: OnWait240Sec(); break;
	default:  OnWait0Sec();   break;
	}

	switch(GetPrivateProfileInt(_T("Setting"), _T("ZoomType"), 0, m_Ini))
	{
	case 100:  CheckRadioZoomType(ID_ZOOM_100, 100); break;
	case 125:  CheckRadioZoomType(ID_ZOOM_125, 125); break;
	case 150:  CheckRadioZoomType(ID_ZOOM_150, 150); break;
	case 200:  CheckRadioZoomType(ID_ZOOM_200, 200); break;
	default:   CheckRadioZoomType(ID_ZOOM_AUTO, 0); break;
	}

	switch(GetPrivateProfileInt(_T("Setting"), _T("Temperature"), 0, m_Ini))
	{
	case   1:	OnFahrenheit(); break;
	default:	OnCelsius();	break;
	}

	switch(GetPrivateProfileInt(_T("Setting"), _T("ResidentMinimize"), 0, m_Ini))
	{
	case   1:	OnResidentMinimize(); break;
	default:	OnResidentHide();	  break;
	}

	// USB/IEEE1394 Support
	m_Ata.FlagUsbSat     = ! GetPrivateProfileInt(_T("USB"), _T("SAT"), 1, m_Ini);
	m_Ata.FlagUsbIodata  = ! GetPrivateProfileInt(_T("USB"), _T("IODATA"), 1, m_Ini);
	m_Ata.FlagUsbSunplus = ! GetPrivateProfileInt(_T("USB"), _T("Sunplus"), 1, m_Ini);
	m_Ata.FlagUsbLogitec = ! GetPrivateProfileInt(_T("USB"), _T("Logitec"), 1, m_Ini);
	m_Ata.FlagUsbJmicron = ! GetPrivateProfileInt(_T("USB"), _T("JMicron"), 1, m_Ini);
	m_Ata.FlagUsbCypress = ! GetPrivateProfileInt(_T("USB"), _T("Cypress"), 1, m_Ini);

	OnUsbSat();
	OnUsbIodata();
	OnUsbSunplus();
	OnUsbLogitec();
	OnUsbJmicron();
	OnUsbCypress();

	InitAta((BOOL)GetPrivateProfileInt(_T("Setting"), _T("UseWMI"), 1, m_Ini), m_FlagAdvancedDiskSearch, NULL, m_FlagWorkaroundHD204UI, m_FlagWorkaroundAdataSsd);

	if(m_FlagStartupExit)
	{
		EndDialog(0);
		return FALSE;
	}

	m_SizeX = SIZE_X;
	m_SizeY = SIZE_Y;

	EnableDpiAware();
	InitDHtmlDialog(m_SizeX, m_SizeY, ((CDiskInfoApp*)AfxGetApp())->m_MainDlgPath);
	//	SetWindowTitle(_T("Initializing..."));

	DEV_BROADCAST_DEVICEINTERFACE filter;
	filter.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
	filter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
	filter.dbcc_classguid = StrageGUID;
	m_hDevNotify = RegisterDeviceNotification(m_hWnd, &filter, DEVICE_NOTIFY_WINDOW_HANDLE);

	return TRUE; 
}

#ifdef SUISHO_SHIZUKU_SUPPORT
void CDiskInfoDlg::ChangeShizukuImage(DWORD index)
{
	CString arg, cstr;
	TCHAR path[MAX_PATH];
	TCHAR exe[_MAX_FNAME];
	
	GetModuleFileName(NULL, path, sizeof(path));
	_wsplitpath_s(path, NULL, 0, NULL, 0, exe, _MAX_FNAME, NULL, 0);

	if(index == 0 || MAX_SHIZUKU_IMAGE < index)
	{
		srand(GetTickCount());
		index = rand() % MAX_SHIZUKU_IMAGE + 1;
	}

	arg.Format(_T("res://%s.exe/#2110/%d"), exe, IDR_SHIZUKU_0 + index);
	CallScript(_T("setShizuku"), arg);
	arg.Format(_T("res://%s.exe/#2110/%d"), exe, IDR_SHIZUKU_COPYRIGHT);
	CallScript(_T("setShizukuCopyright"), arg);

	cstr.Format(_T("%d"), index);
	WritePrivateProfileString(_T("Setting"), _T("ShizukuImageType"), cstr, m_Ini);
}
#endif

void CDiskInfoDlg::OnDocumentComplete(LPDISPATCH pDisp, LPCTSTR szUrl)
{
	CString cstr;
	cstr = szUrl;
	static BOOL once = FALSE;
	if(cstr.Find(_T("html")) != -1 || cstr.Find(_T("dlg")) != -1)
	{
		if(! once)
		{
			CheckStartup();
			// CheckResident();
			CheckHideSerialNumber();
			ChangeTheme(m_CurrentTheme);
			if(m_CurrentTheme.Compare(_T("Simplicity")) == 0)
			{
				m_FlagGoodGreen = TRUE;
			}
			else
			{
				m_FlagGoodGreen = FALSE;
			}
			ChangeZoomType(m_ZoomType);
			UpdateDialogSize();
			ChangeLang(m_CurrentLang);
			CheckPage();
			
			m_FlagShowWindow = TRUE;		
			CenterWindow();

#ifdef SUISHO_SHIZUKU_SUPPORT
			ChangeShizukuImage(m_ShizukuImageType);
#endif
			if(m_FlagResident)
			{
				AlarmOverheat();
			//	CheckTrayTemperatureIcon();
				if(! m_FlagResidentMinimize)
				{
					m_FlagShowWindow = FALSE;
				}
			}
			else
			{
				ShowWindow(SW_SHOW);
			}

			if(m_NowDetectingUnitPowerOnHours != TRUE)
			{
				SetWindowTitle(_T(""));
			}

			once = TRUE;
			m_FlagInitializing = FALSE;

			// Font Settings
			CallScript(_T("setFont"), m_FontFace);
		}
	}
}

void CDiskInfoDlg::InitAta(BOOL useWmi, BOOL advancedDiskSearch, PBOOL flagChangeDisk, BOOL workaroundHD204UI, BOOL workaroundAdataSsd)
{
	static BOOL once = FALSE;

	KillTimer(TIMER_SET_POWER_ON_UNIT);
	SetWindowTitle(i18n(_T("Message"), _T("DETECT_DISK")));
	m_PowerOnHoursClass = _T("valueR");
	m_NowDetectingUnitPowerOnHours = FALSE;

	m_Ata.Init(useWmi, advancedDiskSearch, flagChangeDisk, workaroundHD204UI, workaroundAdataSsd);
	
	if(! once)
	{
		CheckResident();
		once = TRUE;
	}

	DWORD errorCount = 0;
	for(int i = 0; i < m_Ata.vars.GetCount(); i++)
	{
		int unitType = GetPrivateProfileInt(_T("PowerOnUnit"), m_Ata.vars[i].ModelSerial, -1, m_Ini);
		if(unitType >= 0)
		{
			if(m_Ata.vars[i].DetectedTimeUnitType == m_Ata.POWER_ON_MILLI_SECONDS)
			{
				unitType = m_Ata.POWER_ON_MILLI_SECONDS;
			}
			else if(m_Ata.vars[i].DetectedTimeUnitType == m_Ata.POWER_ON_10_MINUTES)
			{
				unitType = m_Ata.POWER_ON_10_MINUTES;
			}
			m_Ata.vars[i].MeasuredTimeUnitType = unitType;
			m_Ata.vars[i].MeasuredPowerOnHours = m_Ata.GetPowerOnHoursEx(i, unitType);
		}
		else if(m_Ata.vars[i].PowerOnRawValue > 0)
		{
			errorCount++;
		}

		m_FlagAutoRefreshTarget[i] = GetPrivateProfileInt(_T("AutoRefreshTarget"), m_Ata.vars[i].ModelSerial, 1, m_Ini);;
		m_Ata.vars[i].AlarmTemperature = GetPrivateProfileInt(_T("AlarmTemperature"), m_Ata.vars[i].ModelSerial, 50, m_Ini);
		m_Ata.vars[i].AlarmHealthStatus = GetPrivateProfileInt(_T("AlarmHealthStatus"), m_Ata.vars[i].ModelSerial, 1, m_Ini);

		m_Ata.vars[i].Threshold05     = GetPrivateProfileInt(_T("ThreasholdOfCaution05"), m_Ata.vars[i].ModelSerial, 1, m_Ini);
		m_Ata.vars[i].ThresholdC5     = GetPrivateProfileInt(_T("ThreasholdOfCautionC5"), m_Ata.vars[i].ModelSerial, 1, m_Ini);
		m_Ata.vars[i].ThresholdC6     = GetPrivateProfileInt(_T("ThreasholdOfCautionC6"), m_Ata.vars[i].ModelSerial, 1, m_Ini);

		m_Ata.vars[i].DiskStatus = m_Ata.CheckDiskStatus(i);
		SaveSmartInfo(i);

#ifdef BENCHMARK
		// Benchmark Reuslt
		TCHAR str[256];
		GetPrivateProfileString(_T("Benchmark"), m_Ata.vars[i].ModelSerial, _T("0.0"), str, 256, m_Ini);
		m_Ata.vars[i].Speed = Decode10X(str) / 1000.0;
#endif
	}
#ifdef ALERT_VOICE_SUPPORT
	AlertSound(0, AS_PLAY_SOUND);
#endif	
	if(errorCount)
	{
		SetTimer(TIMER_SET_POWER_ON_UNIT, 130000, 0);
	//	SetWindowTitle(i18n(_T("Message"), _T("DETECT_UNIT_POWER_ON_HOURS")));
	//	m_PowerOnHoursClass = _T("valueR gray");
		m_PowerOnHoursClass = _T("valueR");
		m_NowDetectingUnitPowerOnHours = TRUE;
	}
	SetWindowTitle(_T(""));

	AutoAamApmAdaption();

	#ifdef GADGET_SUPPORT
	UpdateShareInfo();
	#endif
}

void CDiskInfoDlg::InitListCtrl()
{
	DWORD style = ListView_GetExtendedListViewStyle(m_List.m_hWnd);
	style |= LVS_EX_FULLROWSELECT | /*LVS_EX_GRIDLINES |*/ LVS_EX_LABELTIP ;
	ListView_SetExtendedListViewStyle(m_List.m_hWnd, style);

	m_List.SetImageList(&m_ImageList, LVSIL_SMALL);
}

CString CDiskInfoDlg::GetDiskStatus(DWORD statusCode)
{
	switch(statusCode)
	{
	case CAtaSmart::DISK_STATUS_GOOD:
		return i18n(_T("DiskStatus"), _T("GOOD"));
		break;
	case CAtaSmart::DISK_STATUS_CAUTION:
		return i18n(_T("DiskStatus"), _T("CAUTION"));
		break;
	case CAtaSmart::DISK_STATUS_BAD:
		return i18n(_T("DiskStatus"), _T("BAD"));
		break;
	case CAtaSmart::DISK_STATUS_UNKNOWN:
	default:
		return i18n(_T("DiskStatus"), _T("UNKNOWN"));
		break;
	}
}

CString CDiskInfoDlg::GetDiskStatusClass(DWORD statusCode)
{
	switch(statusCode)
	{
	case CAtaSmart::DISK_STATUS_GOOD:
		return _T("diskStatusGood");
		break;
	case CAtaSmart::DISK_STATUS_CAUTION:
		return _T("diskStatusCaution");
		break;
	case CAtaSmart::DISK_STATUS_BAD:
		return _T("diskStatusBad");
		break;
	case CAtaSmart::DISK_STATUS_UNKNOWN:
	default:
		return _T("diskStatusUnknown");
		break;
	}
}

CString CDiskInfoDlg::GetTemperatureClass(DWORD temperature)
{
	if(temperature >= 55)
	{
		return _T("temperatureBad");
	}
	else if(temperature >= 50)
	{
		return _T("temperatureCaution");
	}
	else if(temperature <= 0)
	{
		return _T("temperatureUnknown");
	}
	else
	{
		return _T("temperatureGood");
	}
}

CString CDiskInfoDlg::GetDiskStatusReason(DWORD index)
{
	CString result, cstr;
	DWORD value = 0;
	
	if(m_Ata.vars[index].DiskStatus == CAtaSmart::DISK_STATUS_BAD)
	{
		if(m_Ata.vars[index].IsSsd && m_Ata.vars[index].IsRawValues8)
		{
		}
		else if(m_Ata.vars[index].IsSsd)
		{
			for(DWORD j = 0; j < m_Ata.vars[index].AttributeCount; j++)
			{
				if( m_Ata.vars[index].Threshold[j].ThresholdValue != 0
				&& 	m_Ata.vars[index].Attribute[j].CurrentValue < m_Ata.vars[index].Threshold[j].ThresholdValue)
				{
					cstr.Format(_T("%02X"), m_Ata.vars[index].Attribute[j].Id);
					result += i18n(_T("DiskStatus"), _T("BAD")) + _T(" (") + cstr + _T(") ")+ i18n(m_Ata.vars[index].SmartKeyName, cstr);
					cstr.Format(_T("\n"));
					result += cstr;
				}
			}
		}
		else
		{
			for(DWORD j = 0; j < m_Ata.vars[index].AttributeCount; j++)
			{
				if(((0x01 <= m_Ata.vars[index].Attribute[j].Id && m_Ata.vars[index].Attribute[j].Id <= 0x0D)
				||	m_Ata.vars[index].Attribute[j].Id == 0xB8
				||	(0xBB <= m_Ata.vars[index].Attribute[j].Id && m_Ata.vars[index].Attribute[j].Id <= 0xC1)
				||	(0xC3 <= m_Ata.vars[index].Attribute[j].Id && m_Ata.vars[index].Attribute[j].Id <= 0xD1)
				||	(0xD3 <= m_Ata.vars[index].Attribute[j].Id && m_Ata.vars[index].Attribute[j].Id <= 0xD4)
				||	(0xDC <= m_Ata.vars[index].Attribute[j].Id && m_Ata.vars[index].Attribute[j].Id <= 0xE4)
				||	(0xE6 <= m_Ata.vars[index].Attribute[j].Id && m_Ata.vars[index].Attribute[j].Id <= 0xE7)
				||	m_Ata.vars[index].Attribute[j].Id == 0xF0
				||	m_Ata.vars[index].Attribute[j].Id == 0xFA
				||	m_Ata.vars[index].Attribute[j].Id == 0xFE
				)
				&&	m_Ata.vars[index].Threshold[j].ThresholdValue != 0
				&& 	m_Ata.vars[index].Attribute[j].CurrentValue < m_Ata.vars[index].Threshold[j].ThresholdValue)
				{
					cstr.Format(_T("%02X"), m_Ata.vars[index].Attribute[j].Id);
					result += i18n(_T("DiskStatus"), _T("BAD")) + _T(" (") + cstr + _T(") ")+ i18n(m_Ata.vars[index].SmartKeyName, cstr);
					cstr.Format(_T("\n"));
					result += cstr;
				}
			}
		}
	}

	if(m_Ata.vars[index].DiskStatus == CAtaSmart::DISK_STATUS_CAUTION
	|| m_Ata.vars[index].DiskStatus == CAtaSmart::DISK_STATUS_BAD)
	{
		for(DWORD j = 0; j < m_Ata.vars[index].AttributeCount; j++)
		{
			if(m_Ata.vars[index].Attribute[j].Id == 0x05 // Reallocated Sectors Count
			|| m_Ata.vars[index].Attribute[j].Id == 0xC5 // Current Pending Sector Count
			|| m_Ata.vars[index].Attribute[j].Id == 0xC6 // Off-Line Scan Uncorrectable Sector Count
			)
			{
				value = MAKELONG(
							MAKEWORD(
								m_Ata.vars[index].Attribute[j].RawValue[0],
								m_Ata.vars[index].Attribute[j].RawValue[1]),
							MAKEWORD(
								m_Ata.vars[index].Attribute[j].RawValue[2],
								m_Ata.vars[index].Attribute[j].RawValue[3])
							);
				if(value != 0xFFFFFFFF && value != 0x0 && ! m_Ata.vars[index].IsSsd)
				{
					cstr.Format(_T("%02X"), m_Ata.vars[index].Attribute[j].Id);
					result += i18n(_T("DiskStatus"), _T("CAUTION")) + _T(" [") + cstr + _T("] ")+ i18n(_T("Smart"), cstr);
					cstr.Format(_T(" : %d\n"), value);
					result += cstr;
				}
			}
			else
			if((m_Ata.vars[index].Attribute[j].Id == 0xE8 && (m_Ata.vars[index].DiskVendorId == m_Ata.SSD_VENDOR_INTEL || m_Ata.vars[index].DiskVendorId == m_Ata.SSD_VENDOR_PLEXTOR))
			|| (m_Ata.vars[index].Attribute[j].Id == 0xBB && m_Ata.vars[index].DiskVendorId == m_Ata.SSD_VENDOR_MTRON)
			||((m_Ata.vars[index].Attribute[j].Id == 0xB4 || m_Ata.vars[index].Attribute[j].Id == 0xB3) && m_Ata.vars[index].DiskVendorId == m_Ata.SSD_VENDOR_SAMSUNG)
			|| (m_Ata.vars[index].Attribute[j].Id == 0xD1 && m_Ata.vars[index].DiskVendorId == m_Ata.SSD_VENDOR_INDILINX)
			|| (m_Ata.vars[index].Attribute[j].Id == 0xE7 && m_Ata.vars[index].DiskVendorId == m_Ata.SSD_VENDOR_SANDFORCE)
			|| (m_Ata.vars[index].Attribute[j].Id == 0xAA && m_Ata.vars[index].DiskVendorId == m_Ata.SSD_VENDOR_JMICRON && ! m_Ata.vars[index].IsRawValues8
			|| (m_Ata.vars[index].Attribute[j].Id == 0xCA && m_Ata.vars[index].DiskVendorId == m_Ata.SSD_VENDOR_MICRON)
			|| (m_Ata.vars[index].Attribute[j].Id == 0xE9 && m_Ata.vars[index].DiskVendorId == m_Ata.SSD_VENDOR_OCZ)
			)
			)
			{
				cstr.Format(_T("%02X"), m_Ata.vars[index].Attribute[j].Id);
				if(m_Ata.vars[index].DiskStatus == CAtaSmart::DISK_STATUS_CAUTION)
				{
					result += i18n(_T("DiskStatus"), _T("CAUTION")) + _T(" [") + cstr + _T("] ") + i18n(m_Ata.vars[index].SmartKeyName, cstr);
				}
				else if(m_Ata.vars[index].DiskStatus == CAtaSmart::DISK_STATUS_BAD)
				{
					result += i18n(_T("DiskStatus"), _T("BAD")) + _T(" [") + cstr + _T("] ") + i18n(m_Ata.vars[index].SmartKeyName, cstr);
				}
				cstr.Format(_T(" : %d\n"), m_Ata.vars[index].Life);
				result += cstr;
			}
		}
	}

	result.TrimRight();
	return result;
}

CString CDiskInfoDlg::GetLogicalDriveInfo(DWORD index, INT maxLength)
{
	DWORD map = m_Ata.vars[index].DriveLetterMap;
	CString resultS, resultM, resultL;
	ULARGE_INTEGER freeBytesAvailableToCaller;
	ULARGE_INTEGER totalNumberOfBytes;
	ULARGE_INTEGER totalNumberOfFreeBytes;

	for(int j = 0; j < 26; j++)
	{
		if(map & (1 << j))
		{
			CString cstr;
			CString letter;
			TCHAR volumeNameBuffer[256];

			letter.Format(_T("%C:\\"), j + 'A');
			GetDiskFreeSpaceEx(letter, &freeBytesAvailableToCaller,
				&totalNumberOfBytes, &totalNumberOfFreeBytes);
			GetVolumeInformation(letter,  volumeNameBuffer, 256, NULL, NULL, NULL, NULL, 0);

			cstr.Format(_T("%C: %s [%.1f/%.1f GB (%.1f %%)]\r\n"), 
				j + 'A', volumeNameBuffer,
				totalNumberOfFreeBytes.QuadPart / 1024 / 1024 / 1024.0,
				totalNumberOfBytes.QuadPart  / 1024 / 1024 / 1024.0,
				(double)totalNumberOfFreeBytes.QuadPart / (double)totalNumberOfBytes.QuadPart * 100);
		
			resultL += cstr;

			cstr.Format(_T("%C: %s [%.1f GB]\r\n"), 
				j + 'A', volumeNameBuffer,
				totalNumberOfBytes.QuadPart  / 1024 / 1024 / 1024.0);

			resultM += cstr;

			cstr.Format(_T("%C: %s\r\n"), 
				j + 'A', volumeNameBuffer);

			resultS += cstr;
		}
	}
	resultL.TrimRight();
	resultM.TrimRight();
	resultS.TrimRight();

	if(resultL.GetLength() < maxLength)
	{
		return resultL;
	}
	else if(resultM.GetLength() < maxLength)
	{
		return resultM;
	}
	else
	{
		return resultS;
	}
}

void CDiskInfoDlg::InitDriveList()
{
	CString cstr;

	for(int i = 0; i < 8; i++)
	{
		m_LiDisk[i] = _T("");
		cstr.Format(_T("Disk%d"), i);
		SetElementPropertyEx(cstr, DISPID_IHTMLELEMENT_CLASSNAME, _T("hidden"));
		SetElementPropertyEx(cstr, DISPID_IHTMLELEMENT_TITLE, _T(""));
	}

	for(int i = 0; i < m_Ata.vars.GetCount(); i++)
	{
		CString className, driveLetter, diskStatus;
		if(m_Ata.vars[i].DriveMap.IsEmpty())
		{
			if(m_Ata.vars[i].PhysicalDriveId >= 0)
			{
				driveLetter.Format(_T("Disk %d"), m_Ata.vars[i].PhysicalDriveId);
			//	driveLetter.Format(_T("%s %d"), i18n(_T("DiskStatus"), _T("DISK")), m_Ata.vars[i].PhysicalDriveId);
			}
			else
			{
				driveLetter.Format(_T("Disk --"));
			//	driveLetter.Format(_T("%s --"), i18n(_T("DiskStatus"), _T("DISK")));
			}
		}
		else if(m_Ata.vars[i].DriveMap.GetLength() > 15)
		{
			driveLetter = m_Ata.vars[i].DriveMap.Left(11) + _T(" ...");
		}
		else
		{
			driveLetter = m_Ata.vars[i].DriveMap;
		}

		diskStatus = GetDiskStatus(m_Ata.vars[i].DiskStatus);
		className = GetDiskStatusClass(m_Ata.vars[i].DiskStatus);

		if(m_SelectDisk == i)
		{
			className += _T("Selected");
		}

		// DriveMenu
		if(i / 8 == m_DriveMenuPage)
		{
			CString targetDisk;
			targetDisk.Format(_T("Disk%d"), i % 8);
			if(m_Ata.vars[i].IsSmartEnabled && m_Ata.vars[i].Temperature > 0)
			{
				if(m_FlagFahrenheit)
				{
					m_LiDisk[i % 8].Format(_T("%s<br>%d °F<br>%s"), 
								diskStatus, m_Ata.vars[i].Temperature * 9 / 5 + 32, driveLetter);
				}
				else
				{
					m_LiDisk[i % 8].Format(_T("%s<br>%d °C<br>%s"), 
								diskStatus, m_Ata.vars[i].Temperature, driveLetter);
				}
			//	CString fahrenheit;
			//	fahrenheit.Format(_T("%d °F"), m_Ata.vars[i].Temperature * 9 / 5 + 32);
			//	SetElementPropertyEx(targetDisk, DISPID_IHTMLELEMENT_TITLE, fahrenheit);
			}
			else if(m_Ata.vars[i].IsSmartEnabled)
			{
				if(m_FlagFahrenheit)
				{
					m_LiDisk[i % 8].Format(_T("%s<br>-- °F<br>%s"), diskStatus, driveLetter);
				}
				else
				{
					m_LiDisk[i % 8].Format(_T("%s<br>-- °C<br>%s"), diskStatus, driveLetter);
				}
			}
			else
			{
				if(m_FlagFahrenheit)
				{
					m_LiDisk[i % 8].Format(_T("----<br>-- °F<br>%s"), driveLetter);
				}
				else
				{
					m_LiDisk[i % 8].Format(_T("----<br>-- °C<br>%s"), driveLetter);
				}
			}
			SetElementPropertyEx(targetDisk, DISPID_IHTMLELEMENT_CLASSNAME, className);

			if(m_Ata.vars[i].PhysicalDriveId >= 0)
			{
				cstr.Format(_T("Disk %d : %s %.1f GB\n%s"), m_Ata.vars[i].PhysicalDriveId, m_Ata.vars[i].Model, m_Ata.vars[i].TotalDiskSize / 1000.0, GetLogicalDriveInfo(i));
			}
			else
			{
				cstr.Format(_T("Disk -- : %s %.1f GB\n%s"), m_Ata.vars[i].Model, m_Ata.vars[i].TotalDiskSize / 1000.0, GetLogicalDriveInfo(i));
			}

			SetElementPropertyEx(targetDisk, DISPID_IHTMLELEMENT_TITLE, cstr);
		}
	}

	UpdateData(FALSE);
}
