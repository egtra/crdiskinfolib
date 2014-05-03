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

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

void CDiskInfoDlg::Refresh(DWORD flagForceUpdate)
{
	UpdateThreshold();

	CWaitCursor wait;
	BOOL flagUpdate = FALSE;
	DWORD smartUpdate[CAtaSmart::MAX_DISK] = {0};
	for(int i = 0; i < m_Ata.vars.GetCount(); i++)
	{
		if(flagForceUpdate || m_FlagAutoRefreshTarget[i])
		{
			switch(m_Ata.UpdateSmartInfo(i))
			{
			case CAtaSmart::SMART_STATUS_MAJOR_CHANGE:
				flagUpdate = TRUE;
				smartUpdate[i] = CAtaSmart::SMART_STATUS_MAJOR_CHANGE;
				SaveSmartInfo(i);
				break;
			case CAtaSmart::SMART_STATUS_MINOR_CHANGE:
				smartUpdate[i] = CAtaSmart::SMART_STATUS_MINOR_CHANGE;
				SaveSmartInfo(i);
				break;
			default:
				break;
			}
		}
	}

	if(flagForceUpdate || smartUpdate[m_SelectDisk] == CAtaSmart::SMART_STATUS_MAJOR_CHANGE)
	{
		ChangeDisk(m_SelectDisk);
		UpdateListCtrl(m_SelectDisk);
	}
	else if(smartUpdate[m_SelectDisk] == CAtaSmart::SMART_STATUS_MINOR_CHANGE)
	{
		UpdateListCtrl(m_SelectDisk);
	}

	if(flagForceUpdate || flagUpdate)
	{
		InitDriveList();
		UpdateToolTip();

		if(m_FlagResident)
		{
			AlarmOverheat();
			UpdateTrayTemperatureIcon(TRUE);
		}
	}
}

BOOL CDiskInfoDlg::UpdateListCtrl(DWORD i)
{
	static DWORD preSelectDisk = 0;

	if(m_Ata.vars.GetCount() == 0)
	{
		m_List.DeleteAllItems();
		return FALSE;
	}

	BOOL flag = FALSE;
	if(i == preSelectDisk && m_List.GetItemCount() > 0)
	{
		flag = TRUE;
	}
	else
	{
		m_List.SetRedraw(FALSE);
		m_List.DeleteAllItems();
		preSelectDisk = i;
	}

	if(m_Ata.vars[i].IsSmartCorrect)
	{
		m_List.SetTextColor1(RGB(0, 0, 0));
		m_List.SetTextColor2(RGB(0, 0, 0));
	}
	else
	{
		m_List.SetTextColor1(RGB(192, 192, 192));
		m_List.SetTextColor2(RGB(192, 192, 192));
	}

	CString cstr;
	DWORD caution = 0;
	UINT mask = LVIF_IMAGE;

	DWORD k = 0;

	for(DWORD j = 0; j < m_Ata.vars[i].AttributeCount; j++)
	{
		if(m_Ata.vars[i].Attribute[j].Id == 0x00)
		{
			continue;
		}
		
		if(m_Ata.vars[i].IsSmartCorrect)
		{
			switch(m_Ata.vars[i].Attribute[j].Id)
			{
			case 0x05: // Reallocated Sectors Count
	//		case 0xC4: // Reallocation Event Count
			case 0xC5: // Current Pending Sector Count
			case 0xC6: // Off-Line Scan Uncorrectable Sector Count
				{
				WORD raw = MAKEWORD(m_Ata.vars[i].Attribute[j].RawValue[0], m_Ata.vars[i].Attribute[j].RawValue[1]);
				WORD threshold; 
				switch(m_Ata.vars[i].Attribute[j].Id)
				{
				case 0x05:
					threshold = m_Ata.Threshold05;
					break;
				case 0xC5:
					threshold = m_Ata.ThresholdC5;
					break;
				case 0xC6:
					threshold = m_Ata.ThresholdC6;
					break;
				}
				if(threshold > 0 && raw >= threshold && ! m_Ata.vars[i].IsSsd)
				{
					caution = 1;
				}
				else
				{
					caution = 0;
				}

				if(m_Ata.vars[i].Threshold[j].ThresholdValue != 0 && m_Ata.vars[i].Attribute[j].CurrentValue <= m_Ata.vars[i].Threshold[j].ThresholdValue)
				{
					if(flag)
					{
						m_List.SetItem(k, 0, mask, _T(""), 2 /*IDI_BAD*/, 0, 0, 0, 0);
					}
					else
					{
						m_List.InsertItem(k, _T(""), 2 /*IDI_BAD*/);
					}
				}
				else if(caution > 0)
				{
					if(flag)
					{
						m_List.SetItem(k, 0, mask, _T(""), 1 /*IDI_CAUTION*/, 0, 0, 0, 0);
					}
					else
					{
						m_List.InsertItem(k, _T(""), 1 /*IDI_CAUTION*/);
					}
				}
				else
				{
					if(flag)
					{
						m_List.SetItem(k, 0, mask, _T(""), 0 /*IDI_GOOD*/, 0, 0, 0, 0);
					}
					else
					{
						m_List.InsertItem(k, _T(""), 0 /*IDI_GOOD*/);
					}
				}
				}
				break;
			case 0xBB: // Vendor Specific
				if(m_Ata.vars[i].VendorId == m_Ata.VENDOR_MTRON)
				{
					if(m_Ata.vars[i].Attribute[j].CurrentValue == 0)
					{
						if(flag)
						{
							m_List.SetItem(k, 0, mask, _T(""), 2 /*IDI_BAD*/, 0, 0, 0, 0);
						}
						else
						{
							m_List.InsertItem(k, _T(""), 2 /*IDI_BAD*/);
						}
					}
					else if(m_Ata.vars[i].Attribute[j].CurrentValue < 10)
					{
						if(flag)
						{
							m_List.SetItem(k, 0, mask, _T(""), 1 /*IDI_CAUTION*/, 0, 0, 0, 0);
						}
						else
						{
							m_List.InsertItem(k, _T(""), 1 /*IDI_CAUTION*/);
						}
					}
					else
					{
						if(flag)
						{
							m_List.SetItem(k, 0, mask, _T(""), 0 /*IDI_GOOD*/, 0, 0, 0, 0);
						}
						else
						{
							m_List.InsertItem(k, _T(""), 0 /*IDI_GOOD*/);
						}
					}
				}
				else
				{
					if(flag)
					{
						m_List.SetItem(k, 0, mask, _T(""), 0 /*IDI_GOOD*/, 0, 0, 0, 0);
					}
					else
					{
						m_List.InsertItem(k, _T(""), 0 /*IDI_GOOD*/);
					}
				}
				break;
			default:
				if(((0x01 <= m_Ata.vars[i].Attribute[j].Id && m_Ata.vars[i].Attribute[j].Id <= 0x0D)
				||	(0xBF <= m_Ata.vars[i].Attribute[j].Id && m_Ata.vars[i].Attribute[j].Id <= 0xD1)
				||	(0xDC <= m_Ata.vars[i].Attribute[j].Id && m_Ata.vars[i].Attribute[j].Id <= 0xE4)
				||	(0xE6 <= m_Ata.vars[i].Attribute[j].Id && m_Ata.vars[i].Attribute[j].Id <= 0xE7)
				||	m_Ata.vars[i].Attribute[j].Id == 0xF0
				||	m_Ata.vars[i].Attribute[j].Id == 0xFA
				))
				{
					if(m_Ata.vars[i].Threshold[j].ThresholdValue != 0 && m_Ata.vars[i].Attribute[j].CurrentValue <= m_Ata.vars[i].Threshold[j].ThresholdValue)
					{
						if(flag)
						{
							m_List.SetItem(k, 0, mask, _T(""), 2 /*IDI_BAD*/, 0, 0, 0, 0);
						}
						else
						{
							m_List.InsertItem(k, _T(""), 2 /*IDI_BAD*/);
						}
					}
					else
					{
						if(flag)
						{
							m_List.SetItem(k, 0, mask, _T(""), 0 /*IDI_GOOD*/, 0, 0, 0, 0);
						}
						else
						{
							m_List.InsertItem(k, _T(""), 0 /*IDI_GOOD*/);
						}
					}
				}
				else
				{
						if(flag)
						{
							m_List.SetItem(k, 0, mask, _T(""), 0 /*IDI_GOOD*/, 0, 0, 0, 0);
						}
						else
						{
							m_List.InsertItem(k, _T(""), 0 /*IDI_GOOD*/);
						}
				}

				break;
			}
		}
		else
		{
			if(flag)
			{
				m_List.SetItem(k, 0, mask, _T(""), 3 /*IDI_UNKNOWN*/, 0, 0, 0, 0);
			}
			else
			{
				m_List.InsertItem(k, _T(""), 3 /*IDI_UNKNOWN*/);
			}
		}

		cstr.Format(_T("%02X"), m_Ata.vars[i].Attribute[j].Id);
		m_List.SetItemText(k, 1, cstr);

		TCHAR str[256];
		TCHAR unknown[256];
		TCHAR vendorSpecific[256];
		GetPrivateProfileString(_T("Smart"), _T("UNKNOWN"), _T("Unknown"), unknown, 256, m_CurrentLangPath);
		GetPrivateProfileString(_T("Smart"), _T("VENDOR_SPECIFIC"), _T("Vendor Specific"), vendorSpecific, 256, m_CurrentLangPath);

		BYTE id = m_Ata.vars[i].Attribute[j].Id;
		if(id == 0xBB || id == 0xBD || id == 0xBE || id == 0xE5
		|| (0xE8 <= id && id <= 0xEF) || (0xF1 <= id && id <= 0xF9) || (0xFB <= id && id <= 0xFF))
		{
			GetPrivateProfileString(_T("Smart"), cstr, vendorSpecific, str, 256, m_CurrentLangPath);
		}
		else
		{
			GetPrivateProfileString(_T("Smart"), cstr, unknown, str, 256, m_CurrentLangPath);
		}

		if(m_Ata.vars[i].Attribute[j].Id == 0xBB && m_Ata.vars[i].VendorId == m_Ata.VENDOR_MTRON)
		{
			m_List.SetItemText(k, 2, _T("Total Erase Count (SSD)"));
		}
		else
		{
			m_List.SetItemText(k, 2, str);
		}

		cstr.Format(_T("%d"), m_Ata.vars[i].Attribute[j].CurrentValue);							
		m_List.SetItemText(k, 3, cstr);
		cstr.Format(_T("%d"), m_Ata.vars[i].Attribute[j].WorstValue);							
		m_List.SetItemText(k, 4, cstr);
		cstr.Format(_T("%d"), m_Ata.vars[i].Threshold[j].ThresholdValue);							
		m_List.SetItemText(k, 5, cstr);
		cstr.Format(_T("%02X%02X%02X%02X%02X%02X"), 
			m_Ata.vars[i].Attribute[j].RawValue[5],
			m_Ata.vars[i].Attribute[j].RawValue[4],
			m_Ata.vars[i].Attribute[j].RawValue[3],
			m_Ata.vars[i].Attribute[j].RawValue[2],
			m_Ata.vars[i].Attribute[j].RawValue[1],
			m_Ata.vars[i].Attribute[j].RawValue[0]
		);							
		m_List.SetItemText(k, 6, cstr);

		k++;
	}

	if(! flag)
	{
		m_List.SetRedraw(TRUE);
	}

	return TRUE;
}

