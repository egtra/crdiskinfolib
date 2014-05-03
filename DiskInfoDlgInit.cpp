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

#include "DebugPrint.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

extern const GUID StrageGUID;

BOOL CDiskInfoDlg::OnInitDialog()
{
	CDHtmlMainDialog::OnInitDialog();

	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIconMini, FALSE);		// Set small icon

	InitThemeLang();
	InitMenu();

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

	CString cstr;
	DWORD debugMode = GetPrivateProfileInt(_T("Setting"), _T("DebugMode"), 0, m_Ini);
	SetDebugMode(debugMode);
	cstr.Format(_T("%d"), debugMode);
	WritePrivateProfileString(_T("Setting"), _T("DebugMode"), cstr, m_Ini);

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

	if(! InitAta((BOOL)GetPrivateProfileInt(_T("Setting"), _T("UseWMI"), 1, m_Ini), m_FlagAdvancedDiskSearch, NULL))
	{
//		AfxMessageBox(i18n(_T("Message"), _T("PLEASE_ENABLE_WMI")));
//		ShellExecute(NULL, NULL, _T("services.msc"), NULL, NULL, SW_SHOWNORMAL);	
//		EndDialog(0);
	}
//	else
	{
		if(m_FlagStartupExit)
		{
			EndDialog(0);
			return FALSE;
		}

		EnableDpiAware();
		InitDHtmlDialog(m_SizeX, m_SizeY, ((CDiskInfoApp*)AfxGetApp())->m_MainDlgPath);
	//	SetWindowTitle(_T("Initializing..."));
	}
/*
	DEV_BROADCAST_DEVICEINTERFACE filter;
	filter.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
	filter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
	filter.dbcc_classguid = StrageGUID;
	m_hDevNotify = RegisterDeviceNotification(m_hWnd, &filter, DEVICE_NOTIFY_WINDOW_HANDLE);
*/

	return TRUE; 
}

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
			CheckHideSerialNumber();
			ChangeTheme(m_CurrentTheme);
			ChangeZoomType(m_ZoomType);
			UpdateDialogSize();
			ChangeLang(m_CurrentLang);
			CheckPage();
			
			m_FlagShowWindow = TRUE;		

			CenterWindow();

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

		}
	}
}

VOID CDiskInfoDlg::UpdateThreshold()
{
	m_Ata.Threshold05     = GetPrivateProfileInt(_T("HealthStatus"), _T("ThreasholdOfCaution05"), 1, m_Ini);
	m_Ata.ThresholdC5     = GetPrivateProfileInt(_T("HealthStatus"), _T("ThreasholdOfCautionC5"), 1, m_Ini);
	m_Ata.ThresholdC6     = GetPrivateProfileInt(_T("HealthStatus"), _T("ThreasholdOfCautionC6"), 1, m_Ini);
}