BOOL CDiskInfoDlg::ChangeDisk(DWORD i)
{
	BOOL flagUpdate = FALSE;

	VARIANT dummy;
	VariantInit(&dummy);
	dummy.vt = VT_BSTR;

	if(m_Ata.vars.GetCount() == 0)
	{
		m_Model = i18n(_T("Message"), _T("DISK_NOT_FOUND"));
		m_Firmware = _T("");
		m_SerialNumber = _T("");
		m_PowerOnCount = _T("");
		m_PowerOnHours = _T("");
		m_BufferSize = _T("");
		m_NvCacheSize = _T("");
		m_RotationRate = _T("");
		m_LbaSize = _T("");
		m_Capacity = _T("");
		m_TransferMode = _T("");
		m_Interface = _T("");
		m_AtaAtapi = _T("");
		m_DiskStatus = _T("");
		m_Temperature = _T("");
		m_Feature = _T("");
		m_DriveMap = _T("");
		UpdateData(FALSE);

		UpdateListCtrl(i);
		
		SetElementPropertyEx(_T("DiskStatus"), DISPID_IHTMLELEMENT_CLASSNAME, &dummy, _T("diskStatusUnknown"));
		SetElementPropertyEx(_T("Temperature"), DISPID_IHTMLELEMENT_CLASSNAME, &dummy, _T("temperatureUnknown"));
		SetElementPropertyEx(_T("FeatureSmart"), DISPID_IHTMLELEMENT_CLASSNAME, &dummy, _T("unsupported"));
		SetElementPropertyEx(_T("FeatureApm"), DISPID_IHTMLELEMENT_CLASSNAME, &dummy, _T("unsupported"));
		SetElementPropertyEx(_T("FeatureAam"), DISPID_IHTMLELEMENT_CLASSNAME, &dummy, _T("unsupported"));
		SetElementPropertyEx(_T("Feature48Lba"), DISPID_IHTMLELEMENT_CLASSNAME, &dummy, _T("unsupported"));
		SetElementPropertyEx(_T("FeatureNcq"), DISPID_IHTMLELEMENT_CLASSNAME, &dummy, _T("unsupported"));

		return FALSE;
	}

	static int preSelectedDisk = -1;
	if(preSelectedDisk != i)
	{
		flagUpdate = TRUE;
		preSelectedDisk = i;
	}

	static int preFlagFahrenheit = -1;
	if(preFlagFahrenheit != m_FlagFahrenheit)
	{
		flagUpdate = TRUE;
		preFlagFahrenheit = m_FlagFahrenheit;		
	}

	static CString preLang = _T("");
	if(preLang.Compare(m_CurrentLang) != 0)
	{
		flagUpdate = TRUE;
		preLang = m_CurrentLang;
	}

	CString cstr, diskStatus, diskStatusReason, className, logicalDriveInfo;
	static CString preDiskStatus = _T("");
	static CString preDiskStatusReason = _T("");
	static CString preTemperatureStatus = _T("");
	static CString preLogicalDriveInfo = _T("");
	static BOOL preFlagHideSerialNumber = FALSE;

	diskStatus = GetDiskStatus(m_Ata.vars[i].DiskStatus);
	className = GetDiskStatusClass(m_Ata.vars[i].DiskStatus);
	diskStatusReason = GetDiskStatusReason(i);

	if(preDiskStatus.Compare(className) != 0)
	{
		SetElementPropertyEx(_T("DiskStatus"), DISPID_IHTMLELEMENT_CLASSNAME, &dummy, className);
		preDiskStatus = className;
	}
	if(preDiskStatusReason.Compare(diskStatusReason) != 0)
	{
		SetElementPropertyEx(_T("DiskStatus"), DISPID_IHTMLELEMENT_TITLE, &dummy, diskStatusReason);
		preDiskStatusReason = diskStatusReason;
	}

	m_DiskStatus.Format(_T("%s"), diskStatus);
	// MTRON SSD
	if(m_Ata.vars[i].Life >= 0)
	{
		cstr.Format(_T("\n%d %%"), m_Ata.vars[i].Life);
		m_DiskStatus += cstr;
	}

	className = GetTemperatureClass(m_Ata.vars[i].Temperature);
	if(preTemperatureStatus.Compare(className) != 0)
	{
		SetElementPropertyEx(_T("Temperature"), DISPID_IHTMLELEMENT_CLASSNAME, &dummy, className);
		preTemperatureStatus = className;
	}

	static int preTemperature = -1;
	if(preTemperature != m_Ata.vars[i].Temperature || flagUpdate)
	{
		if(m_Ata.vars[i].Temperature > 0)
		{
			if(m_FlagFahrenheit)
			{
				m_Temperature.Format(_T("%d ‹F"), m_Ata.vars[i].Temperature * 9 / 5 + 32);
			}
			else
			{
				m_Temperature.Format(_T("%d ‹C"), m_Ata.vars[i].Temperature);
			}
		//	CString fahrenheit;
		//	fahrenheit.Format(_T("%d ‹F"), m_Ata.vars[i].Temperature * 9 / 5 + 32);
		//	SetElementPropertyEx(_T("Temperature"), DISPID_IHTMLELEMENT_TITLE, &dummy, fahrenheit);
		}
		else
		{
			if(m_FlagFahrenheit)
			{
				m_Temperature.Format(_T("-- ‹F"));
			}
			else
			{
				m_Temperature.Format(_T("-- ‹C"));
			}
		}
		preTemperature = m_Ata.vars[i].Temperature;
	}

	logicalDriveInfo = GetLogicalDriveInfo(i);
	if(preLogicalDriveInfo.Compare(logicalDriveInfo) != 0)
	{
		SetElementPropertyEx(_T("DriveMap"), DISPID_IHTMLELEMENT_TITLE, &dummy, logicalDriveInfo);
		preLogicalDriveInfo = logicalDriveInfo;
	}

	if(m_Ata.vars[i].Sector48 >= m_Ata.vars[i].Sector28)
	{
		cstr.Format(_T("%I64d"), m_Ata.vars[i].Sector48);
		SetElementPropertyEx(_T("LbaSize"), DISPID_IHTMLELEMENT_TITLE, &dummy, cstr);
		m_LabelLbaSize = _T("48bit LBA");
	}
	else if(m_Ata.vars[i].Sector28 > 0)
	{
		cstr.Format(_T("%d"), m_Ata.vars[i].Sector28);
		SetElementPropertyEx(_T("LbaSize"), DISPID_IHTMLELEMENT_TITLE, &dummy, cstr);
		m_LabelLbaSize = _T("28bit LBA");
	}
	else
	{
		cstr = _T("");
		SetElementPropertyEx(_T("LbaSize"), DISPID_IHTMLELEMENT_TITLE, &dummy, cstr);
		m_LabelLbaSize = _T("CHS");
	}

	if(preFlagHideSerialNumber != m_FlagHideSerialNumber)
	{
		preFlagHideSerialNumber = m_FlagHideSerialNumber;
		flagUpdate = TRUE;
	}

	if(m_FlagHideSerialNumber)
	{
		m_SerialNumber = _T("");
		for(int j = 0; j < m_Ata.vars[i].SerialNumber.GetLength(); j++)
		{
			m_SerialNumber += _T("*");
		}
	}
	else
	{
		m_SerialNumber = m_Ata.vars[i].SerialNumber;
	}
	
	m_Model = m_Ata.vars[i].Model;
	SetElementPropertyEx(_T("Model"), DISPID_IHTMLELEMENT_TITLE, &dummy, m_Ata.vars[i].Enclosure);
	m_Firmware = m_Ata.vars[i].FirmwareRev;

	static int prePowerOnCount = -1;
	if(flagUpdate || prePowerOnCount != m_Ata.vars[i].PowerOnCount)
	{
		if(m_Ata.vars[i].PowerOnCount > 0)
		{
			m_PowerOnCount.Format(_T("%d %s"), m_Ata.vars[i].PowerOnCount, i18n(_T("Dialog"), _T("POWER_ON_COUNT_UNIT")));
		}
		else
		{
			m_PowerOnCount = i18n(_T("Dialog"), _T("UNKNOWN"));
		}
		prePowerOnCount = m_Ata.vars[i].PowerOnCount;
		flagUpdate = TRUE;
	}

	CString IsMinutes;
	CString IsMinutesT;
	CString title;	static int prePowerOnHours = -1;
	if(m_Ata.vars[i].MeasuredPowerOnHours > 0)
	{
		if(flagUpdate || prePowerOnHours != m_Ata.vars[i].MeasuredPowerOnHours)
		{
			if(m_Ata.vars[i].MeasuredTimeUnitType == CAtaSmart::POWER_ON_MINUTES)
			{
				if(m_Ata.vars[i].IsMaxtorMinute)
				{
					IsMinutes = _T("?");
					IsMinutesT = _T(" (?)");
				}
			}
			else
			{
				IsMinutes = _T(" ");
				IsMinutesT = _T("");
			}
			title.Format(_T("%d %s %d %s%s"),
				m_Ata.vars[i].MeasuredPowerOnHours / 24, i18n(_T("Dialog"), _T("POWER_ON_DAYS_UNIT")),
				m_Ata.vars[i].MeasuredPowerOnHours % 24, i18n(_T("Dialog"), _T("POWER_ON_HOURS_UNIT")), 
				IsMinutesT);

			m_PowerOnHours.Format(_T("%d%s%s"),
				m_Ata.vars[i].MeasuredPowerOnHours, IsMinutes, i18n(_T("Dialog"), _T("POWER_ON_HOURS_UNIT")));

			SetElementPropertyEx(_T("PowerOnHours"), DISPID_IHTMLELEMENT_CLASSNAME, &dummy, m_PowerOnHoursClass);
			SetElementPropertyEx(_T("PowerOnHours"), DISPID_IHTMLELEMENT_TITLE, &dummy, title);

			prePowerOnHours = m_Ata.vars[i].MeasuredPowerOnHours;
			flagUpdate = TRUE;
		}
	}
	else if(m_Ata.vars[i].DetectedPowerOnHours >= 0)
	{
		if(flagUpdate || prePowerOnHours != m_Ata.vars[i].DetectedPowerOnHours)
		{
			if(m_Ata.vars[i].DetectedTimeUnitType == CAtaSmart::POWER_ON_MINUTES)
			{
				if(m_Ata.vars[i].IsMaxtorMinute)
				{
					IsMinutes = _T("?");
					IsMinutesT = _T(" (?)");
				}
			}
			else
			{
				IsMinutes = _T(" ");
				IsMinutesT = _T("");
			}
			title.Format(_T("%d %s %d %s%s"),
				m_Ata.vars[i].DetectedPowerOnHours / 24, i18n(_T("Dialog"), _T("POWER_ON_DAYS_UNIT")),
				m_Ata.vars[i].DetectedPowerOnHours % 24, i18n(_T("Dialog"), _T("POWER_ON_HOURS_UNIT")), 
				IsMinutesT);

			m_PowerOnHours.Format(_T("%d%s%s"),
				m_Ata.vars[i].DetectedPowerOnHours, IsMinutes, i18n(_T("Dialog"), _T("POWER_ON_HOURS_UNIT")));

			SetElementPropertyEx(_T("PowerOnHours"), DISPID_IHTMLELEMENT_CLASSNAME, &dummy, m_PowerOnHoursClass);
			SetElementPropertyEx(_T("PowerOnHours"), DISPID_IHTMLELEMENT_TITLE, &dummy, title);

			prePowerOnHours = m_Ata.vars[i].DetectedPowerOnHours;
			flagUpdate = TRUE;
		}
	}
	else
	{
		if(flagUpdate || prePowerOnHours != 0)
		{
			m_PowerOnHours = i18n(_T("Dialog"), _T("UNKNOWN"));
			SetElementPropertyEx(_T("PowerOnHours"), DISPID_IHTMLELEMENT_CLASSNAME, &dummy, m_PowerOnHoursClass);
			SetElementPropertyEx(_T("PowerOnHours"), DISPID_IHTMLELEMENT_TITLE, &dummy, title);

			prePowerOnHours = 0;
			flagUpdate = TRUE;
		}
	}

	if(m_Ata.vars[i].BufferSize > 0)
	{
		m_BufferSize.Format(_T("%d KB"), m_Ata.vars[i].BufferSize / 1024);
		SetElementPropertyEx(_T("BufferSize"), DISPID_IHTMLELEMENT_CLASSNAME, &dummy, _T("supported"));
	}
	else
	{
		m_BufferSize = i18n(_T("Dialog"), _T("UNKNOWN"));
		SetElementPropertyEx(_T("BufferSize"), DISPID_IHTMLELEMENT_CLASSNAME, &dummy, _T("unsupported"));
	}

	if(m_Ata.vars[i].NvCacheSize > 0)
	{
		m_NvCacheSize.Format(_T("%d MB"), (DWORD)(m_Ata.vars[i].NvCacheSize / 1024 / 1024));
	}
	else
	{
		m_NvCacheSize = _T("----");
	}

	if(m_Ata.vars[i].NominalMediaRotationRate == 1) // SSD
	{
		m_RotationRate = _T("---- (SSD)");
		SetElementPropertyEx(_T("RotationRate"), DISPID_IHTMLELEMENT_CLASSNAME, &dummy, _T("supported"));
	}
	else if(m_Ata.vars[i].NominalMediaRotationRate > 0)
	{
		m_RotationRate.Format(_T("%d RPM"), m_Ata.vars[i].NominalMediaRotationRate);
		SetElementPropertyEx(_T("RotationRate"), DISPID_IHTMLELEMENT_CLASSNAME, &dummy, _T("supported"));
	}
	else
	{
		m_RotationRate = i18n(_T("Dialog"), _T("UNKNOWN"));
		SetElementPropertyEx(_T("RotationRate"), DISPID_IHTMLELEMENT_CLASSNAME, &dummy, _T("unsupported"));
	}

	if(m_Ata.vars[i].DiskSizeLba48 >= m_Ata.vars[i].DiskSizeLba28)
	{
		m_LbaSize.Format(_T("%.1f GB"), m_Ata.vars[i].DiskSizeLba48 / 1000.0);
	}
	else if(m_Ata.vars[i].DiskSizeLba28 > 0)
	{
		m_LbaSize.Format(_T("%.1f GB"), m_Ata.vars[i].DiskSizeLba28 / 1000.0);
	}
	else if(m_Ata.vars[i].DiskSizeChs > 0)
	{
		m_LbaSize.Format(_T("%.1f GB"), m_Ata.vars[i].DiskSizeChs / 1000.0);
	}
	else
	{
		m_LbaSize = _T("----");
	}

	m_DriveMap = m_Ata.vars[i].DriveMap;
	if(m_Ata.vars[i].TotalDiskSize < 1000)
	{
		m_Capacity.Format(_T("%.2f GB"), m_Ata.vars[i].TotalDiskSize / 1000.0);
	}
	else if(m_Ata.vars[i].TotalDiskSize > 0)
	{
		m_Capacity.Format(_T("%.1f GB"), m_Ata.vars[i].TotalDiskSize / 1000.0);
	}
	else
	{
		m_Capacity = i18n(_T("Dialog"), _T("UNKNOWN"));
	}
	m_TransferMode = m_Ata.vars[i].MaxTransferMode;
	m_Interface = m_Ata.vars[i].Interface;
	m_AtaAtapi = m_Ata.vars[i].MajorVersion + _T(" | ") + m_Ata.vars[i].MinorVersion;
	if(! m_Ata.vars[i].MinorVersion.IsEmpty())
	{
	//	SetElementPropertyEx(_T("AtaAtapi"), DISPID_IHTMLELEMENT_TITLE, &dummy, m_Ata.vars[i].MinorVersion);
	}

	m_Feature = _T("");

	if(m_Ata.vars[i].IsSmartSupported)
	{
		m_Feature += _T("S.M.A.R.T., ");
		SetElementPropertyEx(_T("FeatureSmart"), DISPID_IHTMLELEMENT_CLASSNAME, &dummy, _T("supported"));
	}
	else
	{
		SetElementPropertyEx(_T("FeatureSmart"), DISPID_IHTMLELEMENT_CLASSNAME, &dummy, _T("unsupported"));
	}

	if(m_Ata.vars[i].IsApmSupported)
	{
		m_Feature += _T("APM, ");
		SetElementPropertyEx(_T("FeatureApm"), DISPID_IHTMLELEMENT_CLASSNAME, &dummy, _T("supported"));
	}
	else
	{
		SetElementPropertyEx(_T("FeatureApm"), DISPID_IHTMLELEMENT_CLASSNAME, &dummy, _T("unsupported"));
	}

	if(m_Ata.vars[i].IsAamSupported)
	{
		m_Feature += _T("AAM, ");
		SetElementPropertyEx(_T("FeatureAam"), DISPID_IHTMLELEMENT_CLASSNAME, &dummy, _T("supported"));
	}
	else
	{
		SetElementPropertyEx(_T("FeatureAam"), DISPID_IHTMLELEMENT_CLASSNAME, &dummy, _T("unsupported"));
	}

	if(m_Ata.vars[i].IsLba48Supported)
	{
		m_Feature += _T("48bit LBA, ");
		SetElementPropertyEx(_T("Feature48Lba"), DISPID_IHTMLELEMENT_CLASSNAME, &dummy, _T("supported"));
	}
	else
	{
		SetElementPropertyEx(_T("Feature48Lba"), DISPID_IHTMLELEMENT_CLASSNAME, &dummy, _T("unsupported"));
	}

	if(m_Ata.vars[i].IsNcqSupported)
	{
		m_Feature += _T("NCQ, ");
		SetElementPropertyEx(_T("FeatureNcq"), DISPID_IHTMLELEMENT_CLASSNAME, &dummy, _T("supported"));
	}
	else
	{
		SetElementPropertyEx(_T("FeatureNcq"), DISPID_IHTMLELEMENT_CLASSNAME, &dummy, _T("unsupported"));
	}

	if(m_Ata.vars[i].IsTrimSupported)
	{
		m_Feature += _T("TRIM, ");
		SetElementPropertyEx(_T("FeatureTrim"), DISPID_IHTMLELEMENT_CLASSNAME, &dummy, _T("supported"));
	}
	else
	{
		SetElementPropertyEx(_T("FeatureTrim"), DISPID_IHTMLELEMENT_CLASSNAME, &dummy, _T("unsupported"));
	}

/*
	if(m_Ata.vars[i].IsNvCacheSupported)
	{
		m_Feature += _T("NV Cache, ");
		SetElementPropertyEx(_T("FeatureNvc"), DISPID_IHTMLELEMENT_CLASSNAME, &dummy, _T("supported"));
	}
	else
	{
		SetElementPropertyEx(_T("FeatureNvc"), DISPID_IHTMLELEMENT_CLASSNAME, &dummy, _T("unsupported"));
	}

	if(m_Ata.vars[i].IsSsd)
	{
		m_Feature += _T("SSD, ");
		SetElementPropertyEx(_T("FeatureSsd"), DISPID_IHTMLELEMENT_CLASSNAME, &dummy, _T("supported"));
	}
	else
	{
		SetElementPropertyEx(_T("FeatureSsd"), DISPID_IHTMLELEMENT_CLASSNAME, &dummy, _T("unsupported"));
	}
*/
	if(! m_Feature.IsEmpty())
	{
		m_Feature.Delete(m_Feature.GetLength() - 2, 2);
	}

#ifdef BENCHMARK
	SetMeter(m_Ata.vars[m_SelectDisk].Speed);
#endif


	UpdateData(FALSE);

	return TRUE;
}

void CDiskInfoDlg::ChangeLang(CString LangName)
{
	m_CurrentLangPath.Format(_T("%s\\%s.lang"), m_LangDir, LangName);
	CString cstr;
	CMenu *menu = GetMenu();
	CMenu subMenu;
	CMenu subSubMenu;
	UINT menuState = 0;

	cstr = i18n(_T("Menu"), _T("FILE"));
	menu->ModifyMenu(0, MF_BYPOSITION | MF_STRING, 0, cstr);
	cstr = i18n(_T("Menu"), _T("EDIT"));
	menu->ModifyMenu(1, MF_BYPOSITION | MF_STRING, 1, cstr);
	cstr = i18n(_T("Menu"), _T("FUNCTION"));
	menu->ModifyMenu(2, MF_BYPOSITION | MF_STRING, 2, cstr);
	cstr = i18n(_T("Menu"), _T("THEME"));
	menu->ModifyMenu(3, MF_BYPOSITION | MF_STRING, 3, cstr);
	cstr = i18n(_T("Menu"), _T("DISK"));
	menu->ModifyMenu(4, MF_BYPOSITION | MF_STRING, 4, cstr);
	cstr = i18n(_T("Menu"), _T("HELP"));
	menu->ModifyMenu(5, MF_BYPOSITION | MF_STRING, 5, cstr);
	cstr = i18n(_T("Menu"), _T("LANGUAGE"));

	if(cstr.Find(_T("Language")) >= 0)
	{
		cstr = _T("&Language");
		menu->ModifyMenu(6, MF_BYPOSITION | MF_STRING, 6, cstr);
	}
	else
	{
		menu->ModifyMenu(6, MF_BYPOSITION | MF_STRING, 6, cstr + _T("(&Language)"));
	}

	cstr = i18n(_T("Menu"), _T("CUSTOMIZE"));
	menuState = menu->GetMenuState(ID_CUSTOMIZE, MF_BYCOMMAND);
	menu->ModifyMenu(ID_CUSTOMIZE, MF_STRING, ID_CUSTOMIZE, cstr);
	menu->EnableMenuItem(ID_CUSTOMIZE, menuState);

	cstr = i18n(_T("Menu"), _T("EXIT"));
	menu->ModifyMenu(ID_FILE_EXIT, MF_STRING, ID_FILE_EXIT, cstr);
	cstr = i18n(_T("Menu"), _T("COPY"));
	cstr += _T("\tCtrl + C");
	menu->ModifyMenu(ID_EDIT_COPY, MF_STRING, ID_EDIT_COPY, cstr);

	// Edit
	subMenu.Attach(menu->GetSubMenu(1)->GetSafeHmenu());
	cstr = i18n(_T("Menu"), _T("COPY_OPTION"));
	subMenu.ModifyMenu(1, MF_BYPOSITION, 1, cstr);
	subMenu.Detach();

	cstr = i18n(_T("Menu"), _T("HIDE_SMART_INFO"));
	menu->ModifyMenu(ID_HIDE_SMART_INFO, MF_STRING, ID_HIDE_SMART_INFO, cstr);
	cstr = i18n(_T("Menu"), _T("HIDE_SERIAL_NUMBER"));
	menu->ModifyMenu(ID_HIDE_SERIAL_NUMBER, MF_STRING, ID_HIDE_SERIAL_NUMBER, cstr);
	cstr = i18n(_T("Menu"), _T("ADVANCED_DISK_SEARCH"));
	menu->ModifyMenu(ID_ADVANCED_DISK_SEARCH, MF_STRING, ID_ADVANCED_DISK_SEARCH, cstr);
	cstr = i18n(_T("Menu"), _T("EVENT_LOG"));
	menu->ModifyMenu(ID_EVENT_LOG, MF_STRING, ID_EVENT_LOG, cstr);
	cstr = i18n(_T("Menu"), _T("RESIDENT"));
	menu->ModifyMenu(ID_RESIDENT, MF_STRING, ID_RESIDENT, cstr);
	
	cstr = i18n(_T("Menu"), _T("HEALTH_STATUS_SETTING"));
	menu->ModifyMenu(ID_HEALTH_STATUS, MF_STRING, ID_HEALTH_STATUS, cstr);

	cstr = i18n(_T("Menu"), _T("AAM_APM_CONTROL"));
	menuState = menu->GetMenuState(ID_AAM_APM, MF_BYCOMMAND);
	menu->ModifyMenu(ID_AAM_APM, MF_STRING, ID_AAM_APM, cstr);
//	menu->EnableMenuItem(ID_AAM_APM, menuState);

	if(m_Ata.vars.GetCount() && menuState != MF_GRAYED)
	{
		menu->EnableMenuItem(ID_AAM_APM, MF_ENABLED);
	}
	else
	{
		menu->EnableMenuItem(ID_AAM_APM, MF_GRAYED);
	}

	cstr = i18n(_T("Menu"), _T("AUTO_AAM_APM_ADAPTION"));
	menu->ModifyMenu(ID_AUTO_AAM_APM, MF_STRING, ID_AUTO_AAM_APM, cstr);
	cstr = i18n(_T("Menu"), _T("STARTUP"));
	menu->ModifyMenu(ID_STARTUP, MF_STRING, ID_STARTUP, cstr);
	cstr = i18n(_T("Menu"), _T("GRAPH"));
	menu->ModifyMenu(ID_GRAPH, MF_STRING, ID_GRAPH, cstr);

	cstr = i18n(_T("Menu"), _T("REFRESH"));
	cstr += _T("\tF5");
	menu->ModifyMenu(ID_REFRESH, MF_STRING, ID_REFRESH, cstr);

	cstr = i18n(_T("Menu"), _T("RESCAN"));
	cstr += _T("\tF6");
	menu->ModifyMenu(ID_RESCAN, MF_STRING, ID_RESCAN, cstr);

	// Function
	subMenu.Attach(menu->GetSubMenu(2)->GetSafeHmenu());
	cstr = i18n(_T("Menu"), _T("AUTO_REFRESH"));
	subMenu.ModifyMenu(1, MF_BYPOSITION, 1, cstr);
	cstr = i18n(_T("Menu"), _T("AUTO_REFRESH_TARGET"));
	subMenu.ModifyMenu(2, MF_BYPOSITION, 2, cstr);
	cstr = i18n(_T("Menu"), _T("TEMPERATURE_TYPE"));
	subMenu.ModifyMenu(9, MF_BYPOSITION, 9, cstr);
	cstr = i18n(_T("Menu"), _T("RESIDENT_STYLE"));
	subMenu.ModifyMenu(12, MF_BYPOSITION, 12, cstr);
	cstr = i18n(_T("Menu"), _T("WAIT_TIME_AT_STARTUP"));
	subMenu.ModifyMenu(14, MF_BYPOSITION, 14, cstr);
	cstr = i18n(_T("Menu"), _T("ADVANCED_FEATURE"));
	subMenu.ModifyMenu(19, MF_BYPOSITION, 19, cstr);
	subMenu.Detach();

	cstr = i18n(_T("Menu"), _T("OPEN_DISK_MANAGEMENT"));
	menu->ModifyMenu(ID_OPEN_DISK_MANAGEMENT, MF_STRING, ID_OPEN_DISK_MANAGEMENT, cstr);
	cstr = i18n(_T("Menu"), _T("OPEN_DEVICE_MANAGER"));
	menu->ModifyMenu(ID_OPEN_DEVICE_MANAGER, MF_STRING, ID_OPEN_DEVICE_MANAGER, cstr);

	cstr = i18n(_T("Menu"), _T("DISABLE"));
	menu->ModifyMenu(ID_AUTO_REFRESH_DISABLE, MF_STRING, ID_AUTO_REFRESH_DISABLE, cstr);
	cstr = i18n(_T("Menu"), _T("AUTO_REFRESH_01_MIN"));
	menu->ModifyMenu(ID_AUTO_REFRESH_01_MIN, MF_STRING, ID_AUTO_REFRESH_01_MIN, cstr);
	cstr = i18n(_T("Menu"), _T("AUTO_REFRESH_03_MIN"));
	menu->ModifyMenu(ID_AUTO_REFRESH_03_MIN, MF_STRING, ID_AUTO_REFRESH_03_MIN, cstr);
	cstr = i18n(_T("Menu"), _T("AUTO_REFRESH_05_MIN"));
	menu->ModifyMenu(ID_AUTO_REFRESH_05_MIN, MF_STRING, ID_AUTO_REFRESH_05_MIN, cstr);
	cstr = i18n(_T("Menu"), _T("AUTO_REFRESH_10_MIN"));
	menu->ModifyMenu(ID_AUTO_REFRESH_10_MIN, MF_STRING, ID_AUTO_REFRESH_10_MIN, cstr);
	cstr = i18n(_T("Menu"), _T("AUTO_REFRESH_30_MIN"));
	menu->ModifyMenu(ID_AUTO_REFRESH_30_MIN, MF_STRING, ID_AUTO_REFRESH_30_MIN, cstr);
	cstr = i18n(_T("Menu"), _T("AUTO_REFRESH_60_MIN"));
	menu->ModifyMenu(ID_AUTO_REFRESH_60_MIN, MF_STRING, ID_AUTO_REFRESH_60_MIN, cstr);

	CheckRadioAutoRefresh();

	cstr = i18n(_T("Menu"), _T("WAIT_0_SEC"));
	menu->ModifyMenu(ID_WAIT_0_SEC, MF_STRING, ID_WAIT_0_SEC, cstr);
	cstr = i18n(_T("Menu"), _T("WAIT_5_SEC"));
	menu->ModifyMenu(ID_WAIT_5_SEC, MF_STRING, ID_WAIT_5_SEC, cstr);
	cstr = i18n(_T("Menu"), _T("WAIT_10_SEC"));
	menu->ModifyMenu(ID_WAIT_10_SEC, MF_STRING, ID_WAIT_10_SEC, cstr);
	cstr = i18n(_T("Menu"), _T("WAIT_15_SEC"));
	menu->ModifyMenu(ID_WAIT_15_SEC, MF_STRING, ID_WAIT_15_SEC, cstr);
	cstr = i18n(_T("Menu"), _T("WAIT_20_SEC"));
	menu->ModifyMenu(ID_WAIT_20_SEC, MF_STRING, ID_WAIT_20_SEC, cstr);
	cstr = i18n(_T("Menu"), _T("WAIT_30_SEC"));
	menu->ModifyMenu(ID_WAIT_30_SEC, MF_STRING, ID_WAIT_30_SEC, cstr);
	cstr = i18n(_T("Menu"), _T("WAIT_40_SEC"));
	menu->ModifyMenu(ID_WAIT_40_SEC, MF_STRING, ID_WAIT_40_SEC, cstr);
	cstr = i18n(_T("Menu"), _T("WAIT_50_SEC"));
	menu->ModifyMenu(ID_WAIT_50_SEC, MF_STRING, ID_WAIT_50_SEC, cstr);
	cstr = i18n(_T("Menu"), _T("WAIT_60_SEC"));
	menu->ModifyMenu(ID_WAIT_60_SEC, MF_STRING, ID_WAIT_60_SEC, cstr);
	cstr = i18n(_T("Menu"), _T("WAIT_90_SEC"));
	menu->ModifyMenu(ID_WAIT_90_SEC, MF_STRING, ID_WAIT_90_SEC, cstr);
	cstr = i18n(_T("Menu"), _T("WAIT_120_SEC"));
	menu->ModifyMenu(ID_WAIT_120_SEC, MF_STRING, ID_WAIT_120_SEC, cstr);
	cstr = i18n(_T("Menu"), _T("WAIT_150_SEC"));
	menu->ModifyMenu(ID_WAIT_150_SEC, MF_STRING, ID_WAIT_150_SEC, cstr);
	cstr = i18n(_T("Menu"), _T("WAIT_180_SEC"));
	menu->ModifyMenu(ID_WAIT_180_SEC, MF_STRING, ID_WAIT_180_SEC, cstr);
	cstr = i18n(_T("Menu"), _T("WAIT_210_SEC"));
	menu->ModifyMenu(ID_WAIT_210_SEC, MF_STRING, ID_WAIT_210_SEC, cstr);
	cstr = i18n(_T("Menu"), _T("WAIT_240_SEC"));
	menu->ModifyMenu(ID_WAIT_240_SEC, MF_STRING, ID_WAIT_240_SEC, cstr);

	CheckRadioWaitTime();

	cstr = i18n(_T("Menu"), _T("CELSIUS"));
	menu->ModifyMenu(ID_CELSIUS, MF_STRING, ID_CELSIUS, cstr);
	cstr = i18n(_T("Menu"), _T("FAHRENHEIT"));
	menu->ModifyMenu(ID_FAHRENHEIT, MF_STRING, ID_FAHRENHEIT, cstr);

	if(m_FlagFahrenheit)
	{
		OnFahrenheit();
	}
	else
	{
		OnCelsius();
	}

	cstr = i18n(_T("Menu"), _T("HIDE"));
	menu->ModifyMenu(ID_RESIDENT_HIDE, MF_STRING, ID_RESIDENT_HIDE, cstr);
	cstr = i18n(_T("Menu"), _T("MINIMIZE"));
	menu->ModifyMenu(ID_RESIDENT_MINIMIZE, MF_STRING, ID_RESIDENT_MINIMIZE, cstr);

	if(m_FlagResidentMinimize)
	{
		OnResidentMinimize();
	}
	else
	{
		OnResidentHide();
	}

	cstr = i18n(_T("Menu"), _T("HELP"));
	menu->ModifyMenu(ID_HELP, MF_STRING, ID_HELP, cstr);
	cstr = i18n(_T("Menu"), _T("HELP_ABOUT"));
	menuState = menu->GetMenuState(ID_HELP_ABOUT, MF_BYCOMMAND);
	menu->ModifyMenu(ID_HELP_ABOUT, MF_STRING, ID_HELP_ABOUT, cstr);
	menu->EnableMenuItem(ID_HELP_ABOUT, menuState);

	cstr = i18n(_T("Menu"), _T("HELP_ABOUT_SMART"));
	menu->ModifyMenu(ID_HELP_ABOUT_SMART, MF_STRING, ID_HELP_ABOUT_SMART, cstr);

	cstr = i18n(_T("TrayMenu"), _T("ENABLE_ALL"));
	menu->ModifyMenu(ID_USB_ENABLE_ALL, MF_STRING, ID_USB_ENABLE_ALL, cstr);

	cstr = i18n(_T("TrayMenu"), _T("DISABLE_ALL"));
	menu->ModifyMenu(ID_USB_DISABLE_ALL, MF_STRING, ID_USB_DISABLE_ALL, cstr);

	// Check Status

	if(m_FlagDumpIdentifyDevice)
	{
		menu->CheckMenuItem(ID_DUMP_IDENTIFY_DEVICE, MF_CHECKED);
	}
	else
	{
		menu->CheckMenuItem(ID_DUMP_IDENTIFY_DEVICE, MF_UNCHECKED);
	}

	if(m_FlagDumpSmartReadData)
	{
		menu->CheckMenuItem(ID_DUMP_SMART_READ_DATA, MF_CHECKED);
	}
	else
	{
		menu->CheckMenuItem(ID_DUMP_SMART_READ_DATA, MF_UNCHECKED);
	}

	if(m_FlagDumpSmartReadThreshold)
	{
		menu->CheckMenuItem(ID_DUMP_SMART_READ_THRESHOLD, MF_CHECKED);
	}
	else
	{
		menu->CheckMenuItem(ID_DUMP_SMART_READ_THRESHOLD, MF_UNCHECKED);
	}

	if(m_FlagHideSerialNumber)
	{
		menu->CheckMenuItem(ID_HIDE_SERIAL_NUMBER, MF_CHECKED);
	}
	else
	{
		menu->CheckMenuItem(ID_HIDE_SERIAL_NUMBER, MF_UNCHECKED);
	}

	if(m_FlagHideSmartInfo)
	{
		menu->CheckMenuItem(ID_HIDE_SMART_INFO, MF_CHECKED);
	}
	else
	{
		menu->CheckMenuItem(ID_HIDE_SMART_INFO, MF_UNCHECKED);
	}

	if(m_FlagAdvancedDiskSearch)
	{
		menu->CheckMenuItem(ID_ADVANCED_DISK_SEARCH, MF_CHECKED);
	}
	else
	{
		menu->CheckMenuItem(ID_ADVANCED_DISK_SEARCH, MF_UNCHECKED);
	}

	if(GetIeVersion() >= 600)
	{
		menu->EnableMenuItem(ID_GRAPH, MF_ENABLED);
	}
	else
	{
		menu->EnableMenuItem(ID_GRAPH, MF_GRAYED);
	}

	if(m_FlagEventLogMenu)
	{
		menu->EnableMenuItem(ID_EVENT_LOG, MF_ENABLED);
		if(m_FlagEventLog)
		{
			menu->CheckMenuItem(ID_EVENT_LOG, MF_CHECKED);
		}
		else
		{
			menu->CheckMenuItem(ID_EVENT_LOG, MF_UNCHECKED);
		}
	}
	else
	{
		menu->EnableMenuItem(ID_EVENT_LOG, MF_GRAYED);
	}

	if(m_FlagAutoAamApm)
	{
		menu->CheckMenuItem(ID_AUTO_AAM_APM, MF_CHECKED);
	}
	else
	{
		menu->CheckMenuItem(ID_AUTO_AAM_APM, MF_UNCHECKED);
	}

	if(m_FlagResident)
	{
		menu->CheckMenuItem(ID_RESIDENT, MF_CHECKED);
	}
	else
	{
		menu->CheckMenuItem(ID_RESIDENT, MF_UNCHECKED);
	}

	if(m_FlagStartup)
	{
		menu->CheckMenuItem(ID_STARTUP, MF_CHECKED);
	}
	else
	{
		menu->CheckMenuItem(ID_STARTUP, MF_UNCHECKED);
	}

	// Disk
	subMenu.Attach(menu->GetSubMenu(MENU_DRIVE_INDEX)->GetSafeHmenu());
	while(subMenu.RemoveMenu(0, MF_BYPOSITION));

	MENUITEMINFO subMenuInfo;
	ZeroMemory(&subMenuInfo, sizeof(MENUITEMINFO));
	subMenuInfo.cbSize = sizeof(MENUITEMINFO);
	subMenuInfo.fMask = MIIM_CHECKMARKS|MIIM_TYPE|MIIM_STATE|MIIM_ID|MIIM_SUBMENU;
	subMenuInfo.fType = MFT_RADIOCHECK;
	subMenuInfo.hbmpChecked = NULL;
	subMenuInfo.fState = MFS_UNCHECKED;
	subMenuInfo.hSubMenu = NULL;

	for(int i = 0; i < m_Ata.vars.GetCount(); i++)
	{
		CString cstr;
		cstr.Format(_T("(%d) %s %.1f GB"), i + 1, m_Ata.vars[i].Model, m_Ata.vars[i].TotalDiskSize / 1000.0);
		subMenuInfo.wID = SELECT_DISK_BASE + i;
		subMenuInfo.dwTypeData = (LPWSTR)cstr.GetString();
		subMenu.InsertMenuItem(-1, &subMenuInfo);

		if(i % 8 == 7 && i + 1 != m_Ata.vars.GetCount())
		{
			subMenu.AppendMenu(MF_SEPARATOR);
		}
	}
	subMenu.CheckMenuRadioItem(SELECT_DISK_BASE, SELECT_DISK_BASE + (INT)m_Ata.vars.GetCount(),
								SELECT_DISK_BASE + m_SelectDisk, MF_BYCOMMAND);
	subMenu.Detach();

	// Auto Refresh Target
	subSubMenu.Attach(menu->GetSubMenu(2)->GetSubMenu(2)->GetSafeHmenu());
	while(subSubMenu.RemoveMenu(0, MF_BYPOSITION));

	MENUITEMINFO subSubMenuInfo;
	ZeroMemory(&subSubMenuInfo, sizeof(MENUITEMINFO));
	subSubMenuInfo.cbSize = sizeof(MENUITEMINFO);
	subSubMenuInfo.fMask = MIIM_CHECKMARKS|MIIM_TYPE|MIIM_STATE|MIIM_ID;
	subSubMenuInfo.fType = NULL;
	subSubMenuInfo.hbmpChecked = NULL;

	cstr = i18n(_T("Menu"), _T("AUTO_REFRESH_TARGET_ALL_DISK"));
	subSubMenuInfo.wID = AUTO_REFRESH_TARGET_BASE + CAtaSmart::MAX_DISK;
	subSubMenuInfo.dwTypeData = (LPWSTR)cstr.GetString();
	subSubMenu.InsertMenuItem(-1, &subSubMenuInfo);

	cstr = i18n(_T("Menu"), _T("AUTO_REFRESH_UNTARGET_ALL_DISK"));
	subSubMenuInfo.wID = AUTO_REFRESH_TARGET_BASE + CAtaSmart::MAX_DISK + 1;
	subSubMenuInfo.dwTypeData = (LPWSTR)cstr.GetString();
	subSubMenu.InsertMenuItem(-1, &subSubMenuInfo);

	subSubMenu.AppendMenu(MF_SEPARATOR);
	for(int i = 0; i < m_Ata.vars.GetCount(); i++)
	{
		CString cstr;
		cstr.Format(_T("(%d) %s %.1f GB"), i + 1, m_Ata.vars[i].Model, m_Ata.vars[i].TotalDiskSize / 1000.0);
		subSubMenuInfo.wID = AUTO_REFRESH_TARGET_BASE + i;
		subSubMenuInfo.dwTypeData = (LPWSTR)cstr.GetString();
		if(m_FlagAutoRefreshTarget[i])
		{
			subSubMenuInfo.fState = MFS_CHECKED;
		}
		else
		{
			subSubMenuInfo.fState = MFS_UNCHECKED;
		}

		subSubMenu.InsertMenuItem(-1, &subSubMenuInfo);

		if(i % 8 == 7 && i + 1 != m_Ata.vars.GetCount())
		{
			subSubMenu.AppendMenu(MF_SEPARATOR);
		}
	}
	subSubMenu.Detach();

	SetMenu(menu);
	DrawMenuBar();

	m_LabelFirmware = i18n(_T("Dialog"), _T("FIRMWARE"));
	m_LabelSerialNumber = i18n(_T("Dialog"), _T("SERIAL_NUMBER"));
	m_LabelTemperature = i18n(_T("Dialog"), _T("TEMPERATURE"));
	m_LabelPowerOnHours = i18n(_T("Dialog"), _T("POWER_ON_HOURS"));
	m_LabelPowerOnCount = i18n(_T("Dialog"), _T("POWER_ON_COUNT"));
	m_LabelFeature = i18n(_T("Dialog"), _T("FEATURE"));
	m_LabelBufferSize = i18n(_T("Dialog"), _T("BUFFER_SIZE"));
	m_LabelDriveMap = i18n(_T("Dialog"), _T("DRIVE_LETTER"));
	m_LabelInterface = i18n(_T("Dialog"), _T("INTERFACE"));
	m_LabelTransferMode = i18n(_T("Dialog"), _T("TRANSFER_MODE"));
	m_LabelAtaAtapi = i18n(_T("Dialog"), _T("STANDARD"));
	m_LabelDiskStatus = i18n(_T("Dialog"), _T("HEALTH_STATUS"));
	m_LabelSmartStatus = i18n(_T("Dialog"), _T("SMART_STATUS"));
	m_LabelNvCacheSize = i18n(_T("Dialog"), _T("NV_CACHE_SIZE"));
	m_LabelRotationRate = i18n(_T("Dialog"), _T("ROTATION_RATE"));

	UpdateData(FALSE);

	if(m_NowDetectingUnitPowerOnHours)
	{
		SetWindowTitle(i18n(_T("Message"), _T("DETECT_UNIT_POWER_ON_HOURS")));
	}

	ChangeDisk(m_SelectDisk);
	InitDriveList();
	UpdateToolTip();
	InitListCtrl();
	UpdateListCtrl(m_SelectDisk);

	if(m_FlagResident)
	{
		UpdateTrayTemperatureIcon(TRUE);
	}

	SetClientRect(m_SizeX, m_SizeY, 1);

	WritePrivateProfileString(_T("Setting"), _T("Language"), LangName, m_Ini);
}