BOOL CDiskInfoDlg::InitAta(BOOL useWmi, BOOL advancedDiskSearch, PBOOL flagChangeDisk)
{
	KillTimer(TIMER_SET_POWER_ON_UNIT);
	SetWindowTitle(i18n(_T("Message"), _T("DETECT_DISK")));
	m_PowerOnHoursClass = _T("valueR");
	m_NowDetectingUnitPowerOnHours = FALSE;
	UpdateThreshold();

	if(m_Ata.Init(useWmi, advancedDiskSearch, flagChangeDisk))
	{
		if(! m_FlagStartupExit)
		{
			CheckResident();
		}

		DWORD errorCount = 0;
		for(int i = 0; i < m_Ata.vars.GetCount(); i++)
		{
			int unitType = GetPrivateProfileInt(_T("PowerOnUnit"), m_Ata.vars[i].ModelSerial, -1, m_Ini);
			if(unitType >= 0)
			{
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

			SaveSmartInfo(i);

#ifdef BENCHMARK
			// Benchmark Reuslt
			TCHAR str[256];
			GetPrivateProfileString(_T("Benchmark"), m_Ata.vars[i].ModelSerial, _T("0.0"), str, 256, m_Ini);
			m_Ata.vars[i].Speed = Decode10X(str) / 1000.0;
#endif
		}
		if(errorCount)
		{
			SetTimer(TIMER_SET_POWER_ON_UNIT, 130000, 0);
			SetWindowTitle(i18n(_T("Message"), _T("DETECT_UNIT_POWER_ON_HOURS")));
			m_PowerOnHoursClass = _T("valueR gray");
			m_NowDetectingUnitPowerOnHours = TRUE;
		}
		else
		{
			SetWindowTitle(_T(""));
		}

		AutoAamApmAdaption();

		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

void CDiskInfoDlg::InitListCtrl()
{
	// DWORD width = 0;
	// Get Cntrol Width
	// CRect cr;
	// m_List.GetWindowRect(&cr);
	//	width = (DWORD)(620 * m_ZoomRatio - GetSystemMetrics(SM_CXVSCROLL));

	DWORD style = ListView_GetExtendedListViewStyle(m_List.m_hWnd);
	style |= LVS_EX_FULLROWSELECT | /*LVS_EX_GRIDLINES |*/ LVS_EX_LABELTIP ;
	ListView_SetExtendedListViewStyle(m_List.m_hWnd, style);

	m_List.SetImageList(&m_ImageList, LVSIL_SMALL);
/*
	while(m_List.DeleteColumn(0)){}
	m_List.DeleteAllItems();
	m_List.InsertColumn(0, _T(""), LVCFMT_CENTER, 25, 0);
	m_List.InsertColumn(1, i18n(_T("Dialog"), _T("LIST_ID")), LVCFMT_CENTER, (int)(32 * m_ZoomRatio), 0);
	m_List.InsertColumn(3, i18n(_T("Dialog"), _T("LIST_CURRENT")), LVCFMT_RIGHT, (int)(72 * m_ZoomRatio), 0);
	m_List.InsertColumn(4, i18n(_T("Dialog"), _T("LIST_WORST")), LVCFMT_RIGHT, (int)(72 * m_ZoomRatio), 0);
	m_List.InsertColumn(5, i18n(_T("Dialog"), _T("LIST_THRESHOLD")), LVCFMT_RIGHT, (int)(72 * m_ZoomRatio), 0);
	m_List.InsertColumn(6, i18n(_T("Dialog"), _T("LIST_RAW_VALUES")), LVCFMT_RIGHT, (int)(140 * m_ZoomRatio), 0);
	m_List.InsertColumn(2, i18n(_T("Dialog"), _T("LIST_ATTRIBUTE_NAME")), LVCFMT_LEFT, (int)(width - 388 * m_ZoomRatio - 25), 0);
*/
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
		for(DWORD j = 0; j < m_Ata.vars[index].AttributeCount; j++)
		{

			if(((0x01 <= m_Ata.vars[index].Attribute[j].Id && m_Ata.vars[index].Attribute[j].Id <= 0x0D)
			||	(0xBF <= m_Ata.vars[index].Attribute[j].Id && m_Ata.vars[index].Attribute[j].Id <= 0xD1)
			||	(0xDC <= m_Ata.vars[index].Attribute[j].Id && m_Ata.vars[index].Attribute[j].Id <= 0xE4)
			||	(0xE6 <= m_Ata.vars[index].Attribute[j].Id && m_Ata.vars[index].Attribute[j].Id <= 0xE7)
			||	m_Ata.vars[index].Attribute[j].Id == 0xF0
			||	m_Ata.vars[index].Attribute[j].Id == 0xFA
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
	if(m_Ata.vars[index].DiskStatus == CAtaSmart::DISK_STATUS_CAUTION
	|| m_Ata.vars[index].DiskStatus == CAtaSmart::DISK_STATUS_BAD)
	{
		for(DWORD j = 0; j < m_Ata.vars[index].AttributeCount; j++)
		{
			switch(m_Ata.vars[index].Attribute[j].Id)
			{
			case 0x05: // Reallocated Sectors Count
	//		case 0xC4: // Reallocation Event Count
			case 0xC5: // Current Pending Sector Count
			case 0xC6: // Off-Line Scan Uncorrectable Sector Count
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
				break;
			case 0xBB:
				if(m_Ata.vars[index].VendorId == m_Ata.SSD_VENDOR_MTRON)
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
				break;
			case 0xD1:
				if(m_Ata.vars[index].VendorId == m_Ata.SSD_VENDOR_INDILINX)
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
				break;
			// Debug
			/*
			case 0xE8:
				if(m_Ata.vars[index].VendorId == m_Ata.SSD_VENDOR_INTEL)
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
				break;
			*/
			default:
				break;
			}
		}
	}

	result.TrimRight();
	return result;
}

CString CDiskInfoDlg::GetLogicalDriveInfo(DWORD index)
{
	DWORD map = m_Ata.vars[index].DriveLetterMap;
	CString result;
	ULARGE_INTEGER freeBytesAvailableToCaller;
	ULARGE_INTEGER totalNumberOfBytes;
	ULARGE_INTEGER totalNumberOfFreeBytes;

	for(int j = 0; j < 26; j++)
	{
		if(map & (1 << j))
		{
			CString cstr;
			cstr.Format(_T("%C: "), j + 'A'); 
			GetDiskFreeSpaceEx(cstr, &freeBytesAvailableToCaller,
				&totalNumberOfBytes, &totalNumberOfFreeBytes);

			cstr.Format(_T("%C: %.1f/%.1f GB (%.1f %%)\r\n"), 
				j + 'A',
				totalNumberOfFreeBytes.QuadPart / 1024 / 1024 / 1024.0,
				totalNumberOfBytes.QuadPart  / 1024 / 1024 / 1024.0,
				(double)totalNumberOfFreeBytes.QuadPart / (double)totalNumberOfBytes.QuadPart * 100);
			result += cstr;
		}
	}
	result.TrimRight();
	return result;
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
					m_LiDisk[i % 8].Format(_T("%s<br>%d ÅãF<br>%s"), 
								diskStatus, m_Ata.vars[i].Temperature * 9 / 5 + 32, driveLetter);
				}
				else
				{
					m_LiDisk[i % 8].Format(_T("%s<br>%d ÅãC<br>%s"), 
								diskStatus, m_Ata.vars[i].Temperature, driveLetter);
				}
			//	CString fahrenheit;
			//	fahrenheit.Format(_T("%d ÅãF"), m_Ata.vars[i].Temperature * 9 / 5 + 32);
			//	SetElementPropertyEx(targetDisk, DISPID_IHTMLELEMENT_TITLE, fahrenheit);
			}
			else if(m_Ata.vars[i].IsSmartEnabled && m_Ata.vars[i].DiskStatus != CAtaSmart::DISK_STATUS_UNKNOWN)
			{
				if(m_FlagFahrenheit)
				{
					m_LiDisk[i % 8].Format(_T("%s<br>-- ÅãF<br>%s"), diskStatus, driveLetter);
				}
				else
				{
					m_LiDisk[i % 8].Format(_T("%s<br>-- ÅãC<br>%s"), diskStatus, driveLetter);
				}
			}
			else
			{
				if(m_FlagFahrenheit)
				{
					m_LiDisk[i % 8].Format(_T("----<br>-- ÅãF<br>%s"), driveLetter);
				}
				else
				{
					m_LiDisk[i % 8].Format(_T("----<br>-- ÅãC<br>%s"), driveLetter);
				}
			}
			SetElementPropertyEx(targetDisk, DISPID_IHTMLELEMENT_CLASSNAME, className);

			if(m_Ata.vars[i].DriveMap.GetLength() > 15)
			{
				cstr.Format(_T("Disk %d : %s %.1f GB\n%s"), m_Ata.vars[i].PhysicalDriveId, m_Ata.vars[i].Model, m_Ata.vars[i].TotalDiskSize / 1000.0, m_Ata.vars[i].DriveMap);
			}
			else
			{
				cstr.Format(_T("Disk %d : %s %.1f GB"), m_Ata.vars[i].PhysicalDriveId, m_Ata.vars[i].Model, m_Ata.vars[i].TotalDiskSize / 1000.0);
			}
			SetElementPropertyEx(targetDisk, DISPID_IHTMLELEMENT_TITLE, cstr);
		}
	}

	UpdateData(FALSE);
}