HRESULT CDiskInfoDlg::OnPreDisk(IHTMLElement* /*pElement*/){SelectDrive(m_SelectDisk - 1);return S_FALSE;}
HRESULT CDiskInfoDlg::OnNextDisk(IHTMLElement* /*pElement*/){SelectDrive(m_SelectDisk + 1);return S_FALSE;}

HRESULT CDiskInfoDlg::OnDisk0(IHTMLElement* /*pElement*/){SelectDrive(0 + m_DriveMenuPage * 8);return S_FALSE;}
HRESULT CDiskInfoDlg::OnDisk1(IHTMLElement* /*pElement*/){SelectDrive(1 + m_DriveMenuPage * 8);return S_FALSE;}
HRESULT CDiskInfoDlg::OnDisk2(IHTMLElement* /*pElement*/){SelectDrive(2 + m_DriveMenuPage * 8);return S_FALSE;}
HRESULT CDiskInfoDlg::OnDisk3(IHTMLElement* /*pElement*/){SelectDrive(3 + m_DriveMenuPage * 8);return S_FALSE;}
HRESULT CDiskInfoDlg::OnDisk4(IHTMLElement* /*pElement*/){SelectDrive(4 + m_DriveMenuPage * 8);return S_FALSE;}
HRESULT CDiskInfoDlg::OnDisk5(IHTMLElement* /*pElement*/){SelectDrive(5 + m_DriveMenuPage * 8);return S_FALSE;}
HRESULT CDiskInfoDlg::OnDisk6(IHTMLElement* /*pElement*/){SelectDrive(6 + m_DriveMenuPage * 8);return S_FALSE;}
HRESULT CDiskInfoDlg::OnDisk7(IHTMLElement* /*pElement*/){SelectDrive(7 + m_DriveMenuPage * 8);return S_FALSE;}

void CDiskInfoDlg::SelectDrive(DWORD i)
{
	if(i >= (DWORD)m_Ata.vars.GetCount())
	{
		return ;
	}

	CWaitCursor wait;
	static int preFlagFahrenheit = -1;

	switch(m_Ata.UpdateSmartInfo(i))
	{
	case CAtaSmart::SMART_STATUS_MAJOR_CHANGE:
	case CAtaSmart::SMART_STATUS_MINOR_CHANGE:
		SaveSmartInfo(i);
		break;
	default:
		if(m_SelectDisk == i)
		{
			if(preFlagFahrenheit == m_FlagFahrenheit)
			{
				return ;
			}
			else
			{
				preFlagFahrenheit = m_FlagFahrenheit;
			}
		}
		break;
	}

	m_SelectDisk = i;
	m_DriveMenuPage = i / 8;
	ChangeDisk(i);
	UpdateListCtrl(i);
	InitDriveList();
	UpdateToolTip();

	CMenu *menu = GetMenu();	
	menu->CheckMenuRadioItem(SELECT_DISK_BASE, SELECT_DISK_BASE + (INT)m_Ata.vars.GetCount(),
								SELECT_DISK_BASE + m_SelectDisk, MF_BYCOMMAND);
	SetMenu(menu);
	DrawMenuBar();
	CheckPage();

	if(m_FlagResident)
	{
		UpdateTrayTemperatureIcon(FALSE);
	}
}

void CDiskInfoDlg::CheckPage()
{
	VARIANT dummy;
	VariantInit(&dummy);
	dummy.vt = VT_BSTR;
	CString cstr;
	if(m_Ata.vars.GetCount() > 4)
	{
		if(0 < m_SelectDisk)
		{
			cstr.Format(_T("visible"));
			SetElementPropertyEx(_T("DivPreDisk"), DISPID_IHTMLELEMENT_CLASSNAME, &dummy, cstr);
		}
		else
		{
			cstr.Format(_T("hidden"));
			SetElementPropertyEx(_T("DivPreDisk"), DISPID_IHTMLELEMENT_CLASSNAME, &dummy, cstr);
		}

		if(m_SelectDisk < (DWORD)m_Ata.vars.GetCount() - 1)
		{
			cstr.Format(_T("visible"));
			SetElementPropertyEx(_T("DivNextDisk"), DISPID_IHTMLELEMENT_CLASSNAME, &dummy, cstr);
		}
		else
		{
			cstr.Format(_T("hidden"));
			SetElementPropertyEx(_T("DivNextDisk"), DISPID_IHTMLELEMENT_CLASSNAME, &dummy, cstr);
		}
	}
	else
	{
		cstr.Format(_T("hidden"));
		SetElementPropertyEx(_T("DivPreDisk"), DISPID_IHTMLELEMENT_CLASSNAME, &dummy, cstr);
		SetElementPropertyEx(_T("DivNextDisk"), DISPID_IHTMLELEMENT_CLASSNAME, &dummy, cstr);
	}
//	UpdateData(FALSE);
}

void CDiskInfoDlg::SaveSmartInfo(DWORD i)
{
	static CTime preTime[CAtaSmart::MAX_DISK] = {0};
	CTime time = CTime::GetTickCount();

	if(time < preTime[i] + SAVE_SMART_PERIOD)
	{
		return ;
	}
	else
	{
		preTime[i] = CTime::GetTickCount();
	}

	CString line;
	CString cstr;
	CStdioFile outFile;
	CString dir;
	CString disk;
	CString path;
	BOOL flagFirst = FALSE;
	TCHAR str[256];

	dir = m_SmartDir;
	CreateDirectory(dir, NULL);

	disk = m_Ata.vars[i].ModelSerial;
	dir += disk;
	CreateDirectory(dir, NULL);

	AlarmHealthStatus(i, dir, disk);

	GetPrivateProfileString(disk, _T("Date"), _T(""), str, 256, dir + _T("\\") + SMART_INI);
	cstr = str;
	if(cstr.IsEmpty())
	{
		flagFirst = TRUE;
		_stprintf_s(str, 256, _T("%s"), time.Format(_T("%Y/%m/%d %H:%M:%S")));
		WritePrivateProfileString(disk + _T("FIRST"), _T("Date"), str, dir + _T("\\") + SMART_INI);

		_stprintf_s(str, 256, _T("%d"), m_Ata.vars[i].DiskStatus);
		WritePrivateProfileString(disk + _T("FIRST"), _T("HealthStatus"), str, dir + _T("\\") + SMART_INI);
	}

	// Check Threshold of Reallocated Sectors Count
	GetPrivateProfileString(disk + _T("THRESHOLD"), _T("05"), _T(""), str, 256, dir + _T("\\") + SMART_INI);
	cstr = str;
	if(cstr.IsEmpty())
	{
		flagFirst = TRUE;
	}

	_stprintf_s(str, 256, _T("%s"), time.Format(_T("%Y/%m/%d %H:%M:%S")));
	WritePrivateProfileString(disk, _T("Date"), str, dir + _T("\\") + SMART_INI);
	
	_stprintf_s(str, 256, _T("%d"), m_Ata.vars[i].DiskStatus);
	WritePrivateProfileString(disk, _T("HealthStatus"), str, dir + _T("\\") + SMART_INI);

	if(m_Ata.vars[i].Temperature > 0)
	{
		AppendLog(dir, disk, _T("Temperature"), time, m_Ata.vars[i].Temperature, flagFirst);
	}

	if(m_Ata.vars[i].MeasuredPowerOnHours > 0 && m_NowDetectingUnitPowerOnHours == FALSE)
	{
		AppendLog(dir, disk, _T("PowerOnHours"), time, m_Ata.vars[i].MeasuredPowerOnHours, flagFirst);
	}

	if(m_Ata.vars[i].PowerOnCount > 0)
	{
		AppendLog(dir, disk, _T("PowerOnCount"), time, m_Ata.vars[i].PowerOnCount, flagFirst);
	}
	for(DWORD j = 0; j < m_Ata.vars[i].AttributeCount; j++)
	{
		cstr.Format(_T("%02X"), m_Ata.vars[i].Attribute[j].Id);
		AppendLog(dir, disk, cstr, time, m_Ata.vars[i].Attribute[j].CurrentValue, 
			flagFirst, m_Ata.vars[i].Threshold[j].ThresholdValue);

		switch(m_Ata.vars[i].Attribute[j].Id)
		{
		case 0x05: // Reallocated Sectors Count
			AppendLog(dir, disk, _T("ReallocatedSectorsCount"), time,
				MAKEWORD(m_Ata.vars[i].Attribute[j].RawValue[0], m_Ata.vars[i].Attribute[j].RawValue[1]), flagFirst);
			break;
		case 0xC4: // Reallocation Event Count
			AppendLog(dir, disk, _T("ReallocationEventCount"), time,
				MAKEWORD(m_Ata.vars[i].Attribute[j].RawValue[0], m_Ata.vars[i].Attribute[j].RawValue[1]), flagFirst);
			break;
		case 0xC5: // Current Pending Sector Count
			AppendLog(dir, disk, _T("CurrentPendingSectorCount"), time,
				MAKEWORD(m_Ata.vars[i].Attribute[j].RawValue[0], m_Ata.vars[i].Attribute[j].RawValue[1]), flagFirst);
			break;
		case 0xC6: // Off-Line Scan Uncorrectable Sector Count
			AppendLog(dir, disk, _T("UncorrectableSectorCount"), time,
				MAKEWORD(m_Ata.vars[i].Attribute[j].RawValue[0], m_Ata.vars[i].Attribute[j].RawValue[1]), flagFirst);
			break;
		default:
			break;
		}
	}
}

BOOL CDiskInfoDlg::AppendLog(CString dir, CString disk, CString file, CTime time, int value, BOOL flagFirst, int threshold)
{
	TCHAR str[256];

	// First Time
	if(flagFirst)
	{
		wsprintf(str, _T("%d"), value);
		WritePrivateProfileString(disk + _T("FIRST"), file, str, dir + _T("\\") + SMART_INI);

		if(file.GetLength() == 2)
		{
			wsprintf(str, _T("%d"), threshold);
			WritePrivateProfileString(disk + _T("THRESHOLD"), file, str, dir + _T("\\") + SMART_INI);
		}
	}

	GetPrivateProfileString(disk, file, _T("-1"), str, 256, dir + _T("\\") + SMART_INI);
	int pre = _tstoi(str);

	if(pre != value)
	{
		// Update
		wsprintf(str, _T("%d"), value);
		WritePrivateProfileString(disk, file, str, dir + _T("\\") + SMART_INI);

		CString line;
		line.Format(_T("%s,%d\n"), time.Format(_T("%Y/%m/%d %H:%M:%S")), value);

		CStdioFile outFile;
		if(outFile.Open(dir + _T("\\") + file + _T(".csv"),
			CFile::modeCreate | CFile::modeNoTruncate | CFile::modeReadWrite | CFile::typeText))
		{
			outFile.SeekToEnd();
			outFile.WriteString(line);
			outFile.Close();
			return TRUE;
		}
	}
	return FALSE;
}