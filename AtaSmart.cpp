/*---------------------------------------------------------------------------*/
//       Author : hiyohiyo
//         Mail : hiyohiyo@crystalmark.info
//          Web : http://crystalmark.info/
//      License : The modified BSD license
//
//                           Copyright 2008-2010 hiyohiyo. All rights reserved.
/*---------------------------------------------------------------------------*/
// Reference : http://www.usefullcode.net/2007/02/hddsmart.html (ja)
//

#include "stdafx.h"
#include "AtaSmart.h"
#include <wbemcli.h>

#include "DnpService.h"

#pragma comment(lib, "wbemuuid.lib")
#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }

static const TCHAR *commandTypeString[] = 
{
	_T("pd"),
	_T("sm"),
	_T("si"),
	_T("sa"),
	_T("sp"),
	_T("io"),
	_T("un"), // Logitec ??? 
	_T("jm"),
	_T("cy")
};

static const TCHAR *ssdVendorString[] = 
{
	_T(""),
	_T(""),
	_T("mt"), // MTron
	_T("ix"), // Indilinx
	_T("jm"), // JMicron
	_T("il"), // Intel
	_T("sg"), // SAMSUNG
	_T("sf"), // SandForce
	_T("mi"), // Micron
};

static const TCHAR *deviceFormFactorString[] = 
{
	_T(""),
	_T("5.25 inch"),
	_T("3.5 inch"),
	_T("2.5 inch"),
	_T("1.8 inch"),
	_T("< 1.8 inch")
};

CAtaSmart::CAtaSmart()
{
	BOOL bosVersionInfoEx;
	ZeroMemory(&m_Os, sizeof(OSVERSIONINFOEX));
	m_Os.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	if(!(bosVersionInfoEx = GetVersionEx((OSVERSIONINFO *)&m_Os)))
	{
		m_Os.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		GetVersionEx((OSVERSIONINFO *)&m_Os);
	}

	m_FlagAtaPassThrough = FALSE;
	m_FlagAtaPassThroughSmart = FALSE;
	if(m_Os.dwMajorVersion >= 6 || (m_Os.dwMajorVersion == 5 && m_Os.dwMinorVersion == 2))
	{
		m_FlagAtaPassThrough = TRUE;
		m_FlagAtaPassThroughSmart = TRUE;
	}
	else if(m_Os.dwMajorVersion == 5 && m_Os.dwMinorVersion == 1)
	{
		CString cstr;
		cstr = m_Os.szCSDVersion;
		cstr.Replace(_T("Service Pack "), _T(""));
		if(_tstoi(cstr) >= 2)
		{
			m_FlagAtaPassThrough = TRUE;
			m_FlagAtaPassThroughSmart = TRUE;
		}
	}
}

CAtaSmart::~CAtaSmart()
{
}

/* PUBLIC FUNCTION */
VOID CAtaSmart::SetAtaPassThroughSmart(BOOL flag)
{
	m_FlagAtaPassThroughSmart = flag;
}

/* PUBLIC FUNCTION */
DWORD CAtaSmart::UpdateSmartInfo(DWORD i)
{
	if(vars.GetCount() == 0)
	{
		return SMART_STATUS_NO_CHANGE;
	}

	static SMART_ATTRIBUTE attribute[MAX_DISK][MAX_ATTRIBUTE] = {0};

	if(vars[i].IsSmartEnabled && vars[i].IsSmartCorrect)
	{
		switch(vars[i].CommandType)
		{
		case CMD_TYPE_PHYSICAL_DRIVE:
			if(! GetSmartAttributePd(vars[i].PhysicalDriveId, vars[i].Target, &(vars[i])))
			{
				WakeUp(vars[i].PhysicalDriveId);
				if(! GetSmartAttributePd(vars[i].PhysicalDriveId, vars[i].Target, &(vars[i])))
				{
					return SMART_STATUS_NO_CHANGE;
				}
			}
			vars[i].DiskStatus = CheckDiskStatus(i);
			break;
		case CMD_TYPE_SCSI_MINIPORT:
			if(! GetSmartAttributeScsi(vars[i].ScsiPort, vars[i].ScsiTargetId, &(vars[i])))
			{
				return SMART_STATUS_NO_CHANGE;
			}
			vars[i].DiskStatus = CheckDiskStatus(i);
			break;
		case CMD_TYPE_SILICON_IMAGE:
			if(! GetSmartAttributeSi(vars[i].PhysicalDriveId, &(vars[i])))
			{
				return SMART_STATUS_NO_CHANGE;
			}
			vars[i].DiskStatus = CheckDiskStatus(i);
			break;
		case CMD_TYPE_SAT:
		case CMD_TYPE_SUNPLUS:
		case CMD_TYPE_IO_DATA:
		case CMD_TYPE_LOGITEC:
		case CMD_TYPE_JMICRON:
		case CMD_TYPE_CYPRESS:
			WakeUp(vars[i].PhysicalDriveId);
			if(! GetSmartAttributeSat(vars[i].PhysicalDriveId, vars[i].Target, &(vars[i])))
			{
				return SMART_STATUS_NO_CHANGE;
			}
			vars[i].DiskStatus = CheckDiskStatus(i);
			break;
		default:
			return SMART_STATUS_NO_CHANGE;
		}

		return CheckSmartAttributeUpdate(i, attribute[i], vars[i].Attribute);
	}

	return SMART_STATUS_NO_CHANGE;
}

/* PUBLIC FUNCTION */
BOOL CAtaSmart::UpdateIdInfo(DWORD i)
{
	BOOL flag = FALSE;
	switch(vars[i].CommandType)
	{
	case CMD_TYPE_PHYSICAL_DRIVE:
		flag =  DoIdentifyDevicePd(vars[i].PhysicalDriveId, vars[i].Target, &(vars[i].IdentifyDevice));
		break;
	case CMD_TYPE_SCSI_MINIPORT:
		flag =  DoIdentifyDeviceScsi(vars[i].ScsiPort, vars[i].ScsiTargetId, &(vars[i].IdentifyDevice));
		break;
	case CMD_TYPE_SILICON_IMAGE:
		flag =  DoIdentifyDeviceSi(vars[i].PhysicalDriveId, vars[i].ScsiPort, vars[i].ScsiBus, vars[i].SiliconImageType, &(vars[i].IdentifyDevice));
		break;		
	case CMD_TYPE_SAT:
	case CMD_TYPE_SUNPLUS:
	case CMD_TYPE_IO_DATA:
	case CMD_TYPE_LOGITEC:
	case CMD_TYPE_JMICRON:
	case CMD_TYPE_CYPRESS:
		flag = DoIdentifyDeviceSat(vars[i].PhysicalDriveId, vars[i].Target, &(vars[i].IdentifyDevice), vars[i].CommandType);
		break;
	default:
		return FALSE;
		break;
	}

	if(vars[i].Major >= 3 && vars[i].IdentifyDevice.CommandSetSupported2 & (1 << 3))
	{
		vars[i].IsApmSupported = TRUE;
		if(vars[i].IdentifyDevice.CommandSetEnabled2 & (1 << 3))
		{
			vars[i].IsApmEnabled = TRUE;
		}
		else
		{
			vars[i].IsApmEnabled = FALSE;
		}
	}
	if(vars[i].Major >= 5 && vars[i].IdentifyDevice.CommandSetSupported2 & (1 << 9))
	{
		vars[i].IsAamSupported = TRUE;
		if(vars[i].IdentifyDevice.CommandSetEnabled2 & (1 << 9))
		{
			vars[i].IsAamEnabled = TRUE;
		}
		else
		{
			vars[i].IsAamEnabled = FALSE;
		}
	}

	return flag;
}

/* PUBLIC FUNCTION */
BYTE CAtaSmart::GetAamValue(DWORD i)
{
	return LOBYTE(vars[i].IdentifyDevice.AcoustricManagement);
}

/* PUBLIC FUNCTION */
BYTE CAtaSmart::GetApmValue(DWORD i)
{
	return LOBYTE(vars[i].IdentifyDevice.CurrentPowerManagement);
}

/* PUBLIC FUNCTION */
BYTE CAtaSmart::GetRecommendAamValue(DWORD i)
{
	return HIBYTE(vars[i].IdentifyDevice.AcoustricManagement);
}

/* PUBLIC FUNCTION */
BYTE CAtaSmart::GetRecommendApmValue(DWORD i)
{
	return HIBYTE(vars[i].IdentifyDevice.CurrentPowerManagement);
}

/* PUBLIC FUNCTION */
BOOL CAtaSmart::EnableAam(DWORD i, BYTE param)
{
	return SendAtaCommand(i, 0xEF, 0x42, param);
}

/* PUBLIC FUNCTION */
BOOL CAtaSmart::DisableAam(DWORD i)
{
	return SendAtaCommand(i, 0xEF, 0xC2, 0);
}

/* PUBLIC FUNCTION */
BOOL CAtaSmart::EnableApm(DWORD i, BYTE param)
{
	return SendAtaCommand(i, 0xEF, 0x05, param);
}

/* PUBLIC FUNCTION */
BOOL CAtaSmart::DisableApm(DWORD i)
{
	return SendAtaCommand(i, 0xEF, 0x85, 0);
}

BOOL CAtaSmart::SendAtaCommand(DWORD i, BYTE main, BYTE sub, BYTE param)
{
	WakeUp(vars[i].PhysicalDriveId);
	switch(vars[i].CommandType)
	{
	case CMD_TYPE_PHYSICAL_DRIVE:
		return SendAtaCommandPd(vars[i].PhysicalDriveId, vars[i].Target, main, sub, param, NULL, 0);
		break;
	case CMD_TYPE_SCSI_MINIPORT:
		return SendAtaCommandScsi(vars[i].ScsiPort, vars[i].ScsiTargetId, main, sub, param);
		break;
	case CMD_TYPE_SILICON_IMAGE:
		return FALSE;
		break;
	case CMD_TYPE_SAT:
	case CMD_TYPE_SUNPLUS:
	case CMD_TYPE_IO_DATA:
	case CMD_TYPE_LOGITEC:
	case CMD_TYPE_JMICRON:
	case CMD_TYPE_CYPRESS:
		return SendAtaCommandSat(vars[i].PhysicalDriveId, vars[i].Target, main, sub, param, vars[i].CommandType);
		break;
	default:
		return FALSE;
		break;
	}
	return FALSE;
}

DWORD CAtaSmart::CheckSmartAttributeUpdate(DWORD index, SMART_ATTRIBUTE* pre, SMART_ATTRIBUTE* cur)
{
	if(! vars[index].IsSmartCorrect)
	{
		return SMART_STATUS_NO_CHANGE;
	}

	if(memcmp(pre, cur, sizeof(SMART_ATTRIBUTE) * MAX_ATTRIBUTE) == 0)
	{
		return SMART_STATUS_NO_CHANGE;
	}
	else
	{
		for(int i = 0; i < MAX_ATTRIBUTE; i++)
		{
			switch(cur[i].Id)
			{
			case 0x09: // Power on Hours
				{
					DWORD preRawValue = MAKELONG(
						MAKEWORD(pre[i].RawValue[0], pre[i].RawValue[1]),
						MAKEWORD(pre[i].RawValue[2], pre[i].RawValue[3])
						);
					DWORD curRawValue = MAKELONG(
						MAKEWORD(cur[i].RawValue[0], cur[i].RawValue[1]),
						MAKEWORD(cur[i].RawValue[2], cur[i].RawValue[3])
						);

					if(vars[index].DiskVendorId == SSD_VENDOR_INDILINX)
					{
						preRawValue = pre[i].WorstValue * 256 + pre[i].CurrentValue;
						curRawValue = cur[i].WorstValue * 256 + cur[i].CurrentValue;
					}

					if(GetPowerOnHours(preRawValue, vars[index].DetectedTimeUnitType)
					!= GetPowerOnHours(curRawValue, vars[index].DetectedTimeUnitType))
					{
						memcpy(pre, cur, sizeof(SMART_ATTRIBUTE) * MAX_ATTRIBUTE);
						return SMART_STATUS_MAJOR_CHANGE;
					}
					if(GetPowerOnHours(preRawValue, vars[index].MeasuredTimeUnitType)
					!= GetPowerOnHours(curRawValue, vars[index].MeasuredTimeUnitType))
					{
						memcpy(pre, cur, sizeof(SMART_ATTRIBUTE) * MAX_ATTRIBUTE);
						return SMART_STATUS_MAJOR_CHANGE;
					}
				}
				break;
			case 0x0C: // Power On Count
				{
					DWORD preRawValue = MAKELONG(
						MAKEWORD(pre[i].RawValue[0], pre[i].RawValue[1]),
						MAKEWORD(pre[i].RawValue[2], pre[i].RawValue[3])
						);
					DWORD curRawValue = MAKELONG(
						MAKEWORD(cur[i].RawValue[0], cur[i].RawValue[1]),
						MAKEWORD(cur[i].RawValue[2], cur[i].RawValue[3])
						);
					if(vars[index].DiskVendorId == SSD_VENDOR_INDILINX)
					{
						preRawValue = pre[i].WorstValue * 256 + pre[i].CurrentValue;
						curRawValue = cur[i].WorstValue * 256 + cur[i].CurrentValue;
					}

					if(preRawValue != curRawValue)
					{
						memcpy(pre, cur, sizeof(SMART_ATTRIBUTE) * MAX_ATTRIBUTE);
						return SMART_STATUS_MAJOR_CHANGE;
					}
				}
				break;
			case 0xBE: // 3.9.4 or later
			case 0xC2: // Temperature
				if(pre[i].RawValue[0] != cur[i].RawValue[0]
				|| pre[i].CurrentValue != cur[i].CurrentValue)
				{
					memcpy(pre, cur, sizeof(SMART_ATTRIBUTE) * MAX_ATTRIBUTE);
					return SMART_STATUS_MAJOR_CHANGE;
				}
				break;
			default:
				break;
			}
		}
		// 3.4.0 - 
		return SMART_STATUS_MAJOR_CHANGE;
		// return SMART_STATUS_MINOR_CHANGE;
	}
}

/* PUBLIC FUNCTION */
BOOL CAtaSmart::MeasuredTimeUnit()
{
	DWORD getTickCount = GetTickCount();
	if(getTickCount > MeasuredGetTickCount + 155000 || MeasuredGetTickCount + 125000 > getTickCount)
	{
		return FALSE;
	}

	for(int i = 0; i < vars.GetCount(); i++)
	{
		if(vars[i].PowerOnRawValue < 0)
		{
			continue;
		}
		UpdateSmartInfo(i);

		DWORD test = vars[i].PowerOnRawValue - vars[i].PowerOnStartRawValue;

		if(vars[i].DiskVendorId == SSD_VENDOR_INDILINX)
		{
			vars[i].MeasuredTimeUnitType = POWER_ON_HOURS;
		}
		else if(vars[i].Model.Find(_T("SAMSUNG")) == 0)
		{
			if(test >= 2)
			{
				vars[i].MeasuredTimeUnitType = POWER_ON_HALF_MINUTES;
			}
			else
			{
				vars[i].MeasuredTimeUnitType = POWER_ON_HOURS;
			}
		}
		else if(vars[i].Model.Find(_T("FUJITSU")) == 0)
		{
			if(test >= 6)
			{
				vars[i].MeasuredTimeUnitType = POWER_ON_SECONDS;
			}
			else if(test >= 4)
			{
				vars[i].MeasuredTimeUnitType = POWER_ON_HALF_MINUTES;
			}
			else if(test >= 2)
			{
				vars[i].MeasuredTimeUnitType = POWER_ON_MINUTES;
			}
			else
			{
				vars[i].MeasuredTimeUnitType = POWER_ON_HOURS;
			}
		}
		else if(vars[i].Model.Find(_T("MAXTOR")) == 0)
		{
			if(test >= 2)
			{
				vars[i].MeasuredTimeUnitType = POWER_ON_MINUTES;
				vars[i].IsMaxtorMinute = TRUE;
			}
			else
			{
				vars[i].MeasuredTimeUnitType = POWER_ON_HOURS;
				vars[i].IsMaxtorMinute = FALSE;
			}
		}
		else
		{
			if(test >= 2)
			{
				vars[i].MeasuredTimeUnitType = POWER_ON_MINUTES;
			}
			else
			{
				vars[i].MeasuredTimeUnitType = POWER_ON_HOURS;
			}
		}
	}

	return TRUE;	
}

/* PUBLIC FUNCTION */
VOID CAtaSmart::Init(BOOL useWmi, BOOL advancedDiskSearch, PBOOL flagChangeDisk, BOOL workaroundHD204UI)
{
	// Debug
	// useWmi = FALSE;

	IsAdvancedDiskSearch = advancedDiskSearch;
	IsEnabledWmi = FALSE;
	IsWorkaroundHD204UI = workaroundHD204UI;

	CArray<DISK_POSITION, DISK_POSITION> previous;
	
	if(flagChangeDisk != NULL)
	{
		*flagChangeDisk = FALSE;
		for(int i = 0; i < vars.GetCount(); i++)
		{
			DISK_POSITION dp;
			dp.PhysicalDriveId = vars[i].PhysicalDriveId;
			dp.ScsiTargetId = vars[i].ScsiTargetId;
			dp.ScsiPort = vars[i].ScsiPort;

			previous.Add(dp);
		}
	}

	// Init
	vars.RemoveAll();
	externals.RemoveAll();
	m_ControllerMap = _T("");
	m_BlackIdeController.RemoveAll();
	m_BlackScsiController.RemoveAll();
	m_SiliconImageController.RemoveAll();
	m_SiliconImageControllerType.RemoveAll();

	m_BlackPhysicalDrive.RemoveAll();

	if(useWmi)
	{
		HRESULT hRes = S_OK;
		ULONG   uReturned = 1;

		IWbemLocator*			pIWbemLocator = NULL;
		IWbemServices*			pIWbemServices = NULL;
		IEnumWbemClassObject*	pEnumCOMDevs = NULL;
		IEnumWbemClassObject*	pEnumCOMDevs2 = NULL;
		IWbemClassObject*		pCOMDev = NULL;
		DWORD driveLetterMap[256] = {0};
		
		DebugPrint(_T("CAtaSmart::Init WMI on - Start"));

		bool initWmi = true;
		CDnpService	cService;
	
		if(! cService.IsServiceRunning(_T("Winmgmt")))
		{
			DebugPrint(_T("Waiting... Winmgmt"));
			initWmi = cService.EasyStart(_T("Winmgmt"));
		}

		if(initWmi)
		{
			try
			{
			//	DebugPrint(_T("CoInitialize()"));
			//	CoInitialize(NULL);
				DebugPrint(_T("CoInitializeSecurity()"));
				CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_DEFAULT,
					RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL);
				DebugPrint(_T("CoCreateInstance()"));
				//CLSID_WbemAdministrativeLocator / 
				if(FAILED(CoCreateInstance(CLSID_WbemLocator, NULL, CLSCTX_INPROC_SERVER,
					IID_IWbemLocator, (LPVOID *)&pIWbemLocator)))
				{
				//	CoUninitialize();
					DebugPrint(_T("NG:WMI Init 1"));
				}
				else 
				{
					long securityFlag = 0;
					if( m_Os.dwMajorVersion >= 6 // Vista or later
				//	|| (m_Os.dwMajorVersion == 5 && m_Os.dwMinorVersion >= 1) // XP or later
					)
					{
						securityFlag = WBEM_FLAG_CONNECT_USE_MAX_WAIT;
					}

					DebugPrint(_T("ConnectServer()"));
					if(FAILED(pIWbemLocator->ConnectServer(SysAllocString(L"\\\\.\\root\\cimv2"), 
						NULL, NULL, 0L,
						securityFlag,
						NULL, NULL, &pIWbemServices)))
					{
					//	CoUninitialize();
						DebugPrint(_T("NG:WMI Init 2"));
					}
					else
					{
						DebugPrint(_T("CoSetProxyBlanket()"));
						hRes = CoSetProxyBlanket(pIWbemServices, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE,
							NULL, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);
						if(FAILED(hRes))
						{
						//	CoUninitialize();
							CString cstr;
							cstr.Format(_T("NG:WMI Init - %08X"), hRes);
							DebugPrint(cstr);
						}
						else
						{
							IsEnabledWmi = TRUE;
							DebugPrint(_T("OK:WMI Init"));
						}
					}
					SAFE_RELEASE(pIWbemLocator);
				}
			}
			catch(...)
			{
				DebugPrint(_T("EX:WMI Init"));
			}
		}
		else
		{
			DebugPrint(_T("NG:WMI Init 3"));
		}

		if(IsEnabledWmi)
		{
			CStringArray csa;
			CString temp, cstr, cstr1, cstr2;
			try
			{// Win32_IDEController
				hRes = pIWbemServices->ExecQuery(SysAllocString(L"WQL"),
					SysAllocString(L"select Name, DeviceID from Win32_IDEController"), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumCOMDevs);
				if(FAILED(hRes))
				{
					goto safeRelease;
				}

				while(pEnumCOMDevs && SUCCEEDED(pEnumCOMDevs->Next(10000, 1, &pCOMDev, &uReturned)) && uReturned == 1)
				{
					BOOL flagBlackList = FALSE;
					VARIANT  pVal;
					VariantInit(&pVal);
					CString name1, deviceId, channel;
					if(pCOMDev->Get(L"DeviceID", 0L, &pVal, NULL, NULL) == WBEM_S_NO_ERROR && pVal.vt > VT_NULL)
					{
						deviceId = pVal.bstrVal;
						if(deviceId.Find(_T("PCIIDE\\IDECHANNEL")) == 0)
						{
							channel = deviceId.Right(1);
						}
						deviceId.Replace(_T("\\"), _T("\\\\"));
						VariantClear(&pVal);
					}
					if(pCOMDev->Get(L"Name", 0L, &pVal, NULL, NULL) == WBEM_S_NO_ERROR && pVal.vt > VT_NULL)
					{
						name1 = pVal.bstrVal;
						if(! channel.IsEmpty())
						{
							name1 += _T(" (") + channel + _T(")");
						}
						m_IdeController.Add(name1);
						VariantClear(&pVal);

						// Black List
					//	if(name1.Find(_T("")) == 0)
					//	{
					//		flagBlackList = TRUE;
					//	}
					}
					SAFE_RELEASE(pCOMDev);

					if(cstr.Find(name1) == -1 || cstr.Find(name1 + _T(" [ATA]")) >= 0)
					{
						csa.Add(cstr);
						cstr = _T("%%%") + name1 + _T(" [ATA]");
						cstr += _T("\r\n");
					}

					CString mapping;
					mapping.Format(_T("ASSOCIATORS OF {Win32_IDEController.DeviceID=\"%s\"} WHERE AssocClass = Win32_IDEControllerDevice"), deviceId);
					pIWbemServices->ExecQuery(SysAllocString(L"WQL"), SysAllocString(mapping), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumCOMDevs2);
					while(pEnumCOMDevs2 && SUCCEEDED(pEnumCOMDevs2->Next(10000, 1, &pCOMDev, &uReturned)) && uReturned == 1)
					{
						VARIANT pVal;
						VariantInit(&pVal);
						CString name2, deviceId, channel;
						if(pCOMDev->Get(L"DeviceID", 0L, &pVal, NULL, NULL) == WBEM_S_NO_ERROR && pVal.vt > VT_NULL)
						{
							deviceId = pVal.bstrVal;
							if(deviceId.Find(_T("PCIIDE\\IDECHANNEL")) == 0)
							{
								channel = deviceId.Right(1);
							}
							else if(flagBlackList)
							{
								m_BlackIdeController.Add(deviceId);
							}
							VariantClear(&pVal);
						}
						if(pCOMDev->Get(L"Name", 0L, &pVal, NULL, NULL) == WBEM_S_NO_ERROR && pVal.vt > VT_NULL)
						{
							name2 = pVal.bstrVal;
							if(! channel.IsEmpty())
							{
								name2 += _T(" (") + channel + _T(")");
							}
							VariantClear(&pVal);
						}
						SAFE_RELEASE(pCOMDev);

						if(cstr.Find(_T("   - ") + name1) >= 0 || cstr.Find(_T("   + ") + name1) >= 0)
						{
							cstr1 = _T("- ") + name1;
							cstr2 = _T("+ ") + name1;
							cstr.Replace(cstr1, cstr2);

							cstr1 = name1 + _T("\r\n     - ") + name2;
							cstr.Replace(name1, cstr1);
						}
						else
						{
							cstr += _T("   - ") + name2 + _T("\r\n");
						}
						cstr.Replace(_T("%%%"), _T(" + "));
					}
					cstr.Replace(_T("%%%"), _T(" - "));
					SAFE_RELEASE(pEnumCOMDevs2);
				}
				csa.Add(cstr);
				SAFE_RELEASE(pEnumCOMDevs);
				DebugPrint(_T("OK:Win32_IDEController"));
			}
			catch(...)
			{
				DebugPrint(_T("EX:Win32_IDEController"));
			}

			try
			{
				cstr = _T("");
				hRes = pIWbemServices->ExecQuery(SysAllocString(L"WQL"),
					SysAllocString(L"select Name, DeviceID from Win32_SCSIController"), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumCOMDevs);
				if(FAILED(hRes))
				{
					goto safeRelease;
				}

				while(pEnumCOMDevs && SUCCEEDED(pEnumCOMDevs->Next(10000, 1, &pCOMDev, &uReturned)) && uReturned == 1)
				{
					BOOL flagBlackList = FALSE;
					BOOL flagSiliconImage = FALSE;
					DWORD siliconImageType = 0;

					VARIANT  pVal;
					VariantInit(&pVal);
					CString name1, deviceId;
					if(pCOMDev->Get(L"DeviceID", 0L, &pVal, NULL, NULL) == WBEM_S_NO_ERROR && pVal.vt > VT_NULL)
					{
						deviceId = pVal.bstrVal;
						deviceId.Replace(_T("\\"), _T("\\\\"));
						VariantClear(&pVal);
					}
					if(pCOMDev->Get(L"Name", 0L, &pVal, NULL, NULL) == WBEM_S_NO_ERROR && pVal.vt > VT_NULL)
					{
						name1 = pVal.bstrVal;
						m_ScsiController.Add(name1);
						VariantClear(&pVal);

						// Black List
						if(! IsAdvancedDiskSearch && name1.Find(_T("VIA VT6410")) == 0
						//|| name1.Find(_T("")) == 0
						)
						{
							flagBlackList = TRUE;
						}

						// Silicon Image Controller
						if(name1.Find(_T("Silicon Image SiI ")) == 0)
						{
							flagSiliconImage = TRUE;
							CString cstr;
							cstr = name1;
							cstr.Replace(_T("Silicon Image SiI "), _T(""));
							siliconImageType = _tstoi(cstr);
						}
					}
					SAFE_RELEASE(pCOMDev);
					if(cstr.Find(name1) == -1 || cstr.Find(name1 + _T(" [SCSI]")) >= 0)
					{
						csa.Add(cstr);
						cstr = _T("%%%") + name1 + _T(" [SCSI]");			
						cstr += _T("\r\n");
					}

					CString mapping;
					mapping.Format(_T("ASSOCIATORS OF {Win32_SCSIController.DeviceID=\"%s\"} WHERE AssocClass = Win32_SCSIControllerDevice"), deviceId);
					pIWbemServices->ExecQuery(SysAllocString(L"WQL"), SysAllocString(mapping), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumCOMDevs2);
					while(pEnumCOMDevs2 && SUCCEEDED(pEnumCOMDevs2->Next(10000, 1, &pCOMDev, &uReturned)) && uReturned == 1)
					{
						VARIANT pVal;
						VariantInit(&pVal);
						CString name2, deviceId;
						if(pCOMDev->Get(L"DeviceID", 0L, &pVal, NULL, NULL) == WBEM_S_NO_ERROR && pVal.vt > VT_NULL)
						{
							deviceId = pVal.bstrVal;
							VariantClear(&pVal);
							if(flagBlackList)
							{
								m_BlackScsiController.Add(deviceId);
							}

							if(flagSiliconImage)
							{
								m_SiliconImageController.Add(deviceId);
								m_SiliconImageControllerType.Add(siliconImageType);
							}
						}

						if(pCOMDev->Get(L"Name", 0L, &pVal, NULL, NULL) == WBEM_S_NO_ERROR && pVal.vt > VT_NULL)
						{
							name2 = pVal.bstrVal;
							VariantClear(&pVal);
						}
						SAFE_RELEASE(pCOMDev);

						if(cstr.Find(_T("   - ") + name1) >= 0 || cstr.Find(_T("   + ") + name1) >= 0)
						{
							cstr1 = _T("- ") + name1;
							cstr2 = _T("+ ") + name1;
							cstr.Replace(cstr1, cstr2);

							cstr1 = name1 + _T("\r\n     - ") + name2;
							cstr.Replace(name1, cstr1);
						}
						else
						{
							cstr += _T("   - ") + name2 + _T("\r\n");
						}
						cstr.Replace(_T("%%%"), _T(" + "));
					}
					cstr.Replace(_T("%%%"), _T(" - "));
					SAFE_RELEASE(pEnumCOMDevs2);
				}
				csa.Add(cstr);
				SAFE_RELEASE(pEnumCOMDevs);
				DebugPrint(_T("OK:Win32_SCSIController"));
			}
			catch(...)
			{
				DebugPrint(_T("EX:Win32_SCSIController"));
			}

			for(int i = 0; i < csa.GetCount(); i++)
			{
				m_ControllerMap += csa.GetAt(i);
			}

			try
			{// Win32_USBController
				hRes = pIWbemServices->ExecQuery(SysAllocString(L"WQL"),
					SysAllocString(L"select Name, DeviceID from Win32_USBController"), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumCOMDevs);
				if(FAILED(hRes))
				{
					goto safeRelease;
				}

				while(pEnumCOMDevs && SUCCEEDED(pEnumCOMDevs->Next(10000, 1, &pCOMDev, &uReturned)) && uReturned == 1)
				{
					VARIANT  pVal;
					VariantInit(&pVal);
					CString deviceId, channel;
					if(pCOMDev->Get(L"DeviceID", 0L, &pVal, NULL, NULL) == WBEM_S_NO_ERROR && pVal.vt > VT_NULL)
					{
						deviceId = pVal.bstrVal;
						deviceId.Replace(_T("\\"), _T("\\\\"));
						VariantClear(&pVal);
					}
					SAFE_RELEASE(pCOMDev);

					CString mapping, enclosure;
					mapping.Format(_T("ASSOCIATORS OF {Win32_USBController.DeviceID=\"%s\"} WHERE AssocClass = Win32_USBControllerDevice"), deviceId);
					pIWbemServices->ExecQuery(SysAllocString(L"WQL"), SysAllocString(mapping), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumCOMDevs2);
					while(pEnumCOMDevs2 && SUCCEEDED(pEnumCOMDevs2->Next(10000, 1, &pCOMDev, &uReturned)) && uReturned == 1)
					{
						VARIANT pVal;
						VariantInit(&pVal);
						if(pCOMDev->Get(L"DeviceID", 0L, &pVal, NULL, NULL) == WBEM_S_NO_ERROR && pVal.vt > VT_NULL)
						{
							cstr = pVal.bstrVal;
							VariantClear(&pVal);
							if(cstr.Find(_T("USBSTOR")) >= 0)
							{
								EXTERNAL_DISK_INFO edi = {0};
								int curPos= 0;
								CString resToken;
								resToken = deviceId.Tokenize(_T("\\&"), curPos);
								while(resToken != _T(""))
								{
									if(resToken.Replace(_T("VID_"), _T("")) > 0)
									{
										edi.UsbVendorId = _tcstol(resToken, NULL, 16);
									}
									else if(resToken.Replace(_T("PID_"), _T("")) > 0)
									{
										edi.UsbProductId = _tcstol(resToken, NULL, 16);
									}
									resToken = deviceId.Tokenize(_T("\\&"), curPos);
								};

								if(pCOMDev->Get(L"Name", 0L, &pVal, NULL, NULL) == WBEM_S_NO_ERROR && pVal.vt > VT_NULL)
								{
									edi.Enclosure = pVal.bstrVal;
									VariantClear(&pVal);
								}
								externals.Add(edi);
							}
							deviceId = cstr;
						}
						SAFE_RELEASE(pCOMDev);
					}
					SAFE_RELEASE(pEnumCOMDevs2);
				}
				SAFE_RELEASE(pEnumCOMDevs);
				DebugPrint(_T("OK:Win32_USBController"));

				for(int i = 0; i < externals.GetCount(); i++)
				{
					cstr.Format(_T("VID=%04Xh, PID=%04Xh"), externals[i].UsbVendorId, externals[i].UsbProductId);
					DebugPrint(cstr);
				}
			}
			catch(...)
			{
				DebugPrint(_T("EX:Win32_USBController"));
			}
/*
			try
			{// Win32_1394Controller
				pIWbemServices->ExecQuery(SysAllocString(L"WQL"),
					SysAllocString(L"select Name, DeviceID from Win32_1394Controller"), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumCOMDevs);
				while(pEnumCOMDevs && SUCCEEDED(pEnumCOMDevs->Next(10000, 1, &pCOMDev, &uReturned)) && uReturned == 1)
				{
					VARIANT  pVal;
					VariantInit(&pVal);
					CString deviceId, channel;
					if(pCOMDev->Get(L"DeviceID", 0L, &pVal, NULL, NULL) == WBEM_S_NO_ERROR && pVal.vt > VT_NULL)
					{
						deviceId = pVal.bstrVal;
						deviceId.Replace(_T("\\"), _T("\\\\"));
						VariantClear(&pVal);
					}
					SAFE_RELEASE(pCOMDev);

					CString mapping, enclosure;
					mapping.Format(_T("ASSOCIATORS OF {Win32_1394Controller.DeviceID=\"%s\"} WHERE AssocClass = Win32_1394ControllerDevice"), deviceId);
					pIWbemServices->ExecQuery(SysAllocString(L"WQL"), SysAllocString(mapping), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumCOMDevs2);
					while(pEnumCOMDevs2 && SUCCEEDED(pEnumCOMDevs2->Next(10000, 1, &pCOMDev, &uReturned)) && uReturned == 1)
					{
						VARIANT pVal;
						VariantInit(&pVal);
						if(pCOMDev->Get(L"DeviceID", 0L, &pVal, NULL, NULL) == WBEM_S_NO_ERROR && pVal.vt > VT_NULL)
						{
							deviceId = pVal.bstrVal;
							VariantClear(&pVal);
						}
						SAFE_RELEASE(pCOMDev);
					}
					SAFE_RELEASE(pEnumCOMDevs2);
				}
				SAFE_RELEASE(pEnumCOMDevs);
				DebugPrint(_T("OK:Win32_1394Controller"));
			}
			catch(...)
			{
				DebugPrint(_T("EX:Win32_1394Controller"));
			}
*/
			/* DEBUG
			for(int i = 0; i < externals.GetCount(); i++)
			{
				CString cstr;
				cstr.Format(_T("Enclosure=%s, VID=%04X, PID=%04X"),
					externals.GetAt(i).Enclosure,
					externals.GetAt(i).VendorId,
					externals.GetAt(i).ProductId);

				AfxMessageBox(cstr);
			}
			*/

			for(int i = 0; i < MAX_SEARCH_PHYSICAL_DRIVE; i++)
			{
				BOOL	bRet;
				HANDLE	hIoCtrl;
				DWORD	dwReturned;
				DISK_GEOMETRY dg = {0};
				CAtaSmart::INTERFACE_TYPE interfaceType = INTERFACE_TYPE_UNKNOWN;
				CAtaSmart::VENDOR_ID vendor = VENDOR_UNKNOWN;
							
				hIoCtrl = GetIoCtrlHandle(i);
				if(hIoCtrl == INVALID_HANDLE_VALUE)
				{
				///	DebugPrint(_T("INVALID_HANDLE_VALUE - continue"));
					continue;
				}
				///	DebugPrint(_T("DeviceIoControl"));
				bRet = ::DeviceIoControl(hIoCtrl, IOCTL_DISK_GET_DRIVE_GEOMETRY, 
					NULL, 0, &dg, sizeof(DISK_GEOMETRY),
					&dwReturned, NULL);
				if(dg.MediaType == FixedMedia)
				{
					CString cstr;
					cstr.Format(_T("WakeUp(%d)"), i);
					DebugPrint(cstr);
					WakeUp(i);
				}
				::CloseHandle(hIoCtrl);
			}

			try
			{
				DebugPrint(_T("DO:SELECT * FROM Win32_DiskDrive"));
				hRes = pIWbemServices->ExecQuery(SysAllocString(L"WQL"), 
					SysAllocString(L"SELECT * FROM Win32_DiskDrive"), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumCOMDevs);
				if(FAILED(hRes))
				{
					goto safeRelease;
				}
				DebugPrint(_T("OK1:SELECT * FROM Win32_DiskDrive"));

				if(! pEnumCOMDevs)
				{
					DebugPrint(_T("pEnumCOMDevs == NULL"));
					goto safeRelease;
				}

				while(SUCCEEDED(pEnumCOMDevs->Next(10000, 1, &pCOMDev, &uReturned)))
				{
					if(uReturned != 1)
					{
						DebugPrint(_T("uReturned != 1"));
						break;
					}
					DebugPrint(_T("while(pEnumCOMDevs ..."));
					CString mapping1, mapping2;
					CString model, deviceId, diskSize, mediaType, interfaceTypeWmi, pnpDeviceId, firmware;
					INT physicalDriveId = -1, scsiPort = -1, scsiTargetId = -1, scsiBus = -1;
					BOOL flagTarget = FALSE;
					BOOL flagBlackList = FALSE;
					DWORD siliconImageType = 0;
					
				try
				{
					VARIANT pVal;
					VariantInit(&pVal);
					if(pCOMDev->Get(L"Size", 0L, &pVal, NULL, NULL) == WBEM_S_NO_ERROR && pVal.vt > VT_NULL)
					{
						diskSize = pVal.bstrVal;
						DebugPrint(_T("diskSize:") + diskSize);
						VariantClear(&pVal);
					}

					if(pCOMDev->Get(L"DeviceID", 0L, &pVal, NULL, NULL) == WBEM_S_NO_ERROR && pVal.vt > VT_NULL)
					{
						deviceId = pVal.bstrVal;
						DebugPrint(_T("deviceId:") + deviceId);
						deviceId.Replace(_T("\\"), _T("\\\\"));
						if(_ttoi(deviceId.Right(2)) >= 10)
						{
							physicalDriveId = _ttoi(deviceId.Right(2));
						}
						else
						{
							physicalDriveId = _ttoi(deviceId.Right(1));
						}
						VariantClear(&pVal);
					}
					if(pCOMDev->Get(L"Model", 0L, &pVal, NULL, NULL) == WBEM_S_NO_ERROR && pVal.vt > VT_NULL)
					{
						model = pVal.bstrVal;
						DebugPrint(_T("model:") + model);
						VariantClear(&pVal);
					}
					if(pCOMDev->Get(L"FirmwareRevision", 0L, &pVal, NULL, NULL) == WBEM_S_NO_ERROR && pVal.vt > VT_NULL)
					{
						firmware = pVal.bstrVal;
						VariantClear(&pVal);
					}
					if(pCOMDev->Get(L"SCSIPort", 0L, &pVal, NULL, NULL) == WBEM_S_NO_ERROR && pVal.vt > VT_NULL)
					{
						scsiPort = pVal.intVal;
						VariantClear(&pVal);
					}			
					if(pCOMDev->Get(L"SCSITargetId", 0L, &pVal, NULL, NULL) == WBEM_S_NO_ERROR && pVal.vt > VT_NULL)
					{
						scsiTargetId = pVal.intVal;
						VariantClear(&pVal);
					}
					if(pCOMDev->Get(L"SCSIBus", 0L, &pVal, NULL, NULL) == WBEM_S_NO_ERROR && pVal.vt > VT_NULL)
					{
						scsiBus = pVal.intVal;
						VariantClear(&pVal);
					}
					if(pCOMDev->Get(L"MediaType", 0L, &pVal, NULL, NULL) == WBEM_S_NO_ERROR && pVal.vt > VT_NULL)
					{
						mediaType = pVal.bstrVal;
						DebugPrint(_T("mediaType:") + mediaType);
						mediaType.MakeLower();
						VariantClear(&pVal);

						if(mediaType.Find(_T("removable")) >= 0 || mediaType.IsEmpty())
						{
							flagTarget = FALSE;
						}
						else
						{
							flagTarget = TRUE;
						}
					}
					if(pCOMDev->Get(L"InterfaceType", 0L, &pVal, NULL, NULL) == WBEM_S_NO_ERROR && pVal.vt > VT_NULL)
					{
						interfaceTypeWmi = pVal.bstrVal;
						DebugPrint(_T("interfaceTypeWmi:") + interfaceTypeWmi);
						VariantClear(&pVal);
					}

					// Added 3.3.1 (Controller Black List Support)
					if(pCOMDev->Get(L"PNPDeviceID", 0L, &pVal, NULL, NULL) == WBEM_S_NO_ERROR && pVal.vt > VT_NULL)
					{
						pnpDeviceId = pVal.bstrVal;
						DebugPrint(_T("pnpDeviceId:") + pnpDeviceId);
						VariantClear(&pVal);

						// Is Silicon Image Controller
						for(int i = 0; i < m_SiliconImageController.GetCount(); i++)
						{
							if(m_SiliconImageController.GetAt(i).Find(pnpDeviceId) >= 0)
							{
								DebugPrint(_T("SiliconImageController:") + pnpDeviceId);
								siliconImageType = m_SiliconImageControllerType.GetAt(i);
							}
						}

						for(int i = 0; i < m_BlackIdeController.GetCount(); i++)
						{
							if(m_BlackIdeController.GetAt(i).Find(pnpDeviceId) >= 0)
							{
								DebugPrint(_T("BlackIdeController:") + pnpDeviceId);
								m_BlackPhysicalDrive.Add(physicalDriveId);
								flagBlackList = TRUE;
							}
						}
						for(int i = 0; i < m_BlackScsiController.GetCount(); i++)
						{
							if(m_BlackScsiController.GetAt(i).Find(pnpDeviceId) >= 0)
							{
								DebugPrint(_T("BlackScsiController:") + pnpDeviceId);
								flagBlackList = TRUE;
							}
						}
					}

					SAFE_RELEASE(pCOMDev);

					if(! flagBlackList)
					{
						// GetDiskInfo
						CString cstr;
						cstr.Format(_T("DO:GetDiskInfo pd=%d, sp=%d, st=%d, mt=%s"), physicalDriveId, scsiPort, scsiTargetId, mediaType);
						DebugPrint(cstr);

						INTERFACE_TYPE interfaceType = INTERFACE_TYPE_UNKNOWN;
						VENDOR_ID usbVendorId = VENDOR_UNKNOWN;
						DWORD usbProductId = 0;

						if(interfaceTypeWmi.Find(_T("1394")) >= 0 || model.Find(_T(" IEEE 1394 SBP2 Device")) > 0)
						{
							DebugPrint(_T("INTERFACE_TYPE_IEEE1394"));
							interfaceType = INTERFACE_TYPE_IEEE1394;
						}
						else if(interfaceTypeWmi.Find(_T("USB")) >= 0 || model.Find(_T(" USB Device")) > 0)
						{
							DebugPrint(_T("INTERFACE_TYPE_USB"));
							interfaceType = INTERFACE_TYPE_USB;
						}
						else
						{
							flagTarget = TRUE;
						}
						
						for(int i = 0; i < externals.GetCount(); i++)
						{
							if(model.Find(externals.GetAt(i).Enclosure) == 0)
							{
								usbVendorId = (VENDOR_ID)externals[i].UsbVendorId;
								usbProductId = externals[i].UsbProductId;
								cstr.Format(_T("usbVendorId=%04X, usbProductId=%04X"), usbVendorId, usbProductId);
								DebugPrint(cstr);
							}
						}

						if(IsAdvancedDiskSearch && mediaType.IsEmpty())
						{
							flagTarget = TRUE;
						}

						// [2010/12/05] Workaround for SAMSUNG HD204UI
						// http://sourceforge.net/apps/trac/smartmontools/wiki/SamsungF4EGBadBlocks
						if((model.Find(_T("SAMSUNG HD155UI")) == 0 || model.Find(_T("SAMSUNG HD204UI")) == 0) && firmware.Find(_T("1AQ10003")) != 0 && IsWorkaroundHD204UI)
						{
							flagTarget = FALSE;
						}

						DebugPrint(_T("flagTarget && GetDiskInfo"));
						if(flagTarget && GetDiskInfo(physicalDriveId, scsiPort, scsiTargetId, interfaceType, usbVendorId, usbProductId, scsiBus, siliconImageType))
						{
							DebugPrint(_T("int index = (int)vars.GetCount() - 1;"));
							int index = (int)vars.GetCount() - 1;
							if(! diskSize.IsEmpty())
							{
								DWORD totalDiskSize = (DWORD)(_ttoi64(diskSize) / 1000 / 1000 - 49);
								if(0 < vars[index].TotalDiskSize && vars[index].TotalDiskSize < 1000) // < 1GB
								{
								//	vars[index].TotalDiskSize == vars[index].DiskSizeChs;
								}
								else if(vars[index].TotalDiskSize < 10 * 1000) // < 10GB
								{
									vars[index].TotalDiskSize = totalDiskSize;
								}
								else if((totalDiskSize < vars[index].TotalDiskSize - 100
								|| vars[index].TotalDiskSize + 100 < totalDiskSize)
								&& totalDiskSize > vars[index].DiskSizeLba48
								)
								{
									vars[index].TotalDiskSize = totalDiskSize;
								}
							}

							BOOL flagSkipModelCheck = FALSE;

							vars[index].ModelWmi = model;
							// Model
							model.Replace(_T(" SCSI Disk Device"), _T(""));
							model.Replace(_T(" SATA Disk Device"), _T(""));
							model.Replace(_T(" ATA Disk Device"), _T(""));
							model.Replace(_T(" SCSI Device"), _T(""));
							model.Replace(_T(" SATA Device"), _T(""));
							model.Replace(_T(" ATA Device"), _T(""));

							if(model.Replace(_T(" IEEE 1394 SBP2 Device"), _T("")) > 0 || interfaceTypeWmi.Find(_T("1394")) >= 0)
							{
								flagSkipModelCheck = TRUE;
								cstr.Format(_T("IEEE 1394 (%s)"), vars[index].Interface);
								vars[index].Interface = cstr;
								vars[index].InterfaceType = INTERFACE_TYPE_IEEE1394;
								for(int i = 0; i < externals.GetCount(); i++)
								{
									if(externals.GetAt(i).Enclosure.Find(vars[index].ModelWmi) == 0)
									{
										vars[index].Enclosure = externals.GetAt(i).Enclosure;
										vars[index].UsbVendorId  = externals.GetAt(i).UsbVendorId;
										vars[index].UsbProductId = externals.GetAt(i).UsbProductId;
									}
								}
							}
							else if(model.Replace(_T(" USB Device"), _T("")) > 0 || interfaceTypeWmi.Find(_T("USB")) >= 0)
							{
								flagSkipModelCheck = TRUE;
								cstr.Format(_T("USB (%s)"), vars[index].Interface);
								vars[index].Interface = cstr;
								vars[index].InterfaceType = INTERFACE_TYPE_USB;

								for(int i = 0; i < externals.GetCount(); i++)
								{
									if(externals.GetAt(i).Enclosure.Find(vars[index].ModelWmi) == 0)
									{
										vars[index].Enclosure = externals.GetAt(i).Enclosure;
										vars[index].UsbVendorId  = externals.GetAt(i).UsbVendorId;
										vars[index].UsbProductId = externals.GetAt(i).UsbProductId;
									}
								}
							}

							CString cmp, cmp1, cmp2, cmp3;
							cmp = model;
							cmp.Replace(_T(" "), _T(""));
							cmp1 = cmp.Left(16);

							cmp = vars[index].Model;
							cmp.Replace(_T(" "), _T(""));
							cmp2 = cmp.Left(16);

							cmp = vars[index].ModelReverse;
							cmp.Replace(_T(" "), _T(""));
							cmp3 = cmp.Left(16);
							
							if(vars[index].Model.IsEmpty())
							{
								DebugPrint(_T("WmiModel: ") + model);
								DebugPrint(_T("SerialNumber: ") + vars[index].SerialNumber);
								DebugPrint(_T("vars.RemoveAt(index) - 1"));
								vars.RemoveAt(index);
							}
							else if(flagSkipModelCheck)
							{
								// None
							}
							else if(model.IsEmpty() || cmp1.Compare(cmp2) == 0)
							{
								// None
							}
							else if(cmp1.Compare(cmp3) == 0)
							{
								vars[index].SerialNumber = vars[index].SerialNumberReverse;
								vars[index].FirmwareRev = vars[index].FirmwareRevReverse;
								vars[index].Model = vars[index].ModelReverse;
								vars[index].ModelSerial = vars[index].Model + vars[index].SerialNumber;
								vars[index].ModelSerial.Replace(_T("/"), _T(""));
							}
							else
							{
								DebugPrint(_T("WmiModel: ") + model);
								DebugPrint(_T("Model: ") + vars[index].Model);
								DebugPrint(_T("SerialNumber: ") + vars[index].SerialNumber);
								DebugPrint(_T("vars.RemoveAt(index) - 2"));
								vars.RemoveAt(index);
							}

							// DEBUG
							// vars[index].VendorId = VENDOR_MTRON;
							DebugPrint(_T("OK:Check Model Name"));
						}
					}
				}
				catch(...)
				{
					DebugPrint(_T("EX:while(pEnumCOMDevs"));
				}
				}
				SAFE_RELEASE(pEnumCOMDevs);
				DebugPrint(_T("OK2:SELECT * FROM Win32_DiskDrive"));
			}
			catch(...)
			{
				DebugPrint(_T("EX:SELECT * FROM Win32_DiskDrive"));
			}

			// Drive Letter Mapping
			try
			{

				DebugPrint(_T("DO:SELECT * FROM Win32_DiskPartition"));
				hRes = pIWbemServices->ExecQuery(SysAllocString(L"WQL"), 
					SysAllocString(L"SELECT * FROM Win32_DiskPartition"), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumCOMDevs);
				if(FAILED(hRes))
				{
					DebugPrint(_T("NG:SELECT * FROM Win32_DiskPartition"));
					goto safeRelease;
				}
				DebugPrint(_T("OK:SELECT * FROM Win32_DiskPartition"));

				while(pEnumCOMDevs && SUCCEEDED(pEnumCOMDevs->Next(10000, 1, &pCOMDev, &uReturned)) && uReturned == 1)
				{
					DWORD physicalDriveId = 0;
					CString partition, drive, mapping, cstr;
					VARIANT pVal;
					VariantInit(&pVal);
					if(pCOMDev->Get(L"DeviceID", 0L, &pVal, NULL, NULL) == WBEM_S_NO_ERROR && pVal.vt > VT_NULL)
					{
						partition = pVal.bstrVal;
						VariantClear(&pVal);
					}
					cstr = partition;
					cstr.Replace(_T("Disk #"), _T(""));
					physicalDriveId = _ttoi(cstr);
					SAFE_RELEASE(pCOMDev);

					hRes = pIWbemServices->ExecQuery(SysAllocString(L"WQL"), 
						SysAllocString(L"SELECT * FROM Win32_LogicalDisk Where DriveType = 3"), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumCOMDevs2);
					if(FAILED(hRes))
					{
						DebugPrint(_T("NG:SELECT * FROM Win32_LogicalDisk Where DriveType = 3"));
						goto safeRelease;
					}

					while(pEnumCOMDevs2 && SUCCEEDED(pEnumCOMDevs2->Next(100000, 1, &pCOMDev, &uReturned)) && uReturned == 1)
					{
						VariantClear(&pVal);
						if(pCOMDev->Get(L"DeviceID", 0L, &pVal, NULL, NULL) == WBEM_S_NO_ERROR && pVal.vt > VT_NULL)
						{
							drive = pVal.bstrVal;
							VariantClear(&pVal);
						}
						SAFE_RELEASE(pCOMDev);

						IWbemContext *pCtx = 0;
						IWbemCallResult *pResult = 0;
						mapping.Format(_T("Win32_LogicalDiskToPartition.Antecedent=\"Win32_DiskPartition.DeviceID=\\\"%s\\\"\",Dependent=\"Win32_LogicalDisk.DeviceID=\\\"%s\\\"\""),
							partition, drive);
						
						BSTR bstr;
						bstr = mapping.AllocSysString();
						pIWbemServices->GetObject(bstr, 0, pCtx, &pCOMDev, &pResult);
						SysFreeString(bstr);
						if(pCOMDev)
						{
							driveLetterMap[physicalDriveId] |= 1 << (drive.GetAt(0) - 'A');
						}
						SAFE_RELEASE(pCOMDev);
					}
					SAFE_RELEASE(pEnumCOMDevs2);
				}
				SAFE_RELEASE(pEnumCOMDevs);

				DebugPrint(_T("OK:Drive Letter Mapping"));

			}
			catch(...)
			{
				DebugPrint(_T("EX:Drive Letter Mapping"));
			}

safeRelease:

			SAFE_RELEASE(pCOMDev);
			SAFE_RELEASE(pEnumCOMDevs);
			SAFE_RELEASE(pEnumCOMDevs2);
			SAFE_RELEASE(pIWbemServices);
		//	CoUninitialize();
		//  DebugPrint(_T("OK:CoUninitialize()"));
		}

		for(int i = 0; i < vars.GetCount(); i++)
		{
			CString driveLetter = _T("");
			for(int j = 0; j < 26; j++)
			{
				if(driveLetterMap[vars[i].PhysicalDriveId] & (1 << j))
				{
					CString cstr;
					cstr.Format(_T("%C"), j + 'A'); 
					driveLetter += cstr + _T(": ");
					vars[i].DriveLetterMap += (1 << j);
					DebugPrint(cstr);
				}
			}
			vars[i].DriveMap.Append(driveLetter);
			vars[i].DriveMap.TrimRight();
		}
	}
	else
	{
		DebugPrint(_T("CAtaSmart::Init WMI off - Start"));
	}
	
	// \\\\.\\PhysicalDrive%d
	for(int i = 0; i < MAX_SEARCH_PHYSICAL_DRIVE; i++)
	{


///		CString cstr;
///		cstr.Format(_T("Search Physical Drive : %d"), i);
///		DebugPrint(cstr);

		BOOL	flagChecked = FALSE;
		BOOL	bRet;
		HANDLE	hIoCtrl;
		DWORD	dwReturned;
		DISK_GEOMETRY dg;
		CAtaSmart::INTERFACE_TYPE interfaceType = INTERFACE_TYPE_UNKNOWN;
		CAtaSmart::VENDOR_ID vendor = VENDOR_UNKNOWN;

///		cstr.Format(_T("vars.GetCount() : %d"), vars.GetCount());
///		DebugPrint(cstr);

		for(int j = 0; j < vars.GetCount(); j++)
		{
			if(i == vars[j].PhysicalDriveId)
			{
				flagChecked = TRUE;
			}
		}

		for(int j = 0; j < m_BlackPhysicalDrive.GetCount(); j++)
		{
			if(i == m_BlackPhysicalDrive.GetAt(j))
			{
				flagChecked = TRUE;
			}
		}

		if(flagChecked)
		{
///			DebugPrint(_T("flagChecked - continue"));
			continue;
		}

///		DebugPrint(_T("GetIoCtrlHandle"));
		hIoCtrl = GetIoCtrlHandle(i);
		if(hIoCtrl == INVALID_HANDLE_VALUE)
		{
///			DebugPrint(_T("INVALID_HANDLE_VALUE - continue"));
			continue;
		}
///		DebugPrint(_T("DeviceIoControl"));
		bRet = ::DeviceIoControl(hIoCtrl, IOCTL_DISK_GET_DRIVE_GEOMETRY, 
			NULL, 0, &dg, sizeof(DISK_GEOMETRY),
			&dwReturned, NULL);
		if(bRet == FALSE || dwReturned != sizeof(DISK_GEOMETRY) || dg.MediaType != FixedMedia)
		{
///			DebugPrint(_T("CloseHandle - continue"));
			::CloseHandle(hIoCtrl);
			continue;
		}


		// [2010/12/05] Workaround for SAMSUNG HD204UI
		// http://sourceforge.net/apps/trac/smartmontools/wiki/SamsungF4EGBadBlocks

		DWORD	dwRet;
		DWORD	dwLen;
		BYTE*	pcbData;
		STORAGE_DEVICE_DESCRIPTOR*	pDescriptor;
		STORAGE_PROPERTY_QUERY		sQuery;
		CString model, firmware;

		dwLen = 4096;
		pcbData = new BYTE[dwLen];
		if(pcbData == NULL)
		{
			continue;
		}
		ZeroMemory(pcbData, dwLen);
		sQuery.PropertyId	= StorageDeviceProperty;
		sQuery.QueryType	= PropertyStandardQuery;
		sQuery.AdditionalParameters[0] = NULL;
		
		bRet = ::DeviceIoControl(hIoCtrl, IOCTL_STORAGE_QUERY_PROPERTY, &sQuery, sizeof(STORAGE_PROPERTY_QUERY), pcbData,dwLen,&dwRet,NULL);
		if(bRet == FALSE)
		{
			delete	pcbData;
			::CloseHandle(hIoCtrl);
			continue;
		}

		pDescriptor = (STORAGE_DEVICE_DESCRIPTOR*)pcbData;
		if(pDescriptor->ProductIdOffset)
		{
			model	= (char*)pDescriptor + pDescriptor->ProductIdOffset;
		}
		if(pDescriptor->ProductRevisionOffset)
		{
			firmware	= (char*)pDescriptor + pDescriptor->ProductRevisionOffset;
		}

		delete	pcbData;

		if((model.Find(_T("SAMSUNG HD155UI")) == 0 || model.Find(_T("SAMSUNG HD204UI")) == 0) && firmware.Find(_T("1AQ10003")) != 0 && IsWorkaroundHD204UI)
		{
			continue;
		}

		// [2010/12/05] Workaround for SAMSUNG HD204UI
		// http://sourceforge.net/apps/trac/smartmontools/wiki/SamsungF4EGBadBlocks
		
		// USB-HDD Check
		if(! IsEnabledWmi)
		{
			DebugPrint(_T("USB-HDD Check"));

			BYTE cbData[4096];
			ZeroMemory(cbData, 4096);

			STORAGE_DEVICE_DESCRIPTOR*	pDescriptor;
			STORAGE_PROPERTY_QUERY		sQuery;
			DWORD	dwRet;

			sQuery.PropertyId	= StorageDeviceProperty;
			sQuery.QueryType	= PropertyStandardQuery;
			sQuery.AdditionalParameters[0] = NULL;

///			DebugPrint(_T("DeviceIoControl"));
			bRet = ::DeviceIoControl(hIoCtrl, IOCTL_STORAGE_QUERY_PROPERTY, &sQuery,
				sizeof(STORAGE_PROPERTY_QUERY), &cbData, 4096, &dwRet, NULL);

			if(bRet != FALSE)
			{
				DebugPrint(_T("Check Bus Type"));
				pDescriptor = (STORAGE_DEVICE_DESCRIPTOR*)&cbData;
				if(pDescriptor->BusType == BusTypeUsb)
				{
					DebugPrint(_T("Bus Type = USB"));
					interfaceType = INTERFACE_TYPE_USB;
					vendor = USB_VENDOR_ALL;
				}
			}
		}
///		DebugPrint(_T("CloseHandle"));
		::CloseHandle(hIoCtrl);

		DebugPrint(_T("DO:GetDiskInfo"));
		if(GetDiskInfo(i, -1, -1, interfaceType, vendor))
		{
			DebugPrint(_T("OK:GetDiskInfo"));
			int index = (int)vars.GetCount() - 1;
			CString cmp;

///			cstr.Format(_T("index: %d"), index);
			cmp = vars[index].Model;
			DebugPrint(_T("Check Reverse"));
			if(cmp.Find(_T("DW C")) == 0 // WDC 
			|| cmp.Find(_T("iHat")) == 0 // Hitachi
			|| cmp.Find(_T("ASSM")) == 0 // SAMSUNG
			|| cmp.Find(_T("aMtx")) == 0 // Maxtor
			|| cmp.Find(_T("OTHS")) == 0 // TOSHIBA
			|| cmp.Find(_T("UFIJ")) == 0 // FUJITSU
			)	
			{
				DebugPrint(_T("Reverse"));
				vars[index].SerialNumber = vars[index].SerialNumberReverse;
				vars[index].FirmwareRev = vars[index].FirmwareRevReverse;
				vars[index].Model = vars[index].ModelReverse;
				vars[index].ModelSerial = vars[index].Model + vars[index].SerialNumber;
				vars[index].ModelSerial.Replace(_T("/"), _T(""));
			}
		}
	}

	DebugPrint(_T("OK:GetDiskInfo - PhysicalDrive"));
	// Sort
	ATA_SMART_INFO* p = vars.GetData();
	qsort(p, vars.GetCount(), sizeof(ATA_SMART_INFO), Compare);

	DebugPrint(_T("OK:qsort"));

	// Advanced Disk Search
	if(IsAdvancedDiskSearch)
	{
		// \\\\.\\Scsi%d:
		for(int i = 0; i < MAX_SEARCH_SCSI_PORT; i++)
		{
			for(int j = 0; j < MAX_SEARCH_SCSI_TARGET_ID; j++)
			{
				if(GetDiskInfo(-1, i, j, INTERFACE_TYPE_UNKNOWN, VENDOR_UNKNOWN))
				{
					int index = (int)vars.GetCount() - 1;
					CString cmp;
					cmp = vars[index].Model;
					if(cmp.Find(_T("DW C")) == 0 // WDC 
					|| cmp.Find(_T("iHat")) == 0 // Hitachi
					|| cmp.Find(_T("ASSM")) == 0 // SAMSUNG
					|| cmp.Find(_T("aMtx")) == 0 // Maxtor
					|| cmp.Find(_T("OTHS")) == 0 // TOSHIBA
					|| cmp.Find(_T("UFIJ")) == 0 // FUJITSU
					)	
					{
						vars[index].SerialNumber = vars[index].SerialNumberReverse;
						vars[index].FirmwareRev = vars[index].FirmwareRevReverse;
						vars[index].Model = vars[index].ModelReverse;
						vars[index].ModelSerial = vars[index].Model + vars[index].SerialNumber;
						vars[index].ModelSerial.Replace(_T("/"), _T(""));
					}
				}
			}
		}
		DebugPrint(_T("OK:GetDiskInfo - Scsi"));
	}

	MeasuredGetTickCount = GetTickCount();
	DebugPrint(_T("CAtaSmart::Init - Complete"));

	if(flagChangeDisk != NULL)
	{
		if(vars.GetCount() != previous.GetCount())
		{
			*flagChangeDisk = TRUE;
		}
		else
		{
			for(int i = 0; i < vars.GetCount(); i++)
			{
				if(vars.GetAt(i).PhysicalDriveId != previous.GetAt(i).PhysicalDriveId
				|| vars.GetAt(i).ScsiTargetId != previous.GetAt(i).ScsiTargetId
				|| vars.GetAt(i).ScsiPort != previous.GetAt(i).ScsiPort
				)
				{
					*flagChangeDisk = TRUE;
					break;
				}
			}
		}
	}
}

int CAtaSmart::Compare(const void *p1, const void *p2)
{
	return ((ATA_SMART_INFO*)p1)->PhysicalDriveId - ((ATA_SMART_INFO*)p2)->PhysicalDriveId;
}

BOOL CAtaSmart::AddDisk(INT physicalDriveId, INT scsiPort, INT scsiTargetId, INT scsiBus, BYTE target, COMMAND_TYPE commandType, IDENTIFY_DEVICE* identify, INT siliconImageType)
{
	if(vars.GetCount() >= MAX_DISK)
	{
		return FALSE;
	}

	ATA_SMART_INFO asi = {0};
	ATA_SMART_INFO asiCheck = {0};

	memcpy(&(asi.IdentifyDevice), identify, sizeof(IDENTIFY_DEVICE));
	asi.PhysicalDriveId = physicalDriveId;
	asi.ScsiBus = scsiBus;
	asi.ScsiPort =  scsiPort;
	asi.ScsiTargetId = scsiTargetId;
	asi.SiliconImageType = siliconImageType;
	asi.CommandType = commandType;
	asiCheck.CommandType = commandType;
	asi.SsdVendorString = _T("");

	if(commandType == CMD_TYPE_PHYSICAL_DRIVE || CMD_TYPE_SAT <= commandType && commandType <= CMD_TYPE_PROLIFIC)
	{
		if(target == 0xB0)
		{
			asi.CommandTypeString.Format(_T("%s2"), commandTypeString[commandType]);
		}
		else
		{
			asi.CommandTypeString.Format(_T("%s1"), commandTypeString[commandType]);
		}
	}
	else
	{
		asi.CommandTypeString = commandTypeString[commandType];
	}

	for(int i = 0; i < MAX_ATTRIBUTE; i++)
	{
		::ZeroMemory(&(asi.Attribute[i]), sizeof(SMART_ATTRIBUTE));
		::ZeroMemory(&(asi.Threshold[i]), sizeof(SMART_THRESHOLD));
	}

	for(int i = 0; i < 512; i++)
	{
		asi.SmartReadData[i] = 0x00;
		asi.SmartReadThreshold[i] = 0x00;
	}

	asi.IsSmartEnabled = FALSE;
	asi.IsIdInfoIncorrect = FALSE;
	asi.IsSmartCorrect = FALSE;
	asi.IsThresholdCorrect = FALSE;
	asi.IsWord88 = FALSE;
	asi.IsWord64_76 = FALSE;
	asi.IsCheckSumError = FALSE;
	asi.IsRawValues8 = FALSE;
	asi.IsRawValues7 = FALSE;
	asi.Is9126MB = FALSE;
	asi.IsThresholdBug = FALSE;

	asi.IsSmartSupported = FALSE;
	asi.IsLba48Supported = FALSE;
	asi.IsNcqSupported = FALSE;
	asi.IsAamSupported = FALSE;
	asi.IsApmSupported = FALSE;
	asi.IsAamEnabled = FALSE;
	asi.IsApmEnabled = FALSE;
	asi.IsNvCacheSupported = FALSE;
	asi.IsMaxtorMinute = FALSE;
	asi.IsSsd = FALSE;
	asi.IsTrimSupported = FALSE;
	
	asi.TotalDiskSize = 0;
	asi.Cylinder = 0;
	asi.Head = 0;
	asi.Sector = 0;
	asi.Sector28 = 0;
	asi.Sector48 = 0;
	asi.NumberOfSectors = 0;
	asi.DiskSizeChs = 0;
	asi.DiskSizeLba28 = 0;
	asi.DiskSizeLba48 = 0;
	asi.BufferSize = 0;
	asi.NvCacheSize = 0;
	asi.TransferModeType = 0;
	asi.DetectedTimeUnitType = 0;
	asi.MeasuredTimeUnitType = 0;
	asi.AttributeCount = 0;
	asi.DetectedPowerOnHours = -1;
	asi.MeasuredPowerOnHours = -1;
	asi.PowerOnRawValue = -1;
	asi.PowerOnStartRawValue = -1;
	asi.PowerOnCount = 0;
	asi.Temperature = 0;
	asi.NominalMediaRotationRate = 0;
//	asi.Speed = 0.0;
	asi.Life = -1;
	asi.HostWrites = 0;
	asi.HostReads = 0;
	asi.GBytesErased = 0;

	asi.Major = 0;
	asi.Minor = 0;

	asi.DiskStatus = 0;
	asi.DriveLetterMap = 0;

	asi.AlarmTemperature = 0;

	asi.InterfaceType = INTERFACE_TYPE_UNKNOWN;

	asi.DiskVendorId = VENDOR_UNKNOWN;
	asi.UsbVendorId = VENDOR_UNKNOWN;
	asi.UsbProductId = 0;
	asi.Target = target;

	asi.SerialNumber = _T("");
	asi.FirmwareRev = _T("");
	asi.Model = _T("");
	asi.ModelReverse = _T("");
	asi.ModelWmi = _T("");
	asi.ModelSerial = _T("");
	asi.DriveMap = _T("");
	asi.MaxTransferMode = _T("");
	asi.CurrentTransferMode = _T("");
	asi.MajorVersion = _T("");
	asi.MinorVersion = _T("");
	asi.Interface = _T("");
	asi.Enclosure = _T("");
	asi.DeviceNominalFormFactor = _T("");
	asi.MinorVersion = _T("");

	CHAR buf[64];

	// Check Sum Error
	BYTE sum = 0;
	BYTE checkSum[IDENTIFY_BUFFER_SIZE];
	memcpy(checkSum, (void *)identify, IDENTIFY_BUFFER_SIZE);
	for(int j = 0; j < IDENTIFY_BUFFER_SIZE; j++)
	{
		sum += checkSum[j];
	}
	if(sum != 0)
	{
		asi.IsCheckSumError = TRUE;
	}
	
	if(CheckAsciiStringError(identify->SerialNumber, sizeof(identify->SerialNumber))
	|| CheckAsciiStringError(identify->FirmwareRev, sizeof(identify->FirmwareRev))
	|| CheckAsciiStringError(identify->Model, sizeof(identify->Model)))
	{
		asi.IsIdInfoIncorrect = TRUE;
	//	DebugPrint(_T("CheckAsciiStringError"));
		return FALSE;
	}

	// Reverse
	strncpy_s(buf, 64, identify->SerialNumber, sizeof(identify->SerialNumber));
	asi.SerialNumberReverse = buf;
	asi.SerialNumberReverse.TrimLeft();
	asi.SerialNumberReverse.TrimRight();
	strncpy_s(buf, 64, identify->FirmwareRev, sizeof(identify->FirmwareRev));
	asi.FirmwareRevReverse = buf;
	asi.FirmwareRevReverse.TrimLeft();
	asi.FirmwareRevReverse.TrimRight();
	strncpy_s(buf, 64, identify->Model, sizeof(identify->Model));
	asi.ModelReverse = buf;
	asi.ModelReverse.TrimLeft();
	asi.ModelReverse.TrimRight();

	ChangeByteOrder(identify->SerialNumber, sizeof(identify->SerialNumber));
	ChangeByteOrder(identify->FirmwareRev, sizeof(identify->FirmwareRev));
	ChangeByteOrder(identify->Model, sizeof(identify->Model));

	// Normal
	strncpy_s(buf, 64, identify->SerialNumber, sizeof(identify->SerialNumber));
	asi.SerialNumber = buf;
	asi.SerialNumber.TrimLeft();
	asi.SerialNumber.TrimRight();
	strncpy_s(buf, 64, identify->FirmwareRev, sizeof(identify->FirmwareRev));
	asi.FirmwareRev = buf;
	asi.FirmwareRev.TrimLeft();
	asi.FirmwareRev.TrimRight();
	strncpy_s(buf, 64, identify->Model, sizeof(identify->Model));
	asi.Model = buf;
	asi.Model.TrimLeft();
	asi.Model.TrimRight();

	if(asi.Model.IsEmpty() || asi.FirmwareRev.IsEmpty())
	{
	//	DebugPrint(_T("asi.Model.IsEmpty() || asi.FirmwareRev.IsEmpty()"));
		asi.IsIdInfoIncorrect = TRUE;
		return FALSE;
	}

// DEBUG
//	asi.Model = _T(" MTRON ") + asi.Model; 

	asi.ModelSerial = asi.Model + asi.SerialNumber;
	asi.ModelSerial.Replace(_T("/"), _T(""));

	asi.Major = GetAtaMajorVersion(identify->MajorVersion, asi.MajorVersion);
	GetAtaMinorVersion(identify->MinorVersion, asi.MinorVersion);
	asi.TransferModeType = GetTransferMode(identify->MultiWordDma, identify->SerialAtaCapabilities,
						identify->UltraDmaMode, asi.CurrentTransferMode, asi.MaxTransferMode,
						asi.Interface, &asi.InterfaceType);
	asi.DetectedTimeUnitType = GetTimeUnitType(asi.Model, asi.FirmwareRev, asi.Major, asi.TransferModeType);

	// Feature
	if(asi.Major >= 3 && asi.IdentifyDevice.CommandSetSupported1 & (1 << 0))
	{
		asi.IsSmartSupported = TRUE;
	}
	if(asi.Major >= 3 && asi.IdentifyDevice.CommandSetSupported2 & (1 << 3))
	{
		asi.IsApmSupported = TRUE;
		if(asi.IdentifyDevice.CommandSetEnabled2 & (1 << 3))
		{
			asi.IsApmEnabled = TRUE;
		}
	}
	if(asi.Major >= 5 && asi.IdentifyDevice.CommandSetSupported2 & (1 << 9))
	{
		asi.IsAamSupported = TRUE;
		if(asi.IdentifyDevice.CommandSetEnabled2 & (1 << 9))
		{
			asi.IsAamEnabled = TRUE;
		}
	}

	if(asi.Major >= 5 && asi.IdentifyDevice.CommandSetSupported2 & (1 << 10))
	{
		asi.IsLba48Supported = TRUE;
	}

	if(asi.Major >= 6 && asi.IdentifyDevice.SerialAtaCapabilities & (1 << 8))
	{
		asi.IsNcqSupported = TRUE;
	}
	if(asi.Major >= 7 && asi.IdentifyDevice.NvCacheCapabilities & (1 << 0))
	{
		asi.IsNvCacheSupported = TRUE;
	}

	if(asi.Major >= 7 && asi.IdentifyDevice.DataSetManagement & (1 << 0))
	{
		asi.IsTrimSupported = TRUE;
	}

	// http://ascii.jp/elem/000/000/203/203345/img.html
	// "NominalMediaRotationRate" is supported by ATA8-ACS but a part of ATA/ATAPI-7 devices support this field.
	if(asi.Major >= 7 && asi.IdentifyDevice.NominalMediaRotationRate == 0x01) 
	{
		asi.IsSsd = TRUE;
		asi.NominalMediaRotationRate = 1;
	}

	if(asi.Major >= 7 && (asi.IdentifyDevice.NominalMediaRotationRate >= 0x401
	&& asi.IdentifyDevice.NominalMediaRotationRate < 0xFFFF))
	{
		asi.NominalMediaRotationRate = asi.IdentifyDevice.NominalMediaRotationRate;
	}

	if(asi.Major >= 7 
	&&(asi.IdentifyDevice.DeviceNominalFormFactor & 0xF) > 0
	&&(asi.IdentifyDevice.DeviceNominalFormFactor & 0xF) <= 5
	)
	{
		asi.DeviceNominalFormFactor.Format(_T("%s"),  
			deviceFormFactorString[asi.IdentifyDevice.DeviceNominalFormFactor & 0xF]);
	//	AfxMessageBox(asi.DeviceNominalFormFactor);
	}

	CString model = asi.Model;
	model.MakeUpper();
	if(model.Find(_T("MAXTOR")) == 0 && asi.DetectedTimeUnitType == POWER_ON_MINUTES)
	{
		asi.IsMaxtorMinute = TRUE;
	}

	// DiskSize & BufferSize
	if(identify->LogicalCylinders > 16383)
	{
		identify->LogicalCylinders = 16383;
		asi.IsIdInfoIncorrect = TRUE;
	}
	if(identify->LogicalHeads > 16)
	{
		identify->LogicalHeads = 16;
		asi.IsIdInfoIncorrect = TRUE;
	}
	if(identify->LogicalSectors > 63)
	{
		identify->LogicalSectors = 63;
		asi.IsIdInfoIncorrect = TRUE;
	}

	asi.Cylinder = identify->LogicalCylinders;
	asi.Head = identify->LogicalHeads;
	asi.Sector = identify->LogicalSectors;
	asi.Sector28 = 0x0FFFFFFF & identify->TotalAddressableSectors;
	asi.Sector48 = 0x0000FFFFFFFFFFFF & identify->MaxUserLba;

	if(identify->TotalAddressableSectors == 0x01100003) // 9126807040 bytes
	{
		asi.Is9126MB = TRUE;
	}

	if(identify->LogicalCylinders == 0 || identify->LogicalHeads == 0 || identify->LogicalSectors == 0)
	{
		return FALSE;
	//	asi.DiskSizeChs   = 0;
	}
	else if(((ULONGLONG)(identify->LogicalCylinders * identify->LogicalHeads * identify->LogicalSectors * 512) / 1000 / 1000) > 1000)
	{
		asi.DiskSizeChs   = (DWORD)(((ULONGLONG)identify->LogicalCylinders * identify->LogicalHeads * identify->LogicalSectors * 512) / 1000 / 1000 - 49);
	}
	else
	{
		asi.DiskSizeChs   = (DWORD)(((ULONGLONG)identify->LogicalCylinders * identify->LogicalHeads * identify->LogicalSectors * 512) / 1000 / 1000);
	}

	asi.NumberOfSectors = (ULONGLONG)identify->LogicalCylinders * identify->LogicalHeads * identify->LogicalSectors;
	if(asi.Sector28 > 0 && ((ULONGLONG)asi.Sector28 * 512) / 1000 / 1000 > 49)
	{
		asi.DiskSizeLba28 = (DWORD)(((ULONGLONG)asi.Sector28 * 512) / 1000 / 1000 - 49);
		asi.NumberOfSectors = asi.Sector28;
	}
	else
	{
		asi.DiskSizeLba28 = 0;
	}

	if(asi.IsLba48Supported && (asi.Sector48 * 512) / 1000 / 1000 > 49)
	{
		asi.DiskSizeLba48 = (DWORD)((asi.Sector48 * 512) / 1000 / 1000 - 49);
		asi.NumberOfSectors = asi.Sector48;
	}
	else
	{
		asi.DiskSizeLba48 = 0;
	}

	asi.BufferSize = identify->BufferSize * 512;
	if(asi.IsNvCacheSupported)
	{
		asi.NvCacheSize = identify->NvCacheSizeLogicalBlocks * 512;
	}

	if(identify->TotalAddressableSectors > 0x0FFFFFFF)
	{
		asi.TotalDiskSize = 0;
	}
	else if(asi.DiskSizeChs == 0)
	{
		asi.TotalDiskSize = 0;
	}
	else if(asi.DiskSizeLba48 > asi.DiskSizeLba28)
	{
		asi.TotalDiskSize = asi.DiskSizeLba48;
	}
	else if(asi.DiskSizeLba28 > asi.DiskSizeChs)
	{
		asi.TotalDiskSize = asi.DiskSizeLba28;
	}
	else
	{
	//	asi.TotalDiskSize = 0;
		asi.TotalDiskSize = asi.DiskSizeChs;
	}

	// Error Check for External ATA Controller
	if(asi.IsLba48Supported && (identify->TotalAddressableSectors < 268435455 && asi.DiskSizeLba28 != asi.DiskSizeLba48))
	{
		asi.DiskSizeLba48 = 0;
	}

	CString debug;
	// Check S.M.A.R.T. Enabled or Diabled
	if(asi.IsSmartSupported || asi.Is9126MB || IsAdvancedDiskSearch)
	{
		switch(asi.CommandType)
		{
		case CMD_TYPE_PHYSICAL_DRIVE:
	// DEBUG
		//	ControlSmartStatusPd(physicalDriveId, asi.Target, DISABLE_SMART);

			debug.Format(_T("GetSmartAttributePd(%d) - 1"), physicalDriveId);
			DebugPrint(debug);
			if(GetSmartAttributePd(physicalDriveId, asi.Target, &asi))
			{
				CheckSsdSupport(asi);
				GetSmartAttributePd(physicalDriveId, asi.Target, &asiCheck);
				if(CheckSmartAttributeCorrect(&asi, &asiCheck))
				{
					debug.Format(_T("GetSmartAttributePd(%d) - 1A"), physicalDriveId);
					asi.IsSmartCorrect = TRUE;
				}
				if(GetSmartThresholdPd(physicalDriveId, asi.Target, &asi))
				{
					asi.IsThresholdCorrect = TRUE;
				}
			//	asi.DiskStatus = CheckDiskStatus(asi.Attribute, asi.Threshold, asi.AttributeCount, asi.DiskVendorId, asi.IsSmartCorrect, asi.IsSsd);
				asi.IsSmartEnabled = TRUE;
			}
			
			if(! asi.IsSmartCorrect && ControlSmartStatusPd(physicalDriveId, asi.Target, ENABLE_SMART))
			{
				debug.Format(_T("GetSmartAttributePd(%d) - 2"), physicalDriveId);
				DebugPrint(debug);
				if(GetSmartAttributePd(physicalDriveId, asi.Target, &asi))
				{
					CheckSsdSupport(asi);
					GetSmartAttributePd(physicalDriveId, asi.Target, &asiCheck);
					if(CheckSmartAttributeCorrect(&asi, &asiCheck))
					{
						debug.Format(_T("GetSmartAttributePd(%d) - 2A"), physicalDriveId);
						asi.IsSmartCorrect = TRUE;
					}
					if(GetSmartThresholdPd(physicalDriveId, asi.Target, &asi))
					{
						asi.IsThresholdCorrect = TRUE;
					}
				//	asi.DiskStatus = CheckDiskStatus(asi.Attribute, asi.Threshold, asi.AttributeCount, asi.DiskVendorId, asi.IsSmartCorrect, asi.IsSsd);
					asi.IsSmartEnabled = TRUE;
				}
			}
			break;
		case CMD_TYPE_SCSI_MINIPORT:
			if(GetSmartAttributeScsi(scsiPort, scsiTargetId, &asi))
			{
				CheckSsdSupport(asi);
				GetSmartAttributeScsi(scsiPort, scsiTargetId, &asiCheck);
				if(CheckSmartAttributeCorrect(&asi, &asiCheck))
				{
					asi.IsSmartCorrect = TRUE;
				}
				if(GetSmartThresholdScsi(scsiPort, scsiTargetId, &asi))
				{
					asi.IsThresholdCorrect = TRUE;
				}
			//	asi.DiskStatus = CheckDiskStatus(asi.Attribute, asi.Threshold, asi.AttributeCount, asi.DiskVendorId, asi.IsSmartCorrect, asi.IsSsd);
				asi.IsSmartEnabled = TRUE;
			}
			
			if(! asi.IsSmartCorrect && ControlSmartStatusScsi(scsiPort, scsiTargetId, ENABLE_SMART))
			{
				if(GetSmartAttributeScsi(scsiPort, scsiTargetId, &asi))
				{
					CheckSsdSupport(asi);
					GetSmartAttributeScsi(scsiPort, scsiTargetId, &asiCheck);
					if(CheckSmartAttributeCorrect(&asi, &asiCheck))
					{
						asi.IsSmartCorrect = TRUE;
					}
					if(GetSmartThresholdScsi(scsiPort, scsiTargetId, &asi))
					{
						asi.IsThresholdCorrect = TRUE;
					}
				//	asi.DiskStatus = CheckDiskStatus(asi.Attribute, asi.Threshold, asi.AttributeCount, asi.DiskVendorId, asi.IsSmartCorrect, asi.IsSsd);
					asi.IsSmartEnabled = TRUE;
				}
			}
			break;
		case CMD_TYPE_SILICON_IMAGE:
			DebugPrint(_T("GetSmartAttributeSi(physicalDriveId, &asi)"));
			if(GetSmartAttributeSi(physicalDriveId, &asi))
			{
				CheckSsdSupport(asi);
				DebugPrint(_T("GetSmartAttributeSi(physicalDriveId, &asiCheck)"));
				GetSmartAttributeSi(physicalDriveId, &asiCheck);
				DebugPrint(_T("CheckSmartAttributeCorrect(&asi, &asiCheck) - 1"));
				if(CheckSmartAttributeCorrect(&asi, &asiCheck))
				{
					asi.IsSmartCorrect = TRUE;
					// Compare Si and Pd 
					GetSmartAttributePd(physicalDriveId, 0xA0, &asiCheck);
					DebugPrint(_T("CheckSmartAttributeCorrect(&asi, &asiCheck) - 2"));
					if(CheckSmartAttributeCorrect(&asi, &asiCheck))
					{
						DebugPrint(_T("GetSmartThresholdPd"));
						// Does not support GetSmartThresholdSi
						GetSmartThresholdPd(physicalDriveId, 0xA0, &asi);
						asi.IsThresholdCorrect = TRUE;
					}
				}

			//	asi.DiskStatus = CheckDiskStatus(asi.Attribute, asi.Threshold, asi.AttributeCount, asi.DiskVendorId, asi.IsSmartCorrect, asi.IsSsd);
				asi.IsSmartEnabled = TRUE;
			}
			break;
			
		case CMD_TYPE_SAT:
		case CMD_TYPE_SUNPLUS:
		case CMD_TYPE_IO_DATA:
		case CMD_TYPE_LOGITEC:
		case CMD_TYPE_JMICRON:
		case CMD_TYPE_CYPRESS:
			debug.Format(_T("GetSmartAttributeSat(%d) - 1 [%s]"), physicalDriveId, commandTypeString[asi.CommandType]);
			DebugPrint(debug);
			if(GetSmartAttributeSat(physicalDriveId, asi.Target, &asi))
			{
				CheckSsdSupport(asi);
				DebugPrint(_T("GetSmartAttributeSat - 1A"));
				GetSmartAttributeSat(physicalDriveId, asi.Target, &asiCheck);
				if(CheckSmartAttributeCorrect(&asi, &asiCheck))
				{
					asi.IsSmartCorrect = TRUE;
				}
				if(GetSmartThresholdSat(physicalDriveId, asi.Target, &asi))
				{
					asi.IsThresholdCorrect = TRUE;
				}
			//	asi.DiskStatus = CheckDiskStatus(asi.Attribute, asi.Threshold, asi.AttributeCount, asi.DiskVendorId, asi.IsSmartCorrect, asi.IsSsd);
				asi.IsSmartEnabled = TRUE;
			}
			
			if(! asi.IsSmartCorrect && ControlSmartStatusSat(physicalDriveId, asi.Target, ENABLE_SMART, asi.CommandType))
			{
				DebugPrint(_T("GetSmartAttributeSat - 2"));
				if(GetSmartAttributeSat(physicalDriveId, asi.Target, &asi))
				{
					CheckSsdSupport(asi);
					DebugPrint(_T("GetSmartAttributeSat - 2A"));
					GetSmartAttributeSat(physicalDriveId, asi.Target, &asiCheck);
					if(CheckSmartAttributeCorrect(&asi, &asiCheck))
					{
						asi.IsSmartCorrect = TRUE;
					}
					if(GetSmartThresholdSat(physicalDriveId, asi.Target, &asi))
					{
						asi.IsThresholdCorrect = TRUE;
					}
				//	asi.DiskStatus = CheckDiskStatus(asi.Attribute, asi.Threshold, asi.AttributeCount, asi.DiskVendorId, asi.IsSmartCorrect, asi.IsSsd);
					asi.IsSmartEnabled = TRUE;
				}
			}
			break;
		default:
			return FALSE;
			break;
		}
	}

	// OCZ-VERTEX3 2.02 Firmware Bug
	// http://crystalmark.info/bbs/c-board.cgi?cmd=one;no=303;id=diskinfo#303
	// http://www.ocztechnologyforum.com/forum/showthread.php?88540-Vertex-3-Issues-Errors-and-Slow-speeds.
	if(asi.DiskVendorId == SSD_VENDOR_SANDFORCE && asi.Model.Find(_T("OCZ-VERTEX3")) == 0 && asi.FirmwareRev.Find(_T("2.02")) == 0)
	{
		asi.IsThresholdBug = TRUE;
	}

	for(int i = 0; i < vars.GetCount(); i++)
	{
		if(asi.Model.Compare(vars[i].Model) == 0 && asi.SerialNumber.Compare(vars[i].SerialNumber) == 0)
		{
			return FALSE;
		}
	}

	// DEBUG
	// asi.IsSmartCorrect = rand() %2;

	asi.PowerOnStartRawValue = asi.PowerOnRawValue;

	if(! asi.IsSmartCorrect)
	{
		asi.DetectedPowerOnHours = -1;
		asi.MeasuredPowerOnHours = -1;
		asi.PowerOnRawValue = -1;
		asi.PowerOnStartRawValue = -1;
		asi.PowerOnCount = 0;
		asi.Temperature = 0;
		asi.DiskStatus = DISK_STATUS_UNKNOWN;
	}

	if(asi.IsIdInfoIncorrect && (! IsAdvancedDiskSearch || commandType >= CMD_TYPE_SAT))
	{
		return FALSE;
	}

	if(vars.GetCount() < MAX_DISK){vars.Add(asi);}

	return TRUE;
}

VOID CAtaSmart::CheckSsdSupport(ATA_SMART_INFO &asi)
{
	// Old SSD Detection
	if(IsSsdOld(asi))
	{
		asi.IsSsd = TRUE;
	}

	if(IsSsdMtron(asi))
	{
		asi.SmartKeyName = _T("SmartMtron");
		asi.DiskVendorId = SSD_VENDOR_MTRON;
		asi.SsdVendorString = ssdVendorString[asi.DiskVendorId];
		asi.IsSsd = TRUE;
	}
	else if(IsSsdJMicron60x(asi))
	{
		asi.SmartKeyName = _T("SmartJMicron60x");
		asi.DiskVendorId = SSD_VENDOR_JMICRON;
		asi.SsdVendorString = ssdVendorString[asi.DiskVendorId];
		asi.IsSsd = TRUE;
		asi.IsRawValues8 = TRUE;
		return ;
	}
	else if(IsSsdJMicron61x(asi))
	{
		asi.SmartKeyName = _T("SmartJMicron61x");
		asi.DiskVendorId = SSD_VENDOR_JMICRON;
		asi.SsdVendorString = ssdVendorString[asi.DiskVendorId];
		asi.IsSsd = TRUE;
		return ;
	}
	else if(IsSsdIndlinx(asi))
	{
		asi.SmartKeyName = _T("SmartIndlinx");
		asi.DiskVendorId = SSD_VENDOR_INDILINX;
		asi.SsdVendorString = ssdVendorString[asi.DiskVendorId];
		asi.IsSsd = TRUE;
		asi.IsRawValues8 = TRUE;
	}
	else if(IsSsdIntel(asi))
	{
		asi.SmartKeyName = _T("SmartIntel");
		asi.DiskVendorId = SSD_VENDOR_INTEL;
		asi.SsdVendorString = ssdVendorString[asi.DiskVendorId];
		asi.IsSsd = TRUE;
	}
	else if(IsSsdSamsung(asi))
	{
		asi.SmartKeyName = _T("SmartSamsung");
		asi.DiskVendorId = SSD_VENDOR_SAMSUNG;
		asi.SsdVendorString = ssdVendorString[asi.DiskVendorId];
		asi.IsSsd = TRUE;
	}
	else if(IsSsdSandForce(asi))
	{
		asi.SmartKeyName = _T("SmartSandForce");
		asi.DiskVendorId = SSD_VENDOR_SANDFORCE;
		asi.SsdVendorString = ssdVendorString[asi.DiskVendorId];
		asi.IsSsd = TRUE;
		asi.IsRawValues7 = TRUE;
	}
	else if(IsSsdMicron(asi))
	{
		asi.SmartKeyName = _T("SmartMicron");
		asi.DiskVendorId = SSD_VENDOR_MICRON;
		asi.SsdVendorString = ssdVendorString[asi.DiskVendorId];
		asi.IsSsd = TRUE;
	}
	else if(asi.IsSsd)
	{
		asi.SmartKeyName = _T("SmartSsd");
		return ;
	}
	else
	{
		asi.SmartKeyName = _T("Smart");
		return ;
	}

// Update Life
	for(DWORD j = 0; j < asi.AttributeCount; j++)
	{
		switch(asi.Attribute[j].Id)
		{
		case 0xBB:
			if(asi.DiskVendorId == SSD_VENDOR_MTRON)
			{
				if(asi.Attribute[j].CurrentValue <= 100)
				{
					asi.Life = asi.Attribute[j].CurrentValue;
				}
			}
			break;
		case 0xD1:
			if(asi.DiskVendorId == SSD_VENDOR_INDILINX)
			{
				if(asi.Attribute[j].CurrentValue <= 100)
				{
					asi.Life = asi.Attribute[j].CurrentValue;
				}
			}
			break;
		case 0xE8:
			if(asi.DiskVendorId == SSD_VENDOR_INTEL)
			{
				if(asi.Attribute[j].CurrentValue <= 100)
				{
					asi.Life = asi.Attribute[j].CurrentValue;
				}
			}
			break;
		case 0xE1:
			if(asi.DiskVendorId == SSD_VENDOR_INTEL)
			{
				asi.HostWrites  = MAKELONG(
					MAKEWORD(asi.Attribute[j].RawValue[0], asi.Attribute[j].RawValue[1]),
					MAKEWORD(asi.Attribute[j].RawValue[2], asi.Attribute[j].RawValue[3])
					);
			}
			break;
		case 0xF1:
			if(asi.DiskVendorId == SSD_VENDOR_INTEL || asi.DiskVendorId == SSD_VENDOR_SANDFORCE)
			{
				asi.HostWrites  = MAKELONG(
					MAKEWORD(asi.Attribute[j].RawValue[0], asi.Attribute[j].RawValue[1]),
					MAKEWORD(asi.Attribute[j].RawValue[2], asi.Attribute[j].RawValue[3])
					);
			}
			break;
		case 0xF2:
			if(asi.DiskVendorId == SSD_VENDOR_INTEL || asi.DiskVendorId == SSD_VENDOR_SANDFORCE)
			{
				asi.HostReads  = MAKELONG(
					MAKEWORD(asi.Attribute[j].RawValue[0], asi.Attribute[j].RawValue[1]),
					MAKEWORD(asi.Attribute[j].RawValue[2], asi.Attribute[j].RawValue[3])
					);
			}
			break;
		case 0x64:
			if(asi.DiskVendorId == SSD_VENDOR_SANDFORCE)
			{
				asi.GBytesErased  = MAKELONG(
					MAKEWORD(asi.Attribute[j].RawValue[0], asi.Attribute[j].RawValue[1]),
					MAKEWORD(asi.Attribute[j].RawValue[2], asi.Attribute[j].RawValue[3])
					);
			}
			break;
		case 0xB4:
			if(asi.DiskVendorId == SSD_VENDOR_SAMSUNG)
			{
				if(asi.Attribute[j].CurrentValue <= 100)
				{
					asi.Life = asi.Attribute[j].CurrentValue;
				}
			}
			break;
		case 0xE7:
			if(asi.DiskVendorId == SSD_VENDOR_SANDFORCE)
			{
				if(asi.Attribute[j].CurrentValue <= 100)
				{
					asi.Life = asi.Attribute[j].CurrentValue;
				}
			}
			break;
		case 0xAA:
			if(asi.DiskVendorId == SSD_VENDOR_JMICRON && ! asi.IsRawValues8)
			{
				if(asi.Attribute[j].CurrentValue <= 100)
				{
					asi.Life = asi.Attribute[j].CurrentValue;
				}
			}
			break;
		case 0xCA:
			if(asi.DiskVendorId == SSD_VENDOR_MICRON)
			{
				if(asi.Attribute[j].CurrentValue <= 100)
				{
					asi.Life = asi.Attribute[j].CurrentValue;
				}
			}
			break;
		default:
			break;
		}
	}
}

BOOL CAtaSmart::IsSsdOld(ATA_SMART_INFO &asi)
{
	return asi.Model.Find(_T("OCZ")) == 0 
		|| asi.Model.Find(_T("SPCC")) == 0
		|| asi.Model.Find(_T("PATRIOT")) == 0
		|| asi.Model.Find(_T("Solid")) >= 0
		|| asi.Model.Find(_T("SSD")) >= 0
		|| asi.Model.Find(_T("SiliconHardDisk")) >= 0
		|| asi.Model.Find(_T("PHOTOFAST")) == 0
		|| asi.Model.Find(_T("STT_FTM")) == 0
		|| asi.Model.Find(_T("Super Talent")) == 0
		;
}

BOOL CAtaSmart::IsSsdMtron(ATA_SMART_INFO &asi)
{
	return ((asi.Attribute[ 0].Id == 0xBB && asi.AttributeCount == 1) || asi.Model.Find(_T("MTRON")) == 0);
}

BOOL CAtaSmart::IsSsdJMicron60x(ATA_SMART_INFO &asi)
{
	BOOL flagSmartType = FALSE;

	if(asi.Attribute[ 0].Id == 0x0C
	&& asi.Attribute[ 1].Id == 0x09
	&& asi.Attribute[ 2].Id == 0xC2
	&& asi.Attribute[ 3].Id == 0xE5
	&& asi.Attribute[ 4].Id == 0xE8
	&& asi.Attribute[ 5].Id == 0xE9
//	&& asi.Attribute[ 6].Id == 0xEA
//	&& asi.Attribute[ 7].Id == 0xEB
	)
	{
		flagSmartType = TRUE;
	}

	return flagSmartType;
}

BOOL CAtaSmart::IsSsdJMicron61x(ATA_SMART_INFO &asi)
{
	BOOL flagSmartType = FALSE;

	if(asi.Attribute[ 0].Id == 0x01
	&& asi.Attribute[ 1].Id == 0x02
	&& asi.Attribute[ 2].Id == 0x03
	&& asi.Attribute[ 3].Id == 0x05
	&& asi.Attribute[ 4].Id == 0x07
	&& asi.Attribute[ 5].Id == 0x08
	&& asi.Attribute[ 6].Id == 0x09
	&& asi.Attribute[ 7].Id == 0x0A
	&& asi.Attribute[ 8].Id == 0x0C
	&& asi.Attribute[ 9].Id == 0xA8
	&& asi.Attribute[10].Id == 0xAF
	&& asi.Attribute[11].Id == 0xC0
	&& asi.Attribute[12].Id == 0xC2
//	&& asi.Attribute[13].Id == 0xF0
//	&& asi.Attribute[14].Id == 0xAA
//	&& asi.Attribute[15].Id == 0xAD
	)
	{
		flagSmartType = TRUE;
	}

	return flagSmartType;
}

BOOL CAtaSmart::IsSsdIndlinx(ATA_SMART_INFO &asi)
{
	BOOL flagSmartType = FALSE;

	if(asi.Attribute[ 0].Id == 0x01
	&& asi.Attribute[ 1].Id == 0x09
	&& asi.Attribute[ 2].Id == 0x0C
	&& asi.Attribute[ 3].Id == 0xB8
	&& asi.Attribute[ 4].Id == 0xC3
	&& asi.Attribute[ 5].Id == 0xC4
//	&& asi.Attribute[ 6].Id == 0xC5
//	&& asi.Attribute[ 7].Id == 0xC6
//	&& asi.Attribute[ 8].Id == 0xC7
//	&& asi.Attribute[ 9].Id == 0xC8
//	&& asi.Attribute[10].Id == 0xC9
//	&& asi.Attribute[11].Id == 0xCA
//	&& asi.Attribute[12].Id == 0xCB
//	&& asi.Attribute[13].Id == 0xCC
//	&& asi.Attribute[14].Id == 0xCD
//	&& asi.Attribute[15].Id == 0xCE
//	&& asi.Attribute[16].Id == 0xCF
//	&& asi.Attribute[17].Id == 0xD0
//	&& asi.Attribute[18].Id == 0xD1
//	&& asi.Attribute[19].Id == 0xD2
//	&& asi.Attribute[20].Id == 0xD3
	)
	{
		flagSmartType = TRUE;
	}

	return flagSmartType;
		/*
		   asi.Model.Find(_T("OCZ-VERTEX")) == 0
		|| asi.Model.Find(_T("G-Monster-V3")) == 0
		|| asi.Model.Find(_T("G-Monster-V5")) == 0 
		|| (asi.Model.Find(_T("STT_FTM")) == 0 && asi.Model.Find(_T("GX")) > 0)
		|| asi.Model.Find(_T("Solidata")) == 0
		*/
}

BOOL CAtaSmart::IsSsdIntel(ATA_SMART_INFO &asi)
{
	BOOL flagSmartType = FALSE;

	if(asi.Attribute[ 0].Id == 0x03
	&& asi.Attribute[ 1].Id == 0x04
	&& asi.Attribute[ 2].Id == 0x05
	&& asi.Attribute[ 3].Id == 0x09
	&& asi.Attribute[ 4].Id == 0x0C
	&& asi.Attribute[ 5].Id == 0xC0
	)
	{
		if(asi.Attribute[ 6].Id == 0xE8 && asi.Attribute[ 7].Id == 0xE9)
		{
			flagSmartType = TRUE;
		}
		else if(asi.Attribute[ 6].Id == 0xE1)
		{
			flagSmartType = TRUE;
		}
	}

	return (asi.Model.Find(_T("INTEL")) == 0 || asi.Model.Find(_T(" INTEL")) > 0 || flagSmartType == TRUE);
}


BOOL CAtaSmart::IsSsdSamsung(ATA_SMART_INFO &asi)
{
	BOOL flagSmartType = FALSE;

	if(asi.Attribute[ 0].Id == 0x09
	&& asi.Attribute[ 1].Id == 0x0C
	&& asi.Attribute[ 2].Id == 0xB2
	&& asi.Attribute[ 3].Id == 0xB3
	&& asi.Attribute[ 4].Id == 0xB4
	)
	{
		flagSmartType = TRUE;
	}

	if(asi.Attribute[ 0].Id == 0x09
	&& asi.Attribute[ 1].Id == 0x0C
	&& asi.Attribute[ 2].Id == 0xAF
	&& asi.Attribute[ 3].Id == 0xB0
	&& asi.Attribute[ 4].Id == 0xB1
	&& asi.Attribute[ 5].Id == 0xB2
	&& asi.Attribute[ 6].Id == 0xB3
	&& asi.Attribute[ 7].Id == 0xB4
	)
	{
		flagSmartType = TRUE;
	}

	return flagSmartType;
}

BOOL CAtaSmart::IsSsdSandForce(ATA_SMART_INFO &asi)
{
	BOOL flagSmartType = FALSE;

	if(asi.Attribute[ 0].Id == 0x01
	&& asi.Attribute[ 1].Id == 0x05
	&& asi.Attribute[ 2].Id == 0x09
	&& asi.Attribute[ 3].Id == 0x0C
	&& asi.Attribute[ 4].Id == 0x0D
	&& asi.Attribute[ 5].Id == 0x64
	&& asi.Attribute[ 6].Id == 0xAA
	)
	{
		flagSmartType = TRUE;
	}

	if(asi.Attribute[ 0].Id == 0x01
	&& asi.Attribute[ 1].Id == 0x05
	&& asi.Attribute[ 2].Id == 0x09
	&& asi.Attribute[ 3].Id == 0x0C
	&& asi.Attribute[ 4].Id == 0xAB
	&& asi.Attribute[ 5].Id == 0xAC
	)
	{
		flagSmartType = TRUE;
	}

	return (asi.Model.Find(_T("SandForce")) == 0 || flagSmartType);
}


BOOL CAtaSmart::IsSsdMicron(ATA_SMART_INFO &asi)
{
	BOOL flagSmartType = FALSE;

	if(asi.Attribute[ 0].Id == 0x01
	&& asi.Attribute[ 1].Id == 0x05
	&& asi.Attribute[ 2].Id == 0x09
	&& asi.Attribute[ 3].Id == 0x0C
	&& asi.Attribute[ 4].Id == 0xAA
	&& asi.Attribute[ 5].Id == 0xAB
	&& asi.Attribute[ 6].Id == 0xAC
	&& asi.Attribute[ 7].Id == 0xAD
	&& asi.Attribute[ 8].Id == 0xAE
	&& asi.Attribute[ 9].Id == 0xB5
	&& asi.Attribute[10].Id == 0xB7
	)
	{
		flagSmartType = TRUE;
	}
	
	return (flagSmartType);
}

BOOL CAtaSmart::CheckSmartAttributeCorrect(ATA_SMART_INFO* asi1, ATA_SMART_INFO* asi2)
{
	if(asi1->AttributeCount != asi2->AttributeCount)
	{
		DebugPrint(_T("asi1->AttributeCount != asi2->AttributeCount"));
		return FALSE;
	}
	
	for(DWORD i = 0; i < asi1->AttributeCount; i++)
	{
		if(asi1->Attribute[i].Id != asi2->Attribute[i].Id)
		{
			DebugPrint(_T("asi1->Attribute[i].Id != asi2->Attribute[i].Id"));
			return FALSE;
		}
	}

	return TRUE; // Correct
}

VOID CAtaSmart::WakeUp(INT physicalDriveId)
{
	HANDLE hFile = INVALID_HANDLE_VALUE;
	CString	strDevice;
	if(physicalDriveId < 0)
	{
		return ;
	}

	strDevice.Format(_T("\\\\.\\PhysicalDrive%d"), physicalDriveId);
	hFile = ::CreateFile(strDevice, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if(hFile != INVALID_HANDLE_VALUE)
	{
		BYTE buf[512];
		const DWORD bufSize = 512;
		DWORD readSize = 0;
		SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
		ReadFile(hFile, buf, bufSize, &readSize, NULL);
		CloseHandle(hFile);
	}
}

BOOL CAtaSmart::GetDiskInfo(INT physicalDriveId, INT scsiPort, INT scsiTargetId, 
	INTERFACE_TYPE interfaceType, VENDOR_ID usbVendorId, DWORD productId, INT scsiBus, DWORD siliconImageType
	)
{
	DebugPrint(_T("GetDiskInfo"));
	if(vars.GetCount() > MAX_DISK)
	{
		DebugPrint(_T("GetDiskInfo - FALSE0"));
		return FALSE;
	}
	// Check overlap
	for(int i = 0; i < vars.GetCount(); i++)
	{
		if(physicalDriveId >= 0 && vars[i].PhysicalDriveId == physicalDriveId)
		{
			DebugPrint(_T("GetDiskInfo - FALSE1"));
			return FALSE;
		}
		else if(scsiPort >= 0 && scsiTargetId >= 0
			&& vars[i].ScsiPort == scsiPort && vars[i].ScsiTargetId == scsiTargetId
			&& vars[i].ScsiBus == scsiBus
			)
		{
			DebugPrint(_T("GetDiskInfo - FALSE2"));
			return FALSE;
		}
	}

	IDENTIFY_DEVICE identify = {0};

	CString debug;
	if(interfaceType == INTERFACE_TYPE_UNKNOWN || interfaceType == INTERFACE_TYPE_PATA || interfaceType == INTERFACE_TYPE_SATA)
	{
		if(siliconImageType)
		{
			WakeUp(physicalDriveId);
			debug.Format(_T("physicalDriveId=%d, scsiPort=%d, scsiBus=%d, scsiTargetId=%d"), physicalDriveId, scsiPort, scsiBus, scsiTargetId);
			DebugPrint(debug);
			if(DoIdentifyDeviceSi(physicalDriveId, scsiPort, scsiBus, siliconImageType, &identify))
			{
				DebugPrint(_T("AddDisk - Si"));
				if(AddDisk(physicalDriveId, scsiPort, scsiTargetId, scsiBus, 0xA0, CMD_TYPE_SILICON_IMAGE, &identify, siliconImageType))
				{
					return TRUE;
				}
				return FALSE;
			}
		}

		if(physicalDriveId >= 0)
		{
			debug.Format(_T("DoIdentifyDevicePd(%d, 0xA0) - 1"), physicalDriveId);
			DebugPrint(debug);
			if(! DoIdentifyDevicePd(physicalDriveId, 0xA0, &identify))
			{
				debug.Format(_T("WakeUp(%d)"), physicalDriveId);
				DebugPrint(debug);
				WakeUp(physicalDriveId);

				debug.Format(_T("DoIdentifyDevicePd(%d, 0xA0) - 2"), physicalDriveId);
				DebugPrint(debug);
				if(! DoIdentifyDevicePd(physicalDriveId, 0xA0, &identify))
				{
					debug.Format(_T("DoIdentifyDevicePd(%d, 0xB0) - 3"), physicalDriveId);
					DebugPrint(debug);

					if(! DoIdentifyDevicePd(physicalDriveId, 0xB0, &identify))
					{
						debug.Format(_T("DoIdentifyDeviceScsi(%d, %d) - 4"), scsiPort, scsiTargetId);
						DebugPrint(debug);

						if(scsiPort >= 0 && scsiTargetId >= 0 && DoIdentifyDeviceScsi(scsiPort, scsiTargetId, &identify))
						{
							debug.Format(_T("AddDisk(%d, %d, %d) - 5"), physicalDriveId, scsiPort, scsiTargetId);
							DebugPrint(debug);
							return AddDisk(physicalDriveId, scsiPort, scsiTargetId, scsiBus, 0xA0, CMD_TYPE_SCSI_MINIPORT, &identify);
						}
						else
						{
							return FALSE;
						}
					}
				}
			}
			debug.Format(_T("AddDisk(%d, %d, %d) - 6"), physicalDriveId, scsiPort, scsiTargetId);
			DebugPrint(debug);
			return AddDisk(physicalDriveId, scsiPort, scsiTargetId, scsiBus, 0xA0, CMD_TYPE_PHYSICAL_DRIVE, &identify);
		}
		else
		{
			debug.Format(_T("DoIdentifyDeviceScsi(%d, %d) - 7"), scsiPort, scsiTargetId);
			DebugPrint(debug);
			if(scsiPort >= 0 && scsiTargetId >= 0 && DoIdentifyDeviceScsi(scsiPort, scsiTargetId, &identify))
			{
				debug.Format(_T("AddDisk(%d, %d, %d) - 8"), physicalDriveId, scsiPort, scsiTargetId);
				DebugPrint(debug);
				return AddDisk(physicalDriveId, scsiPort, scsiTargetId, scsiBus, 0xA0, CMD_TYPE_SCSI_MINIPORT, &identify);
			}
		}
	}
	else if(physicalDriveId >= 0)
	{
		/** DEBUG
		if(TRUE)
		{
			DoIdentifyDeviceSat(physicalDriveId, &identify, CMD_TYPE_DEBUG);
		}
		else
		*/

		WakeUp(physicalDriveId);

		if(interfaceType == INTERFACE_TYPE_USB && usbVendorId == USB_VENDOR_LOGITEC && productId == 0x00D9)
		{
			DebugPrint(_T("FALSE: usbVendorId == USB_VENDOR_LOGITEC && productId == 0x00D9"));
			return FALSE;
		}

		// 2009/7/30, 2009/8/21
		// IO-DATA HDPS-U  (http://crystalmark.info/bbs/c-board.cgi?cmd=one;no=2918;id=report#2918)
		//                 (http://crystalmark.info/bbs/c-board.cgi?cmd=one;no=2978;id=report#2978)
		//                 (http://crystalmark.info/bbs/c-board.cgi?cmd=one;no=2985;id=report#2985)
		if(interfaceType == INTERFACE_TYPE_USB && usbVendorId == USB_VENDOR_IO_DATA && productId == 0x0122)
		{
			DebugPrint(_T("usbVendorId == USB_VENDOR_IO_DATA && productId == 0x0122"));
			if(FlagUsbJmicron && DoIdentifyDeviceSat(physicalDriveId, 0xA0, &identify,  CMD_TYPE_JMICRON))
			{
				DebugPrint(_T("AddDisk - USB0"));
				return AddDisk(physicalDriveId, scsiPort, scsiTargetId, scsiBus, 0xA0,  CMD_TYPE_JMICRON, &identify);
			}
			else if(FlagUsbSat && DoIdentifyDeviceSat(physicalDriveId, 0xA0, &identify, CMD_TYPE_SAT))
			{
				DebugPrint(_T("AddDisk - USB1"));
				return AddDisk(physicalDriveId, scsiPort, scsiTargetId, scsiBus, 0xA0, CMD_TYPE_SAT, &identify);
			}
			else
			{
				DebugPrint(_T("FALSE - USB0"));
				return FALSE;
			}
		}

		if(interfaceType == INTERFACE_TYPE_USB && (usbVendorId == USB_VENDOR_IO_DATA || usbVendorId == USB_VENDOR_ALL))
		{
			DebugPrint(_T("usbVendorId == USB_VENDOR_IO_DATA || usbVendorId == USB_VENDOR_ALL"));
			if(FlagUsbIodata && DoIdentifyDeviceSat(physicalDriveId, 0xA0, &identify, CMD_TYPE_IO_DATA))
			{
				DebugPrint(_T("AddDisk - USB2"));
				return AddDisk(physicalDriveId, scsiPort, scsiTargetId, scsiBus, 0xA0, CMD_TYPE_IO_DATA, &identify);
			}
			else if(FlagUsbIodata && DoIdentifyDeviceSat(physicalDriveId, 0xB0, &identify, CMD_TYPE_IO_DATA))
			{
				DebugPrint(_T("AddDisk - USB3"));
				return AddDisk(physicalDriveId, scsiPort, scsiTargetId, scsiBus, 0xB0, CMD_TYPE_IO_DATA, &identify);
			}
		}

/*
		else if(interfaceType == INTERFACE_TYPE_USB && vendorId == USB_VENDOR_LOGITEC && productId != 0x00D9)
		{
			if(DoIdentifyDeviceSat(physicalDriveId, &identify, CMD_TYPE_LOGITEC))
			{
				return AddDisk(physicalDriveId, scsiPort, scsiTargetId, CMD_TYPE_LOGITEC, &identify);
			}
		}
*/

		if(interfaceType == INTERFACE_TYPE_USB && usbVendorId == USB_VENDOR_SUNPLUS)
		{
			DebugPrint(_T("usbVendorId == USB_VENDOR_SUNPLUS"));
			if(FlagUsbSunplus && DoIdentifyDeviceSat(physicalDriveId, 0xA0, &identify, CMD_TYPE_SUNPLUS))
			{
				DebugPrint(_T("AddDisk - USB4"));
				return AddDisk(physicalDriveId, scsiPort, scsiTargetId, scsiBus, 0xA0, CMD_TYPE_SUNPLUS, &identify);
			}
			else if(FlagUsbSunplus && DoIdentifyDeviceSat(physicalDriveId, 0xB0, &identify, CMD_TYPE_SUNPLUS))
			{
				DebugPrint(_T("AddDisk - USB5"));
				return AddDisk(physicalDriveId, scsiPort, scsiTargetId, scsiBus, 0xB0, CMD_TYPE_SUNPLUS, &identify);
			}
		}
		else if(interfaceType == INTERFACE_TYPE_USB && usbVendorId == USB_VENDOR_CYPRESS)
		{
			DebugPrint(_T("usbVendorId == USB_VENDOR_CYPRESS"));
			if(FlagUsbCypress && DoIdentifyDeviceSat(physicalDriveId, 0xA0, &identify, CMD_TYPE_CYPRESS))
			{
				DebugPrint(_T("AddDisk - USB6"));
				return AddDisk(physicalDriveId, scsiPort, scsiTargetId, scsiBus, 0xA0, CMD_TYPE_CYPRESS, &identify);
			}
			else if(FlagUsbCypress && DoIdentifyDeviceSat(physicalDriveId, 0xB0, &identify, CMD_TYPE_CYPRESS))
			{
				DebugPrint(_T("AddDisk - USB7"));
				return AddDisk(physicalDriveId, scsiPort, scsiTargetId, scsiBus, 0xB0, CMD_TYPE_CYPRESS, &identify);
			}
		}
		else if(interfaceType == INTERFACE_TYPE_USB && 
			(usbVendorId == USB_VENDOR_INITIO || usbVendorId == USB_VENDOR_OXFORD)
			)
		{
			DebugPrint(_T("usbVendorId == USB_VENDOR_INITIO || usbVendorId == USB_VENDOR_OXFORD"));

			if(FlagUsbSat && DoIdentifyDeviceSat(physicalDriveId, 0xA0, &identify, CMD_TYPE_SAT))
			{
				DebugPrint(_T("AddDisk - USB8"));
				return AddDisk(physicalDriveId, scsiPort, scsiTargetId, scsiBus, 0xA0, CMD_TYPE_SAT, &identify);
			}
			else if(FlagUsbSat && DoIdentifyDeviceSat(physicalDriveId, 0xB0, &identify, CMD_TYPE_SAT))
			{
				DebugPrint(_T("AddDisk - USB9"));
				return AddDisk(physicalDriveId, scsiPort, scsiTargetId, scsiBus, 0xB0, CMD_TYPE_SAT, &identify);
			}
		}
		else
		{
			DebugPrint(_T("else (USB-HDD)"));

			if(FlagUsbSat && DoIdentifyDeviceSat(physicalDriveId, 0xA0, &identify, CMD_TYPE_SAT))
			{
				DebugPrint(_T("AddDisk - USB10"));
				return AddDisk(physicalDriveId, scsiPort, scsiTargetId, scsiBus, 0xA0, CMD_TYPE_SAT, &identify);
			}
			else if(FlagUsbJmicron && DoIdentifyDeviceSat(physicalDriveId, 0xA0, &identify, CMD_TYPE_JMICRON))
			{
				DebugPrint(_T("AddDisk - USB11"));
				return AddDisk(physicalDriveId, scsiPort, scsiTargetId, scsiBus, 0xA0, CMD_TYPE_JMICRON, &identify);
			}
			else if(FlagUsbSunplus && DoIdentifyDeviceSat(physicalDriveId, 0xA0, &identify, CMD_TYPE_SUNPLUS))
			{
				DebugPrint(_T("AddDisk - USB12"));
				return AddDisk(physicalDriveId, scsiPort, scsiTargetId, scsiBus, 0xA0, CMD_TYPE_SUNPLUS, &identify);
			}
			else if(FlagUsbCypress && DoIdentifyDeviceSat(physicalDriveId, 0xA0, &identify, CMD_TYPE_CYPRESS))
			{
				DebugPrint(_T("AddDisk - USB13"));
				return AddDisk(physicalDriveId, scsiPort, scsiTargetId, scsiBus, 0xA0, CMD_TYPE_CYPRESS, &identify);
			}
			else if(FlagUsbLogitec && DoIdentifyDeviceSat(physicalDriveId, 0xA0, &identify, CMD_TYPE_LOGITEC))
			{
				DebugPrint(_T("AddDisk - USB14"));
				return AddDisk(physicalDriveId, scsiPort, scsiTargetId, scsiBus, 0xA0, CMD_TYPE_LOGITEC, &identify);
			}
			else if(FlagUsbLogitec && FlagUsbSat && DoIdentifyDeviceSat(physicalDriveId, 0xB0, &identify, CMD_TYPE_SAT))
			{
				DebugPrint(_T("AddDisk - USB15"));
				return AddDisk(physicalDriveId, scsiPort, scsiTargetId, scsiBus, 0xB0, CMD_TYPE_SAT, &identify);
			}
			else if(FlagUsbJmicron && DoIdentifyDeviceSat(physicalDriveId, 0xB0, &identify, CMD_TYPE_JMICRON))
			{
				DebugPrint(_T("AddDisk - USB16"));
				return AddDisk(physicalDriveId, scsiPort, scsiTargetId, scsiBus, 0xB0, CMD_TYPE_JMICRON, &identify);
			}
			else if(FlagUsbSunplus && DoIdentifyDeviceSat(physicalDriveId, 0xB0, &identify, CMD_TYPE_SUNPLUS))
			{
				DebugPrint(_T("AddDisk - USB17"));
				return AddDisk(physicalDriveId, scsiPort, scsiTargetId, scsiBus, 0xB0, CMD_TYPE_SUNPLUS, &identify);
			}
			else if(FlagUsbCypress && DoIdentifyDeviceSat(physicalDriveId, 0xB0, &identify, CMD_TYPE_CYPRESS))
			{
				DebugPrint(_T("AddDisk - USB18"));
				return AddDisk(physicalDriveId, scsiPort, scsiTargetId, scsiBus, 0xB0, CMD_TYPE_CYPRESS, &identify);
			}
			else if(FlagUsbLogitec && DoIdentifyDeviceSat(physicalDriveId, 0xB0, &identify, CMD_TYPE_LOGITEC))
			{
				DebugPrint(_T("AddDisk - USB19"));
				return AddDisk(physicalDriveId, scsiPort, scsiTargetId, scsiBus, 0xB0, CMD_TYPE_LOGITEC, &identify);
			}
		}
	}

	return FALSE;
}

/*---------------------------------------------------------------------------*/
// \\\\.\\PhysicalDriveX
/*---------------------------------------------------------------------------*/
HANDLE CAtaSmart::GetIoCtrlHandle(BYTE index)
{
	CString	strDevice;
	strDevice.Format(_T("\\\\.\\PhysicalDrive%d"), index);

	return ::CreateFile(strDevice, GENERIC_READ | GENERIC_WRITE, 
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, 0, NULL);
}

BOOL CAtaSmart::DoIdentifyDevicePd(INT physicalDriveId, BYTE target, IDENTIFY_DEVICE* data)
{
	BOOL	bRet = FALSE;
	HANDLE	hIoCtrl;
	DWORD	dwReturned;
	CString cstr;

	IDENTIFY_DEVICE_OUTDATA	sendCmdOutParam;
	SENDCMDINPARAMS	sendCmd;

	if(data == NULL)
	{
		return	FALSE;
	}

	DebugPrint(_T("SendAtaCommandPd - IDENTIFY_DEVICE (ATA_PASS_THROUGH)"));
	bRet = SendAtaCommandPd(physicalDriveId, target, 0xEC, 0x00, 0x00, (PBYTE)data, sizeof(IDENTIFY_DEVICE));
	cstr = data->Model;

	if(bRet == FALSE || cstr.IsEmpty())
	{
		::ZeroMemory(data, sizeof(IDENTIFY_DEVICE));
		hIoCtrl = GetIoCtrlHandle(physicalDriveId);
		if(hIoCtrl == INVALID_HANDLE_VALUE)
		{
			return	FALSE;
		}
		::ZeroMemory(&sendCmdOutParam, sizeof(IDENTIFY_DEVICE_OUTDATA));
		::ZeroMemory(&sendCmd, sizeof(SENDCMDINPARAMS));

		sendCmd.irDriveRegs.bCommandReg			= ID_CMD;
		sendCmd.irDriveRegs.bSectorCountReg		= 1;
		sendCmd.irDriveRegs.bSectorNumberReg	= 1;
		sendCmd.irDriveRegs.bDriveHeadReg		= target;
		sendCmd.cBufferSize						= IDENTIFY_BUFFER_SIZE;

		DebugPrint(_T("SendAtaCommandPd - IDENTIFY_DEVICE"));
		bRet = ::DeviceIoControl(hIoCtrl, DFP_RECEIVE_DRIVE_DATA, 
			&sendCmd, sizeof(SENDCMDINPARAMS),
			&sendCmdOutParam, sizeof(IDENTIFY_DEVICE_OUTDATA),
			&dwReturned, NULL);

		::CloseHandle(hIoCtrl);
		
		if(bRet == FALSE || dwReturned != sizeof(IDENTIFY_DEVICE_OUTDATA))
		{
			return	FALSE;
		}

		memcpy_s(data, sizeof(IDENTIFY_DEVICE), sendCmdOutParam.SendCmdOutParam.bBuffer, sizeof(IDENTIFY_DEVICE));
	}

	return	TRUE;
}

BOOL CAtaSmart::GetSmartAttributePd(INT physicalDriveId, BYTE target, ATA_SMART_INFO* asi)
{
	BOOL	bRet = FALSE;
	HANDLE	hIoCtrl;
	DWORD	dwReturned;

	SMART_READ_DATA_OUTDATA	sendCmdOutParam;
	SENDCMDINPARAMS	sendCmd;

	if(m_FlagAtaPassThroughSmart)
	{
		DebugPrint(_T("SendAtaCommandPd - SMART_READ_DATA (ATA_PASS_THROUGH)"));
		bRet = SendAtaCommandPd(physicalDriveId, target, SMART_CMD, READ_ATTRIBUTES, 0x00, 
		(PBYTE)&(asi->SmartReadData), sizeof(asi->SmartReadData));
	}

	if(! bRet)
	{
		hIoCtrl = GetIoCtrlHandle(physicalDriveId);
		if(hIoCtrl == INVALID_HANDLE_VALUE)
		{
			return	FALSE;
		}

		::ZeroMemory(&sendCmdOutParam, sizeof(SMART_READ_DATA_OUTDATA));
		::ZeroMemory(&sendCmd, sizeof(SENDCMDINPARAMS));

		sendCmd.irDriveRegs.bFeaturesReg	= READ_ATTRIBUTES;
		sendCmd.irDriveRegs.bSectorCountReg = 1;
		sendCmd.irDriveRegs.bSectorNumberReg= 1;
		sendCmd.irDriveRegs.bCylLowReg		= SMART_CYL_LOW;
		sendCmd.irDriveRegs.bCylHighReg		= SMART_CYL_HI;
		sendCmd.irDriveRegs.bDriveHeadReg	= target;
		sendCmd.irDriveRegs.bCommandReg		= SMART_CMD;
		sendCmd.cBufferSize					= READ_ATTRIBUTE_BUFFER_SIZE;

		DebugPrint(_T("SendAtaCommandPd - SMART_READ_DATA"));
		bRet = ::DeviceIoControl(hIoCtrl, DFP_RECEIVE_DRIVE_DATA, 
			&sendCmd, sizeof(SENDCMDINPARAMS),
			&sendCmdOutParam, sizeof(SMART_READ_DATA_OUTDATA),
			&dwReturned, NULL);

		::CloseHandle(hIoCtrl);
	
		if(bRet == FALSE || dwReturned != sizeof(SMART_READ_DATA_OUTDATA))
		{
			return	FALSE;
		}

		memcpy_s(&(asi->SmartReadData), 512, &(sendCmdOutParam.SendCmdOutParam.bBuffer), 512);
	}

	return FillSmartInfo(asi);
}

BOOL CAtaSmart::GetSmartThresholdPd(INT physicalDriveId, BYTE target, ATA_SMART_INFO* asi)
{
	BOOL	bRet = FALSE;
	HANDLE	hIoCtrl;
	DWORD	dwReturned;

	SMART_READ_DATA_OUTDATA	sendCmdOutParam;
	SENDCMDINPARAMS	sendCmd;

	if(m_FlagAtaPassThroughSmart)
	{
		DebugPrint(_T("SendAtaCommandPd - SMART_READ_THRESHOLDS (ATA_PASS_THROUGH)"));
		bRet = SendAtaCommandPd(physicalDriveId, target, SMART_CMD, READ_THRESHOLDS, 0x00, 
			(PBYTE)&(asi->SmartReadThreshold), sizeof(asi->SmartReadThreshold));
	}

	if(bRet)
	{
		int j = 0;
		for(int i = 0; i < MAX_ATTRIBUTE; i++)
		{
			memcpy(	&(asi->Threshold[i]), 
					&(asi->SmartReadThreshold[i * sizeof(SMART_THRESHOLD) + 2]), sizeof(SMART_THRESHOLD));

			if(asi->Threshold[j].Id != 0)
			{
				j++;
			}
		}
	}
	else
	{
		hIoCtrl = GetIoCtrlHandle(physicalDriveId);
		if(hIoCtrl == INVALID_HANDLE_VALUE)
		{
			return	FALSE;
		}

		::ZeroMemory(&sendCmdOutParam, sizeof(SMART_READ_DATA_OUTDATA));
		::ZeroMemory(&sendCmd, sizeof(SENDCMDINPARAMS));

		sendCmd.irDriveRegs.bFeaturesReg	= READ_THRESHOLDS;
		sendCmd.irDriveRegs.bSectorCountReg = 1;
		sendCmd.irDriveRegs.bSectorNumberReg= 1;
		sendCmd.irDriveRegs.bCylLowReg		= SMART_CYL_LOW;
		sendCmd.irDriveRegs.bCylHighReg		= SMART_CYL_HI;
		sendCmd.irDriveRegs.bDriveHeadReg	= target;
		sendCmd.irDriveRegs.bCommandReg		= SMART_CMD;
		sendCmd.cBufferSize					= READ_THRESHOLD_BUFFER_SIZE;

		DebugPrint(_T("SendAtaCommandPd - SMART_READ_THRESHOLDS"));
		bRet = ::DeviceIoControl(hIoCtrl, DFP_RECEIVE_DRIVE_DATA, 
			&sendCmd, sizeof(SENDCMDINPARAMS),
			&sendCmdOutParam, sizeof(SMART_READ_DATA_OUTDATA),
			&dwReturned, NULL);

		::CloseHandle(hIoCtrl);
	
		if(bRet == FALSE || dwReturned != sizeof(SMART_READ_DATA_OUTDATA))
		{
			return	FALSE;
		}

		memcpy_s(&(asi->SmartReadThreshold), 512, &(sendCmdOutParam.SendCmdOutParam.bBuffer), 512);
		int j = 0;
		for(int i = 0; i < MAX_ATTRIBUTE; i++)
		{
			memcpy(	&(asi->Threshold[i]), 
					&(sendCmdOutParam.Data[i * sizeof(SMART_THRESHOLD) + 1]), sizeof(SMART_THRESHOLD));

			if(asi->Threshold[j].Id != 0)
			{
				j++;
			}
		}
	}
	
	return	TRUE;
}

BOOL CAtaSmart::ControlSmartStatusPd(INT physicalDriveId, BYTE target, BYTE command)
{
	BOOL	bRet = FALSE;
	HANDLE	hIoCtrl;
	DWORD	dwReturned;

	SENDCMDINPARAMS		sendCmd;
	SENDCMDOUTPARAMS	sendCmdOutParam;

	if(m_FlagAtaPassThroughSmart)
	{
		DebugPrint(_T("SendAtaCommandPd - SMART_CONTROL_STATUS (ATA_PASS_THROUGH)"));
		bRet = SendAtaCommandPd(physicalDriveId, target, SMART_CMD, command, 0x00, NULL, 0);
	}

	if(! bRet)
	{
		DebugPrint(_T("SendAtaCommandPd - SMART_CONTROL_STATUS"));
		hIoCtrl = GetIoCtrlHandle(physicalDriveId);
		if(hIoCtrl == INVALID_HANDLE_VALUE)
		{
			return	FALSE;
		}

		::ZeroMemory(&sendCmd, sizeof(SENDCMDINPARAMS));
		::ZeroMemory(&sendCmdOutParam, sizeof(SENDCMDOUTPARAMS));

		sendCmd.irDriveRegs.bFeaturesReg	= command;
		sendCmd.irDriveRegs.bSectorCountReg = 1;
		sendCmd.irDriveRegs.bSectorNumberReg= 1;
		sendCmd.irDriveRegs.bCylLowReg		= SMART_CYL_LOW;
		sendCmd.irDriveRegs.bCylHighReg		= SMART_CYL_HI;
		sendCmd.irDriveRegs.bDriveHeadReg	= target;
		sendCmd.irDriveRegs.bCommandReg		= SMART_CMD;
		sendCmd.cBufferSize					= 0;

		bRet = ::DeviceIoControl(hIoCtrl, DFP_SEND_DRIVE_COMMAND, 
			&sendCmd, sizeof(SENDCMDINPARAMS) - 1,
			&sendCmdOutParam, sizeof(SENDCMDOUTPARAMS) -1,
			&dwReturned, NULL);

		::CloseHandle(hIoCtrl);
	}
	
	return	bRet;
}

BOOL CAtaSmart::SendAtaCommandPd(INT physicalDriveId, BYTE target, BYTE main, BYTE sub, BYTE param, PBYTE data, DWORD dataSize)
{
	BOOL	bRet;
	HANDLE	hIoCtrl;
	DWORD	dwReturned;

	hIoCtrl = GetIoCtrlHandle(physicalDriveId);
	if(hIoCtrl == INVALID_HANDLE_VALUE)
	{
		return	FALSE;
	}

	if(m_FlagAtaPassThrough)
	{
		ATA_PASS_THROUGH_EX_WITH_BUFFERS ab;
		::ZeroMemory(&ab, sizeof(ab));
		ab.Apt.Length = sizeof(ATA_PASS_THROUGH_EX);
		ab.Apt.TimeOutValue = 2;
		DWORD size = offsetof(ATA_PASS_THROUGH_EX_WITH_BUFFERS, Buf);
		ab.Apt.DataBufferOffset = size;

		if(dataSize > 0)
		{
			if(dataSize > sizeof(ab.Buf))
			{
				return FALSE;
			}
			ab.Apt.AtaFlags = ATA_FLAGS_DATA_IN;
			ab.Apt.DataTransferLength = dataSize;
			ab.Buf[0] = 0xCF; // magic number
			size += dataSize;
		}

		ab.Apt.CurrentTaskFile.bFeaturesReg = sub;
		ab.Apt.CurrentTaskFile.bSectorCountReg = param;
		ab.Apt.CurrentTaskFile.bDriveHeadReg = target;
		ab.Apt.CurrentTaskFile.bCommandReg = main;

		if(main == SMART_CMD)
		{
			ab.Apt.CurrentTaskFile.bCylLowReg		= SMART_CYL_LOW;
			ab.Apt.CurrentTaskFile.bCylHighReg		= SMART_CYL_HI;
			ab.Apt.CurrentTaskFile.bSectorCountReg  = 1;
			ab.Apt.CurrentTaskFile.bSectorNumberReg = 1;
		}

		bRet = ::DeviceIoControl(hIoCtrl, IOCTL_ATA_PASS_THROUGH,
			&ab, size, &ab, size, &dwReturned, NULL);
		::CloseHandle(hIoCtrl);
		if(bRet && dataSize && data != NULL)
		{
			memcpy_s(data, dataSize, ab.Buf, dataSize);
		}
	}
	else if(m_Os.dwMajorVersion <= 4)
	{
		return FALSE;
	}
	else
	{
		DWORD size = sizeof(CMD_IDE_PATH_THROUGH) - 1 + dataSize;
		CMD_IDE_PATH_THROUGH* buf = (CMD_IDE_PATH_THROUGH*)VirtualAlloc(NULL, size, MEM_COMMIT, PAGE_READWRITE);

		buf->reg.bFeaturesReg		= sub;
		buf->reg.bSectorCountReg	= param;
		buf->reg.bSectorNumberReg	= 0;
		buf->reg.bCylLowReg			= 0;
		buf->reg.bCylHighReg		= 0;
		buf->reg.bDriveHeadReg		= target;
		buf->reg.bCommandReg		= main;
		buf->reg.bReserved		    = 0;
		buf->length					= dataSize;

		bRet = ::DeviceIoControl(hIoCtrl, IOCTL_IDE_PASS_THROUGH,
			buf, size, buf, size, &dwReturned, NULL);
		::CloseHandle(hIoCtrl);
		if(bRet && dataSize && data != NULL)
		{
			memcpy_s(data, dataSize, buf->buffer, dataSize);
		}
		VirtualFree(buf, 0, MEM_RELEASE);
	}

	return	bRet;
}

/*---------------------------------------------------------------------------*/
//  \\\\.\\ScsiX
/*---------------------------------------------------------------------------*/

BOOL CAtaSmart::DoIdentifyDeviceScsi(INT scsiPort, INT scsiTargetId, IDENTIFY_DEVICE* data)
{
	int done = FALSE;
	int controller = 0;
	int current = 0;
	HANDLE hScsiDriveIOCTL = 0;
	CString driveName;
	driveName.Format(_T("\\\\.\\Scsi%d:"), scsiPort);
	hScsiDriveIOCTL = CreateFile(driveName, GENERIC_READ | GENERIC_WRITE,
							FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if(hScsiDriveIOCTL != INVALID_HANDLE_VALUE)
	{
		BYTE buffer[sizeof(SRB_IO_CONTROL) + sizeof(SENDCMDOUTPARAMS) + IDENTIFY_BUFFER_SIZE];
		SRB_IO_CONTROL *p = (SRB_IO_CONTROL *)buffer;
		SENDCMDINPARAMS *pin = (SENDCMDINPARAMS *)(buffer + sizeof(SRB_IO_CONTROL));
		DWORD dummy;
		
		memset(buffer, 0, sizeof(buffer));
		p->HeaderLength = sizeof (SRB_IO_CONTROL);
		p->Timeout = 2;
		p->Length = sizeof(SENDCMDOUTPARAMS) + IDENTIFY_BUFFER_SIZE;
		p->ControlCode = IOCTL_SCSI_MINIPORT_IDENTIFY;
		memcpy((char *)p->Signature, "SCSIDISK", 8);
		pin->irDriveRegs.bCommandReg = ID_CMD;
		pin->bDriveNumber = scsiTargetId;
		
		if(DeviceIoControl(hScsiDriveIOCTL, IOCTL_SCSI_MINIPORT, 
								buffer, sizeof(SRB_IO_CONTROL) + sizeof(SENDCMDINPARAMS) - 1,
								buffer, sizeof(SRB_IO_CONTROL) + sizeof(SENDCMDOUTPARAMS) + IDENTIFY_BUFFER_SIZE,
								&dummy, NULL))
		{
			SENDCMDOUTPARAMS *pOut = (SENDCMDOUTPARAMS *)(buffer + sizeof(SRB_IO_CONTROL));
			if(*(pOut->bBuffer) > 0)
			{
				done = TRUE;
				memcpy_s(data, sizeof(IDENTIFY_DEVICE), pOut->bBuffer, sizeof(IDENTIFY_DEVICE));
			}
		}
		CloseHandle(hScsiDriveIOCTL);
	}
	return done;
}

BOOL CAtaSmart::GetSmartAttributeScsi(INT scsiPort, INT scsiTargetId, ATA_SMART_INFO* asi)
{
	HANDLE hScsiDriveIOCTL = 0;
	CString driveName;
	driveName.Format(_T("\\\\.\\Scsi%d:"), scsiPort);
	hScsiDriveIOCTL = CreateFile(driveName, GENERIC_READ | GENERIC_WRITE,
							FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if(hScsiDriveIOCTL != INVALID_HANDLE_VALUE)
	{
		BYTE buffer[sizeof(SRB_IO_CONTROL) + sizeof(SENDCMDOUTPARAMS) + READ_ATTRIBUTE_BUFFER_SIZE];
		SRB_IO_CONTROL *p = (SRB_IO_CONTROL *)buffer;
		SENDCMDINPARAMS *pin = (SENDCMDINPARAMS *)(buffer + sizeof(SRB_IO_CONTROL));
		DWORD dummy;
		memset(buffer, 0, sizeof(buffer));
		p->HeaderLength = sizeof(SRB_IO_CONTROL);
		p->Timeout = 2;
		p->Length = sizeof(SENDCMDOUTPARAMS) + READ_ATTRIBUTE_BUFFER_SIZE;
		p->ControlCode = IOCTL_SCSI_MINIPORT_READ_SMART_ATTRIBS;
		memcpy((char *)p->Signature, "SCSIDISK", 8);
		pin->irDriveRegs.bFeaturesReg		= READ_ATTRIBUTES;
		pin->irDriveRegs.bSectorCountReg	= 1;
		pin->irDriveRegs.bSectorNumberReg	= 1;
		pin->irDriveRegs.bCylLowReg			= SMART_CYL_LOW;
		pin->irDriveRegs.bCylHighReg		= SMART_CYL_HI;
		pin->irDriveRegs.bCommandReg		= SMART_CMD;
		pin->cBufferSize					= READ_ATTRIBUTE_BUFFER_SIZE;
		pin->bDriveNumber					= scsiTargetId;

		if(DeviceIoControl(hScsiDriveIOCTL, IOCTL_SCSI_MINIPORT, 
								buffer, sizeof(SRB_IO_CONTROL) + sizeof(SENDCMDINPARAMS) - 1,
								buffer, sizeof(SRB_IO_CONTROL) + sizeof(SENDCMDOUTPARAMS) + READ_ATTRIBUTE_BUFFER_SIZE,
								&dummy, NULL))
		{
			SENDCMDOUTPARAMS *pOut = (SENDCMDOUTPARAMS *)(buffer + sizeof(SRB_IO_CONTROL));

			memcpy_s(&(asi->SmartReadData), 512, &(pOut->bBuffer), 512);
			CloseHandle(hScsiDriveIOCTL);
			return FillSmartInfo(asi);
		}

	}
	return FALSE;
}

BOOL CAtaSmart::GetSmartThresholdScsi(INT scsiPort, INT scsiTargetId, ATA_SMART_INFO* asi)
{
	HANDLE hScsiDriveIOCTL = 0;
	CString driveName;
	driveName.Format(_T("\\\\.\\Scsi%d:"), scsiPort);
	hScsiDriveIOCTL = CreateFile(driveName, GENERIC_READ | GENERIC_WRITE,
							FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if(hScsiDriveIOCTL != INVALID_HANDLE_VALUE)
	{
		BYTE buffer[sizeof(SRB_IO_CONTROL) + sizeof(SENDCMDOUTPARAMS) + READ_THRESHOLD_BUFFER_SIZE];
		SRB_IO_CONTROL *p = (SRB_IO_CONTROL *)buffer;
		SENDCMDINPARAMS *pin = (SENDCMDINPARAMS *)(buffer + sizeof(SRB_IO_CONTROL));
		DWORD dummy;
		memset(buffer, 0, sizeof(buffer));
		p->HeaderLength = sizeof(SRB_IO_CONTROL);
		p->Timeout = 2;
		p->Length = sizeof(SENDCMDOUTPARAMS) + READ_THRESHOLD_BUFFER_SIZE;
		p->ControlCode = IOCTL_SCSI_MINIPORT_READ_SMART_THRESHOLDS;
		memcpy((char *)p->Signature, "SCSIDISK", 8);
		pin->irDriveRegs.bFeaturesReg		= READ_THRESHOLDS;
		pin->irDriveRegs.bSectorCountReg	= 1;
		pin->irDriveRegs.bSectorNumberReg	= 1;
		pin->irDriveRegs.bCylLowReg			= SMART_CYL_LOW;
		pin->irDriveRegs.bCylHighReg		= SMART_CYL_HI;
		pin->irDriveRegs.bCommandReg		= SMART_CMD;
		pin->cBufferSize					= READ_THRESHOLD_BUFFER_SIZE;
		pin->bDriveNumber					= scsiTargetId;

		if(DeviceIoControl(hScsiDriveIOCTL, IOCTL_SCSI_MINIPORT, 
								buffer, sizeof(SRB_IO_CONTROL) + sizeof(SENDCMDINPARAMS) - 1,
								buffer, sizeof(SRB_IO_CONTROL) + sizeof(SENDCMDOUTPARAMS) + READ_THRESHOLD_BUFFER_SIZE,
								&dummy, NULL))
		{
			SENDCMDOUTPARAMS *pOut = (SENDCMDOUTPARAMS *)(buffer + sizeof(SRB_IO_CONTROL));
			if(*(pOut->bBuffer) > 0)
			{
				int j = 0;
				memcpy_s(&(asi->SmartReadThreshold), 512, &(pOut->bBuffer), 512);
				for(int i = 0; i < MAX_ATTRIBUTE; i++)
				{
					memcpy(	&(asi->Threshold[j]), 
						&(pOut->bBuffer[i * sizeof(SMART_THRESHOLD) + 2]), sizeof(SMART_THRESHOLD));
					if(asi->Threshold[j].Id != 0)
					{
						j++;
					}
				}
			}
		}
	}
	CloseHandle (hScsiDriveIOCTL);
	
	if(asi->AttributeCount > 0)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

BOOL CAtaSmart::ControlSmartStatusScsi(INT scsiPort, INT scsiTargetId, BYTE command)
{
	BOOL	bRet;
	HANDLE hScsiDriveIOCTL = 0;
	CString driveName;
	driveName.Format(_T("\\\\.\\Scsi%d:"), scsiPort);
	hScsiDriveIOCTL = CreateFile(driveName, GENERIC_READ | GENERIC_WRITE,
							FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if(hScsiDriveIOCTL != INVALID_HANDLE_VALUE)
	{
		BYTE buffer[sizeof(SRB_IO_CONTROL) + sizeof(SENDCMDOUTPARAMS) + SCSI_MINIPORT_BUFFER_SIZE];
		SRB_IO_CONTROL *p = (SRB_IO_CONTROL *)buffer;
		SENDCMDINPARAMS *pin = (SENDCMDINPARAMS *)(buffer + sizeof(SRB_IO_CONTROL));
		DWORD dummy;
		memset(buffer, 0, sizeof(buffer));
		p->HeaderLength = sizeof(SRB_IO_CONTROL);
		p->Timeout = 2;
		p->Length = sizeof(SENDCMDOUTPARAMS) + SCSI_MINIPORT_BUFFER_SIZE;
		if(command == DISABLE_SMART)
		{
			p->ControlCode = IOCTL_SCSI_MINIPORT_DISABLE_SMART;
		}
		else
		{
			p->ControlCode = IOCTL_SCSI_MINIPORT_ENABLE_SMART;
		}
		memcpy((char *)p->Signature, "SCSIDISK", 8);
		pin->irDriveRegs.bFeaturesReg		= command;
		pin->irDriveRegs.bSectorCountReg	= 1;
		pin->irDriveRegs.bSectorNumberReg	= 1;
		pin->irDriveRegs.bCylLowReg			= SMART_CYL_LOW;
		pin->irDriveRegs.bCylHighReg		= SMART_CYL_HI;
		pin->irDriveRegs.bCommandReg		= SMART_CMD;
		pin->cBufferSize					= SCSI_MINIPORT_BUFFER_SIZE;
		pin->bDriveNumber					= scsiTargetId;

		bRet = DeviceIoControl(hScsiDriveIOCTL, IOCTL_SCSI_MINIPORT, 
								buffer, sizeof(SRB_IO_CONTROL) + sizeof(SENDCMDINPARAMS) - 1,
								buffer, sizeof(SRB_IO_CONTROL) + sizeof(SENDCMDOUTPARAMS) + SCSI_MINIPORT_BUFFER_SIZE,
								&dummy, NULL);
	}
	CloseHandle(hScsiDriveIOCTL);

	return bRet;
}


BOOL CAtaSmart::SendAtaCommandScsi(INT scsiPort, INT scsiTargetId, BYTE main, BYTE sub, BYTE param)
{
/** Does not work...
	BOOL	bRet;
	HANDLE hScsiDriveIOCTL = 0;
	CString driveName;
	driveName.Format(_T("\\\\.\\Scsi%d:"), scsiPort);
	hScsiDriveIOCTL = CreateFile(driveName, GENERIC_READ | GENERIC_WRITE,
							FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if(hScsiDriveIOCTL != INVALID_HANDLE_VALUE)
	{
		DWORD dummy;
		CMD_ATA_PASS_THROUGH_WITH_BUFFERS capt;
		::ZeroMemory(&capt, sizeof(CMD_ATA_PASS_THROUGH_WITH_BUFFERS));
		capt.apt.Length = sizeof(CMD_ATA_PASS_THROUGH);
		capt.apt.PathId = 0;
		capt.apt.TargetId = 0;
		capt.apt.Lun = 0;
		capt.apt.TimeOutValue = 10;

		DWORD size = offsetof(CMD_ATA_PASS_THROUGH_WITH_BUFFERS, DataBuf);
		capt.apt.DataBufferOffset = size;

		capt.apt.AtaFlags = 0x02;
		capt.apt.DataTransferLength = 512;
		size += 512;
		capt.DataBuf[0] = 0xCF;

		capt.apt.CurrentTaskFile.bFeaturesReg= sub;
		capt.apt.CurrentTaskFile.bSectorCountReg = param;
		capt.apt.CurrentTaskFile.bDriveHeadReg = 0xA0;
		capt.apt.CurrentTaskFile.bCommandReg = main;

		bRet = DeviceIoControl(hScsiDriveIOCTL, IOCTL_ATA_PASS_THROUGH, 
								&capt, size,
								&capt, size,
								&dummy, NULL);
	}
	CloseHandle(hScsiDriveIOCTL);

	return bRet;
*/
	return FALSE;
}

/*---------------------------------------------------------------------------*/
//  SCSI / ATA Translation (SAT) 
/*---------------------------------------------------------------------------*/

BOOL CAtaSmart::DoIdentifyDeviceSat(INT physicalDriveId, BYTE target, IDENTIFY_DEVICE* data, COMMAND_TYPE type)
{
	BOOL	bRet;
	HANDLE	hIoCtrl;
	DWORD	dwReturned;
	DWORD	length;

	SCSI_PASS_THROUGH_WITH_BUFFERS sptwb;

	if(data == NULL)
	{
		return	FALSE;
	}

	::ZeroMemory(data, sizeof(IDENTIFY_DEVICE));

	hIoCtrl = GetIoCtrlHandle(physicalDriveId);
	if(hIoCtrl == INVALID_HANDLE_VALUE)
	{
		return	FALSE;
	}

	::ZeroMemory(&sptwb,sizeof(SCSI_PASS_THROUGH_WITH_BUFFERS));

	sptwb.Spt.Length = sizeof(SCSI_PASS_THROUGH);
	sptwb.Spt.PathId = 0;
	sptwb.Spt.TargetId = 0;
	sptwb.Spt.Lun = 0;
	sptwb.Spt.SenseInfoLength = 24;
	sptwb.Spt.DataIn = SCSI_IOCTL_DATA_IN;
	sptwb.Spt.DataTransferLength = IDENTIFY_BUFFER_SIZE;
	sptwb.Spt.TimeOutValue = 2;
	sptwb.Spt.DataBufferOffset = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS, DataBuf);
	sptwb.Spt.SenseInfoOffset = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS, SenseBuf);

// DEBUG
//	CString cstr;
//	cstr.Format(_T("DoIdentifyDevice TYPE=%d"), (DWORD)type);
//	DebugPrint(cstr);

	if(type == CMD_TYPE_SAT)
	{
		sptwb.Spt.CdbLength = 12;
		sptwb.Spt.Cdb[0] = 0xA1;//ATA PASS THROUGH(12) OPERATION CODE(A1h)
		sptwb.Spt.Cdb[1] = (4 << 1) | 0; //MULTIPLE_COUNT=0,PROTOCOL=4(PIO Data-In),Reserved
		sptwb.Spt.Cdb[2] = (1 << 3) | (1 << 2) | 2;//OFF_LINE=0,CK_COND=0,Reserved=0,T_DIR=1(ToDevice),BYTE_BLOCK=1,T_LENGTH=2
		sptwb.Spt.Cdb[3] = 0;//FEATURES (7:0)
		sptwb.Spt.Cdb[4] = 1;//SECTOR_COUNT (7:0)
		sptwb.Spt.Cdb[5] = 0;//LBA_LOW (7:0)
		sptwb.Spt.Cdb[6] = 0;//LBA_MID (7:0)
		sptwb.Spt.Cdb[7] = 0;//LBA_HIGH (7:0)
		sptwb.Spt.Cdb[8] = target;
		sptwb.Spt.Cdb[9] = ID_CMD;//COMMAND
	}
	else if(type == CMD_TYPE_SUNPLUS)
	{
		sptwb.Spt.CdbLength = 12;
		sptwb.Spt.Cdb[0] = 0xF8;
		sptwb.Spt.Cdb[1] = 0x00;
		sptwb.Spt.Cdb[2] = 0x22;
		sptwb.Spt.Cdb[3] = 0x10;
		sptwb.Spt.Cdb[4] = 0x01;
		sptwb.Spt.Cdb[5] = 0x00; 
		sptwb.Spt.Cdb[6] = 0x01; 
		sptwb.Spt.Cdb[7] = 0x00; 
		sptwb.Spt.Cdb[8] = 0x00;
		sptwb.Spt.Cdb[9] = 0x00;
		sptwb.Spt.Cdb[10] = target; 
		sptwb.Spt.Cdb[11] = 0xEC; // ID_CMD
	}
	else if(type == CMD_TYPE_IO_DATA)
	{
		sptwb.Spt.CdbLength = 12;
		sptwb.Spt.Cdb[0] = 0xE3;
		sptwb.Spt.Cdb[1] = 0x00;
		sptwb.Spt.Cdb[2] = 0x00;
		sptwb.Spt.Cdb[3] = 0x01;
		sptwb.Spt.Cdb[4] = 0x01;
		sptwb.Spt.Cdb[5] = 0x00; 
		sptwb.Spt.Cdb[6] = 0x00; 
		sptwb.Spt.Cdb[7] = target;
		sptwb.Spt.Cdb[8] = 0xEC;  // ID_CMD
		sptwb.Spt.Cdb[9] = 0x00;
		sptwb.Spt.Cdb[10] = 0x00; 
		sptwb.Spt.Cdb[11] = 0x00;
	}
	else if(type == CMD_TYPE_LOGITEC)
	{
		sptwb.Spt.CdbLength = 10;
		sptwb.Spt.Cdb[0] = 0xE0;
		sptwb.Spt.Cdb[1] = 0x00;
		sptwb.Spt.Cdb[2] = 0x00;
		sptwb.Spt.Cdb[3] = 0x00;
		sptwb.Spt.Cdb[4] = 0x00;
		sptwb.Spt.Cdb[5] = 0x00; 
		sptwb.Spt.Cdb[6] = 0x00; 
		sptwb.Spt.Cdb[7] = target; 
		sptwb.Spt.Cdb[8] = 0xEC;  // ID_CMD
		sptwb.Spt.Cdb[9] = 0x4C;
	}
	else if(type == CMD_TYPE_JMICRON)
	{
		sptwb.Spt.CdbLength = 12;
		sptwb.Spt.Cdb[0] = 0xDF;
		sptwb.Spt.Cdb[1] = 0x10;
		sptwb.Spt.Cdb[2] = 0x00;
		sptwb.Spt.Cdb[3] = 0x02;
		sptwb.Spt.Cdb[4] = 0x00;
		sptwb.Spt.Cdb[5] = 0x00; 
		sptwb.Spt.Cdb[6] = 0x01; 
		sptwb.Spt.Cdb[7] = 0x00; 
		sptwb.Spt.Cdb[8] = 0x00;
		sptwb.Spt.Cdb[9] = 0x00;
		sptwb.Spt.Cdb[10] = target; 
		sptwb.Spt.Cdb[11] = 0xEC; // ID_CMD
	}
	else if(type == CMD_TYPE_CYPRESS)
	{
		sptwb.Spt.CdbLength = 16;
		sptwb.Spt.Cdb[0] = 0x24;
		sptwb.Spt.Cdb[1] = 0x24;
		sptwb.Spt.Cdb[2] = 0x00;
		sptwb.Spt.Cdb[3] = 0xBE;
		sptwb.Spt.Cdb[4] = 0x01;
		sptwb.Spt.Cdb[5] = 0x00; 
		sptwb.Spt.Cdb[6] = 0x00; 
		sptwb.Spt.Cdb[7] = 0x01; 
		sptwb.Spt.Cdb[8] = 0x00;
		sptwb.Spt.Cdb[9] = 0x00;
		sptwb.Spt.Cdb[10] = 0x00; 
		sptwb.Spt.Cdb[11] = target;
		sptwb.Spt.Cdb[12] = 0xEC; // ID_CMD
		sptwb.Spt.Cdb[13] = 0x00;
		sptwb.Spt.Cdb[14] = 0x00;
		sptwb.Spt.Cdb[15] = 0x00;
	}
/*
	else if(type == CMD_TYPE_DEBUG)
	{
		sptwb.Spt.CdbLength = 16;
		for(int i = 0xA0; i <= 0xFF; i++)
		{
			for(int j = 8; j < 16; j++)
			{
				::ZeroMemory(&sptwb.Spt.Cdb, 16);
				sptwb.Spt.Cdb[0] = i;
				sptwb.Spt.Cdb[j - 1] = 0xA0;
				sptwb.Spt.Cdb[j] = 0xEC;

				length = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS, DataBuf) + sptwb.Spt.DataTransferLength;

				bRet = ::DeviceIoControl(hIoCtrl, IOCTL_SCSI_PASS_THROUGH, 
					&sptwb, sizeof(SCSI_PASS_THROUGH),
					&sptwb, length,	&dwReturned, NULL);

				
				if(bRet == FALSE || dwReturned != length)
				{
					continue;
				}

				CString cstr;
				cstr.Format(_T("i = %d, j = %d"), i, j);
				AfxMessageBox(cstr);

				::CloseHandle(hIoCtrl);
				memcpy_s(data, sizeof(IDENTIFY_DEVICE), sptwb.DataBuf, sizeof(IDENTIFY_DEVICE));

				return TRUE;
			}
		}
	}
*/
	else
	{
		return FALSE;
	}

	length = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS, DataBuf) + sptwb.Spt.DataTransferLength;

	bRet = ::DeviceIoControl(hIoCtrl, IOCTL_SCSI_PASS_THROUGH, 
		&sptwb, sizeof(SCSI_PASS_THROUGH),
		&sptwb, length,	&dwReturned, NULL);

	::CloseHandle(hIoCtrl);
	
	if(bRet == FALSE || dwReturned != length)
	{
		return	FALSE;
	}

	memcpy_s(data, sizeof(IDENTIFY_DEVICE), sptwb.DataBuf, sizeof(IDENTIFY_DEVICE));

	return	TRUE;
}

BOOL CAtaSmart::GetSmartAttributeSat(INT PhysicalDriveId, BYTE target, ATA_SMART_INFO* asi)
{
	BOOL	bRet;
	HANDLE	hIoCtrl;
	DWORD	dwReturned;
	DWORD length;

	SCSI_PASS_THROUGH_WITH_BUFFERS sptwb;

	hIoCtrl = GetIoCtrlHandle(PhysicalDriveId);
	if(hIoCtrl == INVALID_HANDLE_VALUE)
	{
		return	FALSE;
	}

	::ZeroMemory(&sptwb,sizeof(SCSI_PASS_THROUGH_WITH_BUFFERS));

	sptwb.Spt.Length = sizeof(SCSI_PASS_THROUGH);
	sptwb.Spt.PathId = 0;
	sptwb.Spt.TargetId = 0;
	sptwb.Spt.Lun = 0;
	sptwb.Spt.SenseInfoLength = 24;
	sptwb.Spt.DataIn = SCSI_IOCTL_DATA_IN;
	sptwb.Spt.DataTransferLength = READ_ATTRIBUTE_BUFFER_SIZE;
	sptwb.Spt.TimeOutValue = 2;
	sptwb.Spt.DataBufferOffset = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS, DataBuf);
	sptwb.Spt.SenseInfoOffset = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS, SenseBuf);

// DEBUG
//	CString cstr;
//	cstr.Format(_T("SmartAttribute TYPE=%d"), asi->CommandType);
//	DebugPrint(cstr);

	COMMAND_TYPE type = asi->CommandType;
	if(type == CMD_TYPE_SAT)
	{
		sptwb.Spt.CdbLength = 12;
		sptwb.Spt.Cdb[0] = 0xA1;//ATA PASS THROUGH(12) OPERATION CODE(A1h)
		sptwb.Spt.Cdb[1] = (4 << 1) | 0; //MULTIPLE_COUNT=0,PROTOCOL=4(PIO Data-In),Reserved
		sptwb.Spt.Cdb[2] = (1 << 3) | (1 << 2) | 2;//OFF_LINE=0,CK_COND=0,Reserved=0,T_DIR=1(ToDevice),BYTE_BLOCK=1,T_LENGTH=2
		sptwb.Spt.Cdb[3] = READ_ATTRIBUTES;//FEATURES (7:0)
		sptwb.Spt.Cdb[4] = 1;//SECTOR_COUNT (7:0)
		sptwb.Spt.Cdb[5] = 1;//LBA_LOW (7:0)
		sptwb.Spt.Cdb[6] = SMART_CYL_LOW;//LBA_MID (7:0)
		sptwb.Spt.Cdb[7] = SMART_CYL_HI;//LBA_HIGH (7:0)
		sptwb.Spt.Cdb[8] = target;
		sptwb.Spt.Cdb[9] = SMART_CMD;//COMMAND
	}
	else if(type == CMD_TYPE_SUNPLUS)
	{
		sptwb.Spt.CdbLength = 12;
		sptwb.Spt.Cdb[0] = 0xF8;
		sptwb.Spt.Cdb[1] = 0x00;
		sptwb.Spt.Cdb[2] = 0x22;
		sptwb.Spt.Cdb[3] = 0x10;
		sptwb.Spt.Cdb[4] = 0x01;
		sptwb.Spt.Cdb[5] = 0xD0;  // READ_ATTRIBUTES
		sptwb.Spt.Cdb[6] = 0x01; 
		sptwb.Spt.Cdb[7] = 0x00; 
		sptwb.Spt.Cdb[8] = 0x4F;  // SMART_CYL_LOW 
		sptwb.Spt.Cdb[9] = 0xC2;  // SMART_CYL_HIGH
		sptwb.Spt.Cdb[10] = target; 
		sptwb.Spt.Cdb[11] = 0xB0; // SMART_CMD
	}
	else if(type == CMD_TYPE_IO_DATA)
	{
		sptwb.Spt.CdbLength = 12;
		sptwb.Spt.Cdb[0] = 0xE3;
		sptwb.Spt.Cdb[1] = 0x00; // ?
		sptwb.Spt.Cdb[2] = 0xD0; // READ_ATTRIBUTES
		sptwb.Spt.Cdb[3] = 0x00; // ?
		sptwb.Spt.Cdb[4] = 0x00; // ?
		sptwb.Spt.Cdb[5] = 0x4F; // SMART_CYL_LOW
		sptwb.Spt.Cdb[6] = 0xC2; // SMART_CYL_HIGH
		sptwb.Spt.Cdb[7] = target; // 
		sptwb.Spt.Cdb[8] = 0xB0; // SMART_CMD
		sptwb.Spt.Cdb[9] = 0x00;  
		sptwb.Spt.Cdb[10] = 0x00; 
		sptwb.Spt.Cdb[11] = 0x00;
	}
	else if(type == CMD_TYPE_LOGITEC)
	{
		sptwb.Spt.CdbLength = 10;
		sptwb.Spt.Cdb[0] = 0xE0;
		sptwb.Spt.Cdb[1] = 0x00;
		sptwb.Spt.Cdb[2] = 0xD0; // READ_ATTRIBUTES
		sptwb.Spt.Cdb[3] = 0x00; 
		sptwb.Spt.Cdb[4] = 0x00;
		sptwb.Spt.Cdb[5] = 0x4F; // SMART_CYL_LOW
		sptwb.Spt.Cdb[6] = 0xC2; // SMART_CYL_HIGH
		sptwb.Spt.Cdb[7] = target; 
		sptwb.Spt.Cdb[8] = 0xB0; // SMART_CMD
		sptwb.Spt.Cdb[9] = 0x4C;
	}
	else if(type == CMD_TYPE_JMICRON)
	{
		sptwb.Spt.CdbLength = 12;
		sptwb.Spt.Cdb[0] = 0xDF;
		sptwb.Spt.Cdb[1] = 0x10;
		sptwb.Spt.Cdb[2] = 0x00;
		sptwb.Spt.Cdb[3] = 0x02;
		sptwb.Spt.Cdb[4] = 0x00;
		sptwb.Spt.Cdb[5] = 0xD0;  // READ_ATTRIBUTES
		sptwb.Spt.Cdb[6] = 0x01; 
		sptwb.Spt.Cdb[7] = 0x01; 
		sptwb.Spt.Cdb[8] = 0x4F;  // SMART_CYL_LOW
		sptwb.Spt.Cdb[9] = 0xC2;  // SMART_CYL_HIGH
		sptwb.Spt.Cdb[10] = target; 
		sptwb.Spt.Cdb[11] = 0xB0; // SMART_CMD
	}
	else if(type == CMD_TYPE_CYPRESS)
	{
		sptwb.Spt.CdbLength = 16;
		sptwb.Spt.Cdb[0] = 0x24;
		sptwb.Spt.Cdb[1] = 0x24;
		sptwb.Spt.Cdb[2] = 0x00;
		sptwb.Spt.Cdb[3] = 0xBE;
		sptwb.Spt.Cdb[4] = 0x01;
		sptwb.Spt.Cdb[5] = 0x00; 
		sptwb.Spt.Cdb[6] = 0xD0;  // READ_ATTRIBUTES
		sptwb.Spt.Cdb[7] = 0x00; 
		sptwb.Spt.Cdb[8] = 0x00;
		sptwb.Spt.Cdb[9] = 0x4F;  // SMART_CYL_LOW
		sptwb.Spt.Cdb[10] = 0xC2; // SMART_CYL_HIGH
		sptwb.Spt.Cdb[11] = target;
		sptwb.Spt.Cdb[12] = 0xB0; // ID_CMD
		sptwb.Spt.Cdb[13] = 0x00;
		sptwb.Spt.Cdb[14] = 0x00;
		sptwb.Spt.Cdb[15] = 0x00;
	}
	else
	{
		return FALSE;
	}

	length = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS, DataBuf) + sptwb.Spt.DataTransferLength;
	bRet = ::DeviceIoControl(hIoCtrl, IOCTL_SCSI_PASS_THROUGH, 
		&sptwb, sizeof(SCSI_PASS_THROUGH),
		&sptwb, length,	&dwReturned, NULL);

	::CloseHandle(hIoCtrl);
	
	if(bRet == FALSE || dwReturned != length)
	{
		return	FALSE;
	}

	CString str;
	asi->AttributeCount = 0;
	int j = 0;

	memcpy_s(&(asi->SmartReadData), 512, &(sptwb.DataBuf), 512);
	return FillSmartInfo(asi);
}

BOOL CAtaSmart::GetSmartThresholdSat(INT physicalDriveId, BYTE target, ATA_SMART_INFO* asi)
{
	BOOL	bRet;
	HANDLE	hIoCtrl;
	DWORD	dwReturned;
	DWORD length;

	SCSI_PASS_THROUGH_WITH_BUFFERS sptwb;

	hIoCtrl = GetIoCtrlHandle(physicalDriveId);
	if(hIoCtrl == INVALID_HANDLE_VALUE)
	{
		return	FALSE;
	}

	::ZeroMemory(&sptwb,sizeof(SCSI_PASS_THROUGH_WITH_BUFFERS));

	sptwb.Spt.Length = sizeof(SCSI_PASS_THROUGH);
	sptwb.Spt.PathId = 0;
	sptwb.Spt.TargetId = 0;
	sptwb.Spt.Lun = 0;
	sptwb.Spt.SenseInfoLength = 24;
	sptwb.Spt.DataIn = SCSI_IOCTL_DATA_IN;
	sptwb.Spt.DataTransferLength = READ_THRESHOLD_BUFFER_SIZE;
	sptwb.Spt.TimeOutValue = 2;
	sptwb.Spt.DataBufferOffset = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS, DataBuf);
	sptwb.Spt.SenseInfoOffset = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS, SenseBuf);

// DEBUG
//	CString cstr;
//	cstr.Format(_T("SmartThreshold TYPE=%d"), asi->CommandType);
//	DebugPrint(cstr);

	COMMAND_TYPE type = asi->CommandType;
	if(type == CMD_TYPE_SAT)
	{
		sptwb.Spt.CdbLength = 12;
		sptwb.Spt.Cdb[0] = 0xA1; ////ATA PASS THROUGH(12) OPERATION CODE (A1h)
		sptwb.Spt.Cdb[1] = (4 << 1) | 0; //MULTIPLE_COUNT=0,PROTOCOL=4(PIO Data-In),Reserved
		sptwb.Spt.Cdb[2] = (1 << 3) | (1 << 2) | 2;//OFF_LINE=0,CK_COND=0,Reserved=0,T_DIR=1(ToDevice),BYTE_BLOCK=1,T_LENGTH=2
		sptwb.Spt.Cdb[3] = READ_THRESHOLDS;//FEATURES (7:0)
		sptwb.Spt.Cdb[4] = 1;//SECTOR_COUNT (7:0)
		sptwb.Spt.Cdb[5] = 1;//LBA_LOW (7:0)
		sptwb.Spt.Cdb[6] = SMART_CYL_LOW;//LBA_MID (7:0)
		sptwb.Spt.Cdb[7] = SMART_CYL_HI;//LBA_HIGH (7:0)
		sptwb.Spt.Cdb[8] = target;
		sptwb.Spt.Cdb[9] = SMART_CMD;//COMMAND
	}
	else if(type == CMD_TYPE_SUNPLUS)
	{
		sptwb.Spt.CdbLength = 12;
		sptwb.Spt.Cdb[0] = 0xF8;
		sptwb.Spt.Cdb[1] = 0x00;
		sptwb.Spt.Cdb[2] = 0x22;
		sptwb.Spt.Cdb[3] = 0x10;
		sptwb.Spt.Cdb[4] = 0x01;
		sptwb.Spt.Cdb[5] = 0xD1; // READ_THRESHOLD
		sptwb.Spt.Cdb[6] = 0x01; 
		sptwb.Spt.Cdb[7] = 0x01; 
		sptwb.Spt.Cdb[8] = 0x4F; // SMART_CYL_LOW
		sptwb.Spt.Cdb[9] = 0xC2; // SMART_CYL_HIGH
		sptwb.Spt.Cdb[10] = target; 
		sptwb.Spt.Cdb[11] = 0xB0;// SMART_CMD
	}
	else if(type == CMD_TYPE_IO_DATA)
	{
		sptwb.Spt.CdbLength = 12;
		sptwb.Spt.Cdb[0] = 0xE3;
		sptwb.Spt.Cdb[1] = 0x00; // ?
		sptwb.Spt.Cdb[2] = 0xD1; // READ_THRESHOLD
		sptwb.Spt.Cdb[3] = 0x00; // ?
		sptwb.Spt.Cdb[4] = 0x00; // ?
		sptwb.Spt.Cdb[5] = 0x4F; // SMART_CYL_LOW 
		sptwb.Spt.Cdb[6] = 0xC2; // SMART_CYL_HIGH
		sptwb.Spt.Cdb[7] = target; // 
		sptwb.Spt.Cdb[8] = 0xB0; // SMART_CMD
		sptwb.Spt.Cdb[9] = 0x00;  
		sptwb.Spt.Cdb[10] = 0x00; 
		sptwb.Spt.Cdb[11] = 0x00;
	}
	else if(type == CMD_TYPE_LOGITEC)
	{
		sptwb.Spt.CdbLength = 10;
		sptwb.Spt.Cdb[0] = 0xE0;
		sptwb.Spt.Cdb[1] = 0x00;
		sptwb.Spt.Cdb[2] = 0xD1; // READ_THRESHOLD
		sptwb.Spt.Cdb[3] = 0x00;
		sptwb.Spt.Cdb[4] = 0x00;
		sptwb.Spt.Cdb[5] = 0x4F; // SMART_CYL_LOW
		sptwb.Spt.Cdb[6] = 0xC2; // SMART_CYL_HIGH
		sptwb.Spt.Cdb[7] = target; 
		sptwb.Spt.Cdb[8] = 0xB0; // SMART_CMD
		sptwb.Spt.Cdb[9] = 0x4C;
	}
	else if(type == CMD_TYPE_JMICRON)
	{
		sptwb.Spt.CdbLength = 12;
		sptwb.Spt.Cdb[0] = 0xDF;
		sptwb.Spt.Cdb[1] = 0x10;
		sptwb.Spt.Cdb[2] = 0x00;
		sptwb.Spt.Cdb[3] = 0x02;
		sptwb.Spt.Cdb[4] = 0x00;
		sptwb.Spt.Cdb[5] = 0xD1;  // READ_THRESHOLD
		sptwb.Spt.Cdb[6] = 0x01; 
		sptwb.Spt.Cdb[7] = 0x01; 
		sptwb.Spt.Cdb[8] = 0x4F;  // SMART_CYL_LOW
		sptwb.Spt.Cdb[9] = 0xC2;  // SMART_CYL_HIGH
		sptwb.Spt.Cdb[10] = target; 
		sptwb.Spt.Cdb[11] = 0xB0; // SMART_CMD
	}
	else if(type == CMD_TYPE_CYPRESS)
	{
		sptwb.Spt.CdbLength = 16;
		sptwb.Spt.Cdb[0] = 0x24;
		sptwb.Spt.Cdb[1] = 0x24;
		sptwb.Spt.Cdb[2] = 0x00;
		sptwb.Spt.Cdb[3] = 0xBE;
		sptwb.Spt.Cdb[4] = 0x01;
		sptwb.Spt.Cdb[5] = 0x00; 
		sptwb.Spt.Cdb[6] = 0xD1;  // READ_THRESHOLD  
		sptwb.Spt.Cdb[7] = 0x00; 
		sptwb.Spt.Cdb[8] = 0x00;
		sptwb.Spt.Cdb[9] = 0x4F;  // SMART_CYL_LOW
		sptwb.Spt.Cdb[10] = 0xC2; // SMART_CYL_HIGH
		sptwb.Spt.Cdb[11] = target;
		sptwb.Spt.Cdb[12] = 0xB0; // ID_CMD
		sptwb.Spt.Cdb[13] = 0x00;
		sptwb.Spt.Cdb[14] = 0x00;
		sptwb.Spt.Cdb[15] = 0x00;
	}
	else
	{
		return FALSE;
	}

	length = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS, DataBuf) + sptwb.Spt.DataTransferLength;
	bRet = ::DeviceIoControl(hIoCtrl, IOCTL_SCSI_PASS_THROUGH, 
		&sptwb, sizeof(SCSI_PASS_THROUGH),
		&sptwb, length,	&dwReturned, NULL);
	
	::CloseHandle(hIoCtrl);
	
	if(bRet == FALSE || dwReturned != length)
	{
		return	FALSE;
	}

	CString str;
	int j = 0;

	memcpy_s(&(asi->SmartReadThreshold), 512, &(sptwb.DataBuf), 512);

	for(int i = 0; i < MAX_ATTRIBUTE; i++)
	{
		memcpy(	&(asi->Threshold[i]), 
				&(sptwb.DataBuf[i * sizeof(SMART_THRESHOLD) + 2]), sizeof(SMART_THRESHOLD));

		if(asi->Threshold[j].Id != 0)
		{
			j++;
		}
	}

	return	TRUE;
}

BOOL CAtaSmart::ControlSmartStatusSat(INT physicalDriveId, BYTE target, BYTE command, COMMAND_TYPE type)
{
	BOOL	bRet;
	HANDLE	hIoCtrl;
	DWORD	dwReturned;

	SCSI_PASS_THROUGH_WITH_BUFFERS sptwb;

	hIoCtrl = GetIoCtrlHandle(physicalDriveId);
	if(hIoCtrl == INVALID_HANDLE_VALUE)
	{
		return	FALSE;
	}

	::ZeroMemory(&sptwb,sizeof(SCSI_PASS_THROUGH_WITH_BUFFERS));

	sptwb.Spt.Length = sizeof(SCSI_PASS_THROUGH);
	sptwb.Spt.PathId = 0;
	sptwb.Spt.TargetId = 0;
	sptwb.Spt.Lun = 0;
	sptwb.Spt.SenseInfoLength = 24;
	sptwb.Spt.DataIn = SCSI_IOCTL_DATA_IN;
	sptwb.Spt.DataTransferLength = 0;
	sptwb.Spt.TimeOutValue = 2;
	sptwb.Spt.DataBufferOffset = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS, DataBuf);
	sptwb.Spt.SenseInfoOffset = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS, SenseBuf);
	if(type == CMD_TYPE_SAT)
	{
		sptwb.Spt.CdbLength = 12;
		sptwb.Spt.Cdb[0] = 0xA1; //ATA PASS THROUGH (12) OPERATION CODE (A1h)
		sptwb.Spt.Cdb[1] = (3 << 1) | 0; //MULTIPLE_COUNT=0,PROTOCOL=3(Non-Data),Reserved
		sptwb.Spt.Cdb[2] = (1 << 3) | (1 << 2) | 2;//OFF_LINE=0,CK_COND=0,Reserved=0,T_DIR=1(ToDevice),BYTE_BLOCK=1,T_LENGTH=2
		sptwb.Spt.Cdb[3] = command;//FEATURES (7:0)
		sptwb.Spt.Cdb[4] = 0;//SECTOR_COUNT (7:0)
		sptwb.Spt.Cdb[5] = 1;//LBA_LOW (7:0)
		sptwb.Spt.Cdb[6] = SMART_CYL_LOW;//LBA_MID (7:0)
		sptwb.Spt.Cdb[7] = SMART_CYL_HI;//LBA_HIGH (7:0)
		sptwb.Spt.Cdb[8] = target;
		sptwb.Spt.Cdb[9] = SMART_CMD;//COMMAND
	}
	else if(type == CMD_TYPE_SUNPLUS)
	{
		sptwb.Spt.CdbLength = 12;
		sptwb.Spt.Cdb[0] = 0xF8;
		sptwb.Spt.Cdb[1] = 0x00;
		sptwb.Spt.Cdb[2] = 0x22;
		sptwb.Spt.Cdb[3] = 0x10;
		sptwb.Spt.Cdb[4] = 0x01;
		sptwb.Spt.Cdb[5] = command;
		sptwb.Spt.Cdb[6] = 0x01; 
		sptwb.Spt.Cdb[7] = 0x00; 
		sptwb.Spt.Cdb[8] = 0x4F;  // SMART_CYL_LOW 
		sptwb.Spt.Cdb[9] = 0xC2;  // SMART_CYL_HIGH
		sptwb.Spt.Cdb[10] = target; 
		sptwb.Spt.Cdb[11] = 0xB0; // SMART_CMD
	}
	else if(type == CMD_TYPE_IO_DATA)
	{
		sptwb.Spt.CdbLength = 12;
		sptwb.Spt.Cdb[0] = 0xE3;
		sptwb.Spt.Cdb[1] = 0x00; // ?
		sptwb.Spt.Cdb[2] = command;
		sptwb.Spt.Cdb[3] = 0x00; // ?
		sptwb.Spt.Cdb[4] = 0x00; // ?
		sptwb.Spt.Cdb[5] = 0x4F; // SMART_CYL_LOW
		sptwb.Spt.Cdb[6] = 0xC2; // SMART_CYL_HIGH
		sptwb.Spt.Cdb[7] = target; // 
		sptwb.Spt.Cdb[8] = 0xB0; // SMART_CMD
		sptwb.Spt.Cdb[9] = 0x00;  
		sptwb.Spt.Cdb[10] = 0x00; 
		sptwb.Spt.Cdb[11] = 0x00;
	}
	else if(type == CMD_TYPE_LOGITEC)
	{
		sptwb.Spt.CdbLength = 10;
		sptwb.Spt.Cdb[0] = 0xE0;
		sptwb.Spt.Cdb[1] = 0x00;
		sptwb.Spt.Cdb[2] = command;
		sptwb.Spt.Cdb[3] = 0x00; 
		sptwb.Spt.Cdb[4] = 0x00;
		sptwb.Spt.Cdb[5] = 0x4F; // SMART_CYL_LOW
		sptwb.Spt.Cdb[6] = 0xC2; // SMART_CYL_HIGH
		sptwb.Spt.Cdb[7] = target; 
		sptwb.Spt.Cdb[8] = 0xB0; // SMART_CMD
		sptwb.Spt.Cdb[9] = 0x4C;
	}
	else if(type == CMD_TYPE_JMICRON)
	{
		sptwb.Spt.CdbLength = 12;
		sptwb.Spt.Cdb[0] = 0xDF;
		sptwb.Spt.Cdb[1] = 0x10;
		sptwb.Spt.Cdb[2] = 0x00;
		sptwb.Spt.Cdb[3] = 0x02;
		sptwb.Spt.Cdb[4] = 0x00;
		sptwb.Spt.Cdb[5] = command;
		sptwb.Spt.Cdb[6] = 0x01; 
		sptwb.Spt.Cdb[7] = 0x01; 
		sptwb.Spt.Cdb[8] = 0x4F;  // SMART_CYL_LOW
		sptwb.Spt.Cdb[9] = 0xC2;  // SMART_CYL_HIGH
		sptwb.Spt.Cdb[10] = target; 
		sptwb.Spt.Cdb[11] = 0xB0; // SMART_CMD
	}
	else if(type == CMD_TYPE_CYPRESS)
	{
		sptwb.Spt.CdbLength = 16;
		sptwb.Spt.Cdb[0] = 0x24;
		sptwb.Spt.Cdb[1] = 0x24;
		sptwb.Spt.Cdb[2] = 0x00;
		sptwb.Spt.Cdb[3] = 0xBE;
		sptwb.Spt.Cdb[4] = 0x00;
		sptwb.Spt.Cdb[5] = 0x00; 
		sptwb.Spt.Cdb[6] = command; 
		sptwb.Spt.Cdb[7] = 0x00; 
		sptwb.Spt.Cdb[8] = 0x00;
		sptwb.Spt.Cdb[9] = 0x4F;  // SMART_CYL_LOW
		sptwb.Spt.Cdb[10] = 0xC2; // SMART_CYL_HIGH
		sptwb.Spt.Cdb[11] = target;
		sptwb.Spt.Cdb[12] = 0xB0; // ID_CMD
		sptwb.Spt.Cdb[13] = 0x00;
		sptwb.Spt.Cdb[14] = 0x00;
		sptwb.Spt.Cdb[15] = 0x00;
	}
	else
	{
		return FALSE;
	}

	DWORD length = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS, DataBuf) + sptwb.Spt.DataTransferLength;
	bRet = ::DeviceIoControl(hIoCtrl, IOCTL_SCSI_PASS_THROUGH, 
		&sptwb, sizeof(SCSI_PASS_THROUGH),
		&sptwb, length,	&dwReturned, NULL);

	::CloseHandle(hIoCtrl);

	return	bRet;
}

BOOL CAtaSmart::SendAtaCommandSat(INT physicalDriveId, BYTE target, BYTE main, BYTE sub, BYTE param, COMMAND_TYPE type)
{
	BOOL	bRet;
	HANDLE	hIoCtrl;
	DWORD	dwReturned;

	SCSI_PASS_THROUGH_WITH_BUFFERS sptwb;

	hIoCtrl = GetIoCtrlHandle(physicalDriveId);
	if(hIoCtrl == INVALID_HANDLE_VALUE)
	{
		return	FALSE;
	}

	::ZeroMemory(&sptwb,sizeof(SCSI_PASS_THROUGH_WITH_BUFFERS));

	sptwb.Spt.Length = sizeof(SCSI_PASS_THROUGH);
	sptwb.Spt.PathId = 0;
	sptwb.Spt.TargetId = 0;
	sptwb.Spt.Lun = 0;
	sptwb.Spt.SenseInfoLength = 24;
	sptwb.Spt.DataIn = SCSI_IOCTL_DATA_IN;
	sptwb.Spt.DataTransferLength = 0;
	sptwb.Spt.TimeOutValue = 2;
	sptwb.Spt.DataBufferOffset = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS, DataBuf);
	sptwb.Spt.SenseInfoOffset = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS, SenseBuf);
	if(type == CMD_TYPE_SAT)
	{
		sptwb.Spt.CdbLength = 12;
		sptwb.Spt.Cdb[0]  = 0xA1; //ATA PASS THROUGH (12) OPERATION CODE (A1h)
		sptwb.Spt.Cdb[1]  = (3 << 1) | 0; //MULTIPLE_COUNT=0,PROTOCOL=3(Non-Data),Reserved
		sptwb.Spt.Cdb[2]  = (1 << 3) | (1 << 2) | 2;//OFF_LINE=0,CK_COND=0,Reserved=0,T_DIR=1(ToDevice),BYTE_BLOCK=1,T_LENGTH=2
		sptwb.Spt.Cdb[3]  = sub;		//FEATURES (7:0)
		sptwb.Spt.Cdb[4]  = param;		//SECTOR_COUNT (7:0)
		sptwb.Spt.Cdb[5]  = 0x00;		//LBA_LOW (7:0)
		sptwb.Spt.Cdb[6]  = 0x00;		//LBA_MID (7:0)
		sptwb.Spt.Cdb[7]  = 0x00;		//LBA_HIGH (7:0)
		sptwb.Spt.Cdb[8]  = target;		//DEVICE_HEAD
		sptwb.Spt.Cdb[9]  = main;		//COMMAND
		sptwb.Spt.Cdb[10] = 0x00;
		sptwb.Spt.Cdb[11] = 0x00;
	}
	else if(type == CMD_TYPE_SUNPLUS)
	{
		sptwb.Spt.CdbLength = 12;
		sptwb.Spt.Cdb[0]  = 0xF8;
		sptwb.Spt.Cdb[1]  = 0x00;
		sptwb.Spt.Cdb[2]  = 0x22;
		sptwb.Spt.Cdb[3]  = 0x10;
		sptwb.Spt.Cdb[4]  = 0x01;
		sptwb.Spt.Cdb[5]  = sub;
		sptwb.Spt.Cdb[6]  = param; 
		sptwb.Spt.Cdb[7]  = 0x00; 
		sptwb.Spt.Cdb[8]  = 0x00;
		sptwb.Spt.Cdb[9]  = 0x00;
		sptwb.Spt.Cdb[10] = target; 
		sptwb.Spt.Cdb[11] = main;
	}
	else if(type == CMD_TYPE_IO_DATA)
	{
		sptwb.Spt.CdbLength = 12;
		sptwb.Spt.Cdb[0]  = 0xE3;
		sptwb.Spt.Cdb[1]  = 0x00;
		sptwb.Spt.Cdb[2]  = sub;
		sptwb.Spt.Cdb[3]  = param;
		sptwb.Spt.Cdb[4]  = 0x00;
		sptwb.Spt.Cdb[5]  = 0x00;
		sptwb.Spt.Cdb[6]  = 0x00;
		sptwb.Spt.Cdb[7]  = target;
		sptwb.Spt.Cdb[8]  = main;
		sptwb.Spt.Cdb[9]  = 0x00;  
		sptwb.Spt.Cdb[10] = 0x00; 
		sptwb.Spt.Cdb[11] = 0x00;
	}
	else if(type == CMD_TYPE_LOGITEC)
	{
		sptwb.Spt.CdbLength = 10;
		sptwb.Spt.Cdb[0] = 0xE0;
		sptwb.Spt.Cdb[1] = 0x00;
		sptwb.Spt.Cdb[2] = sub;
		sptwb.Spt.Cdb[3] = param; 
		sptwb.Spt.Cdb[4] = 0x00;
		sptwb.Spt.Cdb[5] = 0x00;
		sptwb.Spt.Cdb[6] = 0x00;
		sptwb.Spt.Cdb[7] = target; 
		sptwb.Spt.Cdb[8] = main;
		sptwb.Spt.Cdb[9] = 0x4C;		// ?
	}
	else if(type == CMD_TYPE_JMICRON)
	{
		sptwb.Spt.CdbLength = 12;
		sptwb.Spt.Cdb[0]  = 0xDF;
		sptwb.Spt.Cdb[1]  = 0x10;
		sptwb.Spt.Cdb[2]  = 0x00;
		sptwb.Spt.Cdb[3]  = 0x02;
		sptwb.Spt.Cdb[4]  = 0x00;
		sptwb.Spt.Cdb[5]  = sub;
		sptwb.Spt.Cdb[6]  = param; 
		sptwb.Spt.Cdb[7]  = 0x00; 
		sptwb.Spt.Cdb[8]  = 0x00;
		sptwb.Spt.Cdb[9]  = 0x00;
		sptwb.Spt.Cdb[10] = target;
		sptwb.Spt.Cdb[11] = main;
	}
	else if(type == CMD_TYPE_CYPRESS)
	{
		sptwb.Spt.CdbLength = 16;
		sptwb.Spt.Cdb[0]  = 0x24;
		sptwb.Spt.Cdb[1]  = 0x24;
		sptwb.Spt.Cdb[2]  = 0x00;
		sptwb.Spt.Cdb[3]  = 0xBE;
		sptwb.Spt.Cdb[4]  = 0x00;
		sptwb.Spt.Cdb[5]  = 0x00; 
		sptwb.Spt.Cdb[6]  = sub; 
		sptwb.Spt.Cdb[7]  = param; 
		sptwb.Spt.Cdb[8]  = 0x00;
		sptwb.Spt.Cdb[9]  = 0x00;
		sptwb.Spt.Cdb[10] = 0x00;
		sptwb.Spt.Cdb[11] = target;
		sptwb.Spt.Cdb[12] = main;
		sptwb.Spt.Cdb[13] = 0x00;
		sptwb.Spt.Cdb[14] = 0x00;
		sptwb.Spt.Cdb[15] = 0x00;
	}
	else
	{
		return FALSE;
	}

	DWORD length = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS, DataBuf) + sptwb.Spt.DataTransferLength;
	bRet = ::DeviceIoControl(hIoCtrl, IOCTL_SCSI_PASS_THROUGH, 
		&sptwb, sizeof(SCSI_PASS_THROUGH),
		&sptwb, length,	&dwReturned, NULL);

	::CloseHandle(hIoCtrl);

	return	bRet;
}

/*---------------------------------------------------------------------------*/
// Silicon Image Support
/*---------------------------------------------------------------------------*/

HANDLE CAtaSmart::GetIoCtrlHandle(INT scsiPort, DWORD siliconImageType)
{
	HANDLE hScsiDriveIOCTL = 0;
	CString driveName;

	driveName.Format(_T("\\\\.\\Scsi%d:"), scsiPort);
	hScsiDriveIOCTL = CreateFile(driveName, GENERIC_READ | GENERIC_WRITE,
							FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if(hScsiDriveIOCTL == INVALID_HANDLE_VALUE)
	{
		TCHAR szBusPathBody[MAX_PATH];
		TCHAR szSilDeviceName[MAX_PATH];

		if(siliconImageType == 3512)
		{
			siliconImageType = 3112;
		}

		wsprintf(szBusPathBody, _T("tempBusSil%d"), siliconImageType);
		wsprintf(szSilDeviceName, _T("\\Device\\Scsi\\SI%d1"), siliconImageType) ;

		if(DefineDosDevice(DDD_RAW_TARGET_PATH, szBusPathBody, szSilDeviceName))
		{
			driveName.Format(_T("\\\\.\\%s"), szBusPathBody);
			return CreateFile(driveName, GENERIC_READ | GENERIC_WRITE,
						FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
		}
	}
	return hScsiDriveIOCTL;
}

BOOL CAtaSmart::DoIdentifyDeviceSi(INT physicalDriveId, INT scsiPort, INT scsiBus, DWORD siliconImageType, IDENTIFY_DEVICE* data)
{
	int done = FALSE;
	int controller = 0;
	int current = 0;
	HANDLE hScsiDriveIOCTL = 0;

	hScsiDriveIOCTL = GetIoCtrlHandle(scsiPort, siliconImageType);

	if(hScsiDriveIOCTL != INVALID_HANDLE_VALUE)
	{
		SilIdentDev sid;
		memset(&sid, 0, sizeof(sid));
		
		sid.sic.HeaderLength = sizeof(SRB_IO_CONTROL);
		memcpy(sid.sic.Signature, "CMD_IDE ", 8);
		sid.sic.Timeout = 5;
		sid.sic.ControlCode = CTL_CODE(FILE_DEVICE_CONTROLLER, 0x802, METHOD_BUFFERED, FILE_ANY_ACCESS);
		sid.sic.ReturnCode = 0xffffffff;
		sid.sic.Length = sizeof(sid) - offsetof(SilIdentDev, port);
		sid.port = scsiBus;
		sid.maybe_always1 = 1 ;

		DWORD dwReturnBytes;
		if(DeviceIoControl(hScsiDriveIOCTL, IOCTL_SCSI_MINIPORT, &sid, sizeof(sid), &sid, sizeof(sid), &dwReturnBytes, NULL))
		{
			done = TRUE;
			memcpy_s(data, sizeof(IDENTIFY_DEVICE), &sid.id_data, sizeof(IDENTIFY_DEVICE));
		}

		CloseHandle(hScsiDriveIOCTL);
	}
	return done;
}

BOOL CAtaSmart::GetSmartAttributeSi(INT physicalDriveId, ATA_SMART_INFO* asi)
{
//	return GetSmartAttributePd(physicalDriveId, 0xA0, asi);

	HANDLE hScsiDriveIOCTL = 0;

	hScsiDriveIOCTL = GetIoCtrlHandle(physicalDriveId);

	STORAGE_PREDICT_FAILURE spf;
	DWORD dwRetBytes;
	if (DeviceIoControl(hScsiDriveIOCTL, IOCTL_STORAGE_PREDICT_FAILURE,
		&spf, sizeof(spf), &spf, sizeof(spf), &dwRetBytes, NULL))
	{
		if(spf.PredictFailure == 0)
		{
			memcpy_s(&(asi->SmartReadData), 512, &(spf.VendorSpecific), 512);
			return FillSmartInfo(asi);
		}
	}
	return FALSE;
}

BOOL CAtaSmart::GetSmartThresholdSi(INT physicalDriveId, ATA_SMART_INFO* asi)
{
	return FALSE;
}

BOOL CAtaSmart::FillSmartInfo(ATA_SMART_INFO* asi)
{
	CString str;
	asi->AttributeCount = 0;
	int j = 0;
	for(int i = 0; i < MAX_ATTRIBUTE; i++)
	{
		DWORD rawValue = 0;
		memcpy(	&(asi->Attribute[j]), 
			&(asi->SmartReadData[i * sizeof(SMART_ATTRIBUTE) + 2]), sizeof(SMART_ATTRIBUTE));

		if(asi->Attribute[j].Id != 0)
		{
			switch(asi->Attribute[j].Id)
			{
			case 0x09: // Power on Hours
				rawValue = MAKELONG(
					MAKEWORD(asi->Attribute[j].RawValue[0], asi->Attribute[j].RawValue[1]),
					MAKEWORD(asi->Attribute[j].RawValue[2], asi->Attribute[j].RawValue[3])
					);
				if(asi->DiskVendorId == SSD_VENDOR_INDILINX)
				{
					rawValue = asi->Attribute[j].WorstValue * 256 + asi->Attribute[j].CurrentValue;
				}
				asi->PowerOnRawValue = rawValue;
				asi->DetectedPowerOnHours = GetPowerOnHours(rawValue, asi->DetectedTimeUnitType);
				asi->MeasuredPowerOnHours = GetPowerOnHours(rawValue, asi->MeasuredTimeUnitType);
				break;
			case 0x0C: // Power On Count
				rawValue = MAKELONG(
					MAKEWORD(asi->Attribute[j].RawValue[0], asi->Attribute[j].RawValue[1]),
					MAKEWORD(asi->Attribute[j].RawValue[2], asi->Attribute[j].RawValue[3])
					);
				if(asi->DiskVendorId == SSD_VENDOR_INDILINX)
				{
					rawValue = asi->Attribute[j].WorstValue * 256 + asi->Attribute[j].CurrentValue;
				}
				asi->PowerOnCount = rawValue;
				break;
			case 0xBE:
				if(asi->Attribute[j].RawValue[0] > 0 && asi->Attribute[j].RawValue[0] < 100)
				{
					asi->Temperature = asi->Attribute[j].RawValue[0];
				}
				break;
			case 0xC2: // Temperature
				if(asi->Model.Find(_T("SAMSUNG SV")) == 0 && (asi->Attribute[j].RawValue[1] != 0 || asi->Attribute[j].RawValue[0] > 70))
				{
					asi->Temperature = MAKEWORD(asi->Attribute[j].RawValue[0], asi->Attribute[j].RawValue[1]) / 10;			
				}
				else if(asi->DiskVendorId == SSD_VENDOR_SANDFORCE)
				{
					if(asi->Attribute[j].RawValue[2] == 0)
					{
						asi->Temperature = 0;
					}
					else
					{
						asi->Temperature = asi->Attribute[j].RawValue[0];
					}
				}
				else if(asi->Attribute[j].RawValue[0] > 0)
				{
					asi->Temperature = asi->Attribute[j].RawValue[0];
				}

				if(asi->Temperature >= 100)
				{
					asi->Temperature = 0;
				}
				break;
			case 0xBB:
				if(asi->DiskVendorId == SSD_VENDOR_MTRON)
				{
					if(asi->Attribute[j].CurrentValue <= 100)
					{
						asi->Life = asi->Attribute[j].CurrentValue;
					}
				}
				break;
			case 0xD1:
				if(asi->DiskVendorId == SSD_VENDOR_INDILINX)
				{
					if(asi->Attribute[j].CurrentValue <= 100)
					{
						asi->Life = asi->Attribute[j].CurrentValue;
					}
				}
				break;
			case 0xE8:
				if(asi->DiskVendorId == SSD_VENDOR_INTEL)
				{
					if(asi->Attribute[j].CurrentValue <= 100)
					{
						asi->Life = asi->Attribute[j].CurrentValue;
					}
				}
				break;
			case 0xE1:
				if(asi->DiskVendorId == SSD_VENDOR_INTEL)
				{
					asi->HostWrites  = MAKELONG(
						MAKEWORD(asi->Attribute[j].RawValue[0], asi->Attribute[j].RawValue[1]),
						MAKEWORD(asi->Attribute[j].RawValue[2], asi->Attribute[j].RawValue[3])
						);
				}
				break;
			case 0xF2:
				if(asi->DiskVendorId == SSD_VENDOR_INTEL)
				{
					asi->HostReads  = MAKELONG(
						MAKEWORD(asi->Attribute[j].RawValue[0], asi->Attribute[j].RawValue[1]),
						MAKEWORD(asi->Attribute[j].RawValue[2], asi->Attribute[j].RawValue[3])
						);
				}
				break;
			case 0x64:
				if(asi->DiskVendorId == SSD_VENDOR_SANDFORCE)
				{
					asi->GBytesErased  = MAKELONG(
						MAKEWORD(asi->Attribute[j].RawValue[0], asi->Attribute[j].RawValue[1]),
						MAKEWORD(asi->Attribute[j].RawValue[2], asi->Attribute[j].RawValue[3])
						);
				}
				break;
			case 0xB4:
				if(asi->DiskVendorId == SSD_VENDOR_SAMSUNG)
				{
					if(asi->Attribute[j].CurrentValue <= 100)
					{
						asi->Life = asi->Attribute[j].CurrentValue;
					}
				}
				break;
			case 0xE7:
				if(asi->DiskVendorId == SSD_VENDOR_SANDFORCE)
				{
					if(asi->Attribute[j].CurrentValue <= 100)
					{
						asi->Life = asi->Attribute[j].CurrentValue;
					}
				}
				break;
			case 0xAA:
				if(asi->DiskVendorId == SSD_VENDOR_JMICRON && ! asi->IsRawValues8)
				{
					if(asi->Attribute[j].CurrentValue <= 100)
					{
						asi->Life = asi->Attribute[j].CurrentValue;
					}
				}
				break;
			case 0xCA:
				if(asi->DiskVendorId == SSD_VENDOR_MICRON)
				{
					if(asi->Attribute[j].CurrentValue <= 100)
					{
						asi->Life = asi->Attribute[j].CurrentValue;
					}
				}
				break;
			default:
				break;
			}
			j++;
		}
	}
	asi->AttributeCount = j;

	if(asi->AttributeCount > 0)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


/*---------------------------------------------------------------------------*/
// Support Functions
/*---------------------------------------------------------------------------*/

DWORD CAtaSmart::CheckDiskStatus(DWORD i)
{
	if(vars.GetCount() == 0 || ! vars[i].IsSmartCorrect)
	{
		return DISK_STATUS_UNKNOWN;
	}

	// HDD
	if(! vars[i].IsSsd && ! vars[i].IsThresholdCorrect)
	{
		return DISK_STATUS_UNKNOWN;
	}

	if(vars[i].IsThresholdBug)
	{
		return DISK_STATUS_UNKNOWN;
	}

	// DEBUG //
	// vars[i].Attribute[3].RawValue[0] = rand() % 256;

	int error = 0;
	int caution = 0;
	BOOL flagUnknown = TRUE;

	for(DWORD j = 0; j < vars[i].AttributeCount; j++)
	{
		// Check overlap
		for(DWORD k = 0; k < j; k++)
		{
			if(vars[i].Attribute[j].Id == vars[i].Attribute[k].Id)
			{
				return DISK_STATUS_UNKNOWN;
			}
		}

		// Read Error Rate Bug
		if(vars[i].DiskVendorId == SSD_VENDOR_SANDFORCE && vars[i].Attribute[j].Id == 0x01
			&& vars[i].Attribute[j].CurrentValue == 0 && vars[i].Attribute[j].RawValue[0] == 0 && vars[i].Attribute[j].RawValue[1] == 0)
		{
		}
		else if(vars[i].IsSsd && vars[i].IsRawValues8)
		{
		}
		else if(vars[i].IsSsd && ! vars[i].IsRawValues8
		&&	vars[i].Threshold[j].ThresholdValue != 0
		&& 	vars[i].Attribute[j].CurrentValue < vars[i].Threshold[j].ThresholdValue)
		{
			error++;
		}
		else if(((0x01 <= vars[i].Attribute[j].Id && vars[i].Attribute[j].Id <= 0x0D)
		||	(0xBF <= vars[i].Attribute[j].Id && vars[i].Attribute[j].Id <= 0xD1)
		||	(0xDC <= vars[i].Attribute[j].Id && vars[i].Attribute[j].Id <= 0xE4)
		||	(0xE6 <= vars[i].Attribute[j].Id && vars[i].Attribute[j].Id <= 0xE7)
		||	vars[i].Attribute[j].Id == 0xF0
		||	vars[i].Attribute[j].Id == 0xFA
		)
		&&	vars[i].Threshold[j].ThresholdValue != 0
		&& 	vars[i].Attribute[j].CurrentValue < vars[i].Threshold[j].ThresholdValue)
		{
			error++;
		}

		if(vars[i].IsSsd && vars[i].Threshold[j].ThresholdValue != 0)
		{
			flagUnknown = FALSE;
		}

		if( vars[i].Attribute[j].Id == 0x05 // Reallocated Sectors Count
		||	vars[i].Attribute[j].Id == 0xC5 // Current Pending Sector Count
		||	vars[i].Attribute[j].Id == 0xC6 // Off-Line Scan Uncorrectable Sector Count
		)
		{
			if(vars[i].Attribute[j].RawValue[0] == 0xFF
			&& vars[i].Attribute[j].RawValue[1] == 0xFF
			&& vars[i].Attribute[j].RawValue[2] == 0xFF
			&& vars[i].Attribute[j].RawValue[3] == 0xFF)
			{
			}
			else
			{
				WORD raw = MAKEWORD(vars[i].Attribute[j].RawValue[0], vars[i].Attribute[j].RawValue[1]);
				WORD threshold; 
				switch(vars[i].Attribute[j].Id)
				{
				case 0x05:
					threshold = vars[i].Threshold05;
					break;
				case 0xC5:
					threshold = vars[i].ThresholdC5;
					break;
				case 0xC6:
					threshold = vars[i].ThresholdC6;
					break;
				}
				if(threshold > 0 && raw >= threshold && ! vars[i].IsSsd)
				{
					caution = 1;
				}
			}
			if(! vars[i].IsSsd)
			{
				flagUnknown = FALSE;
			}
		}
		else 
		if((vars[i].Attribute[j].Id == 0xE8 && vars[i].DiskVendorId == SSD_VENDOR_INTEL)
		|| (vars[i].Attribute[j].Id == 0xBB && vars[i].DiskVendorId == SSD_VENDOR_MTRON)
		|| (vars[i].Attribute[j].Id == 0xB4 && vars[i].DiskVendorId == SSD_VENDOR_SAMSUNG)
		|| (vars[i].Attribute[j].Id == 0xD1 && vars[i].DiskVendorId == SSD_VENDOR_INDILINX)
		|| (vars[i].Attribute[j].Id == 0xE7 && vars[i].DiskVendorId == SSD_VENDOR_SANDFORCE)
		|| (vars[i].Attribute[j].Id == 0xAA && vars[i].DiskVendorId == SSD_VENDOR_JMICRON && ! vars[i].IsRawValues8)
		|| (vars[i].Attribute[j].Id == 0xCA && vars[i].DiskVendorId == SSD_VENDOR_MICRON)
		)
		{
			if(vars[i].Attribute[j].CurrentValue == 0
			|| vars[i].Attribute[j].CurrentValue < vars[i].Threshold[j].ThresholdValue
			)
			{
				error = 1;
			}
			else if(vars[i].Attribute[j].CurrentValue <= 10)
			{
				caution = 1;
			}
			else
			{
				flagUnknown = FALSE;
			}
		}	
	}

	if(error > 0)
	{
		return DISK_STATUS_BAD;
	}
	else if(flagUnknown)
	{
		return DISK_STATUS_UNKNOWN;
	}
	else if(caution > 0)
	{
		return DISK_STATUS_CAUTION;
	}
	else
	{
		return DISK_STATUS_GOOD;
	}
}

VOID CAtaSmart::ChangeByteOrder(PCHAR str, DWORD length)
{
	CHAR	temp;
	for(DWORD i = 0; i < length; i += 2)
	{
		temp = str[i];
		str[i] = str[i+1];
		str[i+1] = temp;
	}
}

BOOL CAtaSmart::CheckAsciiStringError(PCHAR str, DWORD length)
{
	BOOL flag = FALSE;
	for(DWORD i = 0; i < length; i++)
	{
		if((0x00 < str[i] && str[i] < 0x20))
		{
			str[i] = 0x20;
			break;
		}
		else if(str[i] >= 0x7f)
		{ 
			flag = TRUE;
			break;
		}
	}
	return flag;
}

DWORD CAtaSmart::GetPowerOnHours(DWORD rawValue, DWORD timeUnitType)
{
	switch(timeUnitType)
	{
	case POWER_ON_UNKNOWN:
		return 0;
		break;
	case POWER_ON_HOURS:
		return rawValue;
		break;
	case POWER_ON_MINUTES:
		return rawValue / 60;
		break;
	case POWER_ON_HALF_MINUTES:
		return rawValue / 120;
		break;
	case POWER_ON_SECONDS:
		return rawValue / 60 / 60;
		break;
	default:
		return rawValue;
		break;
	}
}

DWORD CAtaSmart::GetPowerOnHoursEx(DWORD i, DWORD timeUnitType)
{
	DWORD rawValue = vars[i].PowerOnRawValue;
	switch(timeUnitType)
	{
	case POWER_ON_UNKNOWN:
		return 0;
		break;
	case POWER_ON_HOURS:
		return rawValue;
		break;
	case POWER_ON_MINUTES:
		return rawValue / 60;
		break;
	case POWER_ON_HALF_MINUTES:
		return rawValue / 120;
		break;
	case POWER_ON_SECONDS:
		return rawValue / 60 / 60;
		break;
	default:
		return rawValue;
		break;
	}
}

DWORD CAtaSmart::GetTransferMode(WORD w63, WORD w76, WORD w88, CString &current, CString &max, CString &type, INTERFACE_TYPE* interfaceType)
{
	DWORD tm = TRANSFER_MODE_PIO;
	current = max = _T("");
	type = _T("Parallel ATA");
	*interfaceType = INTERFACE_TYPE_PATA;

	// Multiword DMA or PIO
	if(w63 & 0x0700)
	{
		tm = TRANSFER_MODE_PIO_DMA;
		current = max = _T("PIO / DMA");
	}

	// Ultara DMA Max Transfer Mode
		 if(w88 & 0x0040){tm = TRANSFER_MODE_ULTRA_DMA_133; max = _T("Ultra DMA/133");}
	else if(w88 & 0x0020){tm = TRANSFER_MODE_ULTRA_DMA_100; max = _T("Ultra DMA/100");}
	else if(w88 & 0x0010){tm = TRANSFER_MODE_ULTRA_DMA_66;  max = _T("Ultra DMA/66");}
	else if(w88 & 0x0008){tm = TRANSFER_MODE_ULTRA_DMA_44;  max = _T("Ultra DMA/44");}
	else if(w88 & 0x0004){tm = TRANSFER_MODE_ULTRA_DMA_33;  max = _T("Ultra DMA/33");}
	else if(w88 & 0x0002){tm = TRANSFER_MODE_ULTRA_DMA_25;  max = _T("Ultra DMA/25");}
	else if(w88 & 0x0001){tm = TRANSFER_MODE_ULTRA_DMA_16;  max = _T("Ultra DMA/16");}

	// Ultara DMA Current Transfer Mode
		 if(w88 & 0x4000){current = _T("Ultra DMA/133");}
	else if(w88 & 0x2000){current = _T("Ultra DMA/100");}
	else if(w88 & 0x1000){current = _T("Ultra DMA/66");}
	else if(w88 & 0x0800){current = _T("Ultra DMA/44");}
	else if(w88 & 0x0400){current = _T("Ultra DMA/33");}
	else if(w88 & 0x0200){current = _T("Ultra DMA/25");}
	else if(w88 & 0x0100){current = _T("Ultra DMA/16");}

	// Serial ATA
	if(w76 != 0x0000 && w76 != 0xFFFF)
	{
		current = max = _T("SATA/150");
		type = _T("Serial ATA");
		*interfaceType = INTERFACE_TYPE_SATA;
	}

		 if(w76 & 0x0010){tm = TRANSFER_MODE_UNKNOWN;  current = max = _T("Unknown");}
	else if(w76 & 0x0008){tm = TRANSFER_MODE_SATA_600; current = max = _T("SATA/600");}
	else if(w76 & 0x0004){tm = TRANSFER_MODE_SATA_300; current = max = _T("SATA/300");}
	else if(w76 & 0x0002){tm = TRANSFER_MODE_SATA_150; current = max = _T("SATA/150");}

	return tm;
}

DWORD CAtaSmart::GetTimeUnitType(CString model, CString firmware, DWORD major, DWORD transferMode)
{
	model.MakeUpper();

	if(model.Find(_T("FUJITSU")) == 0)
	{
		if(major >= 8)
		{
			return POWER_ON_HOURS;
		}
		else
		{
			return POWER_ON_SECONDS;
		}
	}
	else if(model.Find(_T("HITACHI_DK")) == 0)
	{
		return POWER_ON_MINUTES;
	}
	else if(model.Find(_T("MAXTOR")) == 0)
	{
		if(transferMode >= TRANSFER_MODE_SATA_300
		|| model.Find(_T("MAXTOR 6H")) == 0		// Maxtor DiamondMax 11 family
		|| model.Find(_T("MAXTOR 7H500")) == 0	// Maxtor MaXLine Pro 500 family
		|| model.Find(_T("MAXTOR 6L0")) == 0	// Maxtor DiamondMax Plus D740X family
		|| model.Find(_T("MAXTOR 4K")) == 0		// Maxtor DiamondMax D540X-4K family
		)
		{
			return POWER_ON_HOURS;
		}
		else
		{
			return POWER_ON_MINUTES;
		}
	}
	else if(model.Find(_T("SAMSUNG")) == 0)
	{
		if(transferMode >= TRANSFER_MODE_SATA_300)
		{
			return POWER_ON_HOURS;
		}
		else if(-23 >= _tstoi(firmware.Right(3)) && _tstoi(firmware.Right(3)) >= -39)
		{
			return POWER_ON_HALF_MINUTES;
		}
		else if(model.Find(_T("SAMSUNG SV")) == 0
		||		model.Find(_T("SAMSUNG SP")) == 0
		||		model.Find(_T("SAMSUNG HM")) == 0
		||		model.Find(_T("SAMSUNG MP")) == 0
		)
		{
			return POWER_ON_HALF_MINUTES;
		}
		else
		{
			return POWER_ON_HOURS;
		}
	}
	else
	{
		return POWER_ON_HOURS;
	}
}

DWORD CAtaSmart::GetAtaMajorVersion(WORD w80, CString &majorVersion)
{
	DWORD major = 0;

	if(w80 == 0x0000 || w80 == 0xFFFF)
	{
		return 0;
	}

	for(int i = 14; i > 0; i--)
	{
		if((w80 >> i) & 0x1)
		{
			major = i;
			break;
		}
	}

	if(major > 10)
	{
		majorVersion = _T("");
	}
	else if(major == 10)
	{
		majorVersion = _T("ACS3");
	}
	else if(major == 9)
	{
		majorVersion = _T("ACS2");
	}
	else if(major == 8)
	{
		majorVersion = _T("ATA8-ACS");
	}
	else if(major >= 4)
	{
		majorVersion.Format(_T("ATA/ATAPI-%d"), major);
	}
	else if(major == 0)
	{
		majorVersion = _T("----");
	}
	else
	{
		majorVersion.Format(_T("ATA-%d"), major);
	}

	return major;
}

// Last Update : 2011/03/21
// Reference : http://www.t13.org/Documents/MinutesDefault.aspx?DocumentType=4&DocumentStage=2
//           - d2161r0-ATAATAPI_Command_Set_-_3.pdf
//           - d1153r18-ATA-ATAPI-4.pdf 
VOID CAtaSmart::GetAtaMinorVersion(WORD w81, CString &minor)
{
	switch(w81)
	{
	case 0x0000:
	case 0xFFFF:
				//	minor = _T("Not Reported");									break;
					minor = _T("----");											break;
	case 0x0001:	minor = _T("ATA (ATA-1) X3T9.2 781D prior to revision 4");	break;
	case 0x0002:	minor = _T("ATA-1 published, ANSI X3.221-1994");			break;
	case 0x0003:	minor = _T("ATA (ATA-1) X3T10 781D revision 4");			break;
	case 0x0004:	minor = _T("ATA-2 published, ANSI X3.279-1996");			break;
	case 0x0005:	minor = _T("ATA-2 X3T10 948D prior to revision 2k");		break;
	case 0x0006:	minor = _T("ATA-3 X3T10 2008D revision 1");					break;
	case 0x0007:	minor = _T("ATA-2 X3T10 948D revision 2k");					break;
	case 0x0008:	minor = _T("ATA-3 X3T10 2008D revision 0");					break;
	case 0x0009:	minor = _T("ATA-2 X3T10 948D revision 3");					break;
	case 0x000A:	minor = _T("ATA-3 published, ANSI X3.298-199x");			break;
	case 0x000B:	minor = _T("ATA-3 X3T10 2008D revision 6");					break;
	case 0x000C:	minor = _T("ATA-3 X3T13 2008D revision 7 and 7a");			break;
	case 0x000D:	minor = _T("ATA/ATAPI-4 X3T13 1153D version 6");			break;
	case 0x000E:	minor = _T("ATA/ATAPI-4 T13 1153D version 13");				break;
	case 0x000F:	minor = _T("ATA/ATAPI-4 X3T13 1153D version 7");			break;
	case 0x0010:	minor = _T("ATA/ATAPI-4 T13 1153D version 18");				break;
	case 0x0011:	minor = _T("ATA/ATAPI-4 T13 1153D version 15");				break;
	case 0x0012:	minor = _T("ATA/ATAPI-4 published, ANSI INCITS 317-1998");	break;
	case 0x0013:	minor = _T("ATA/ATAPI-5 T13 1321D version 3");				break;
	case 0x0014:	minor = _T("ATA/ATAPI-4 T13 1153D version 14");				break;
	case 0x0015:	minor = _T("ATA/ATAPI-5 T13 1321D version 1");				break;
	case 0x0016:	minor = _T("ATA/ATAPI-5 published, ANSI INCITS 340-2000");	break;
	case 0x0017:	minor = _T("ATA/ATAPI-4 T13 1153D version 17");				break;
	case 0x0018:	minor = _T("ATA/ATAPI-6 T13 1410D version 0");				break;
	case 0x0019:	minor = _T("ATA/ATAPI-6 T13 1410D version 3a");				break;
	case 0x001A:	minor = _T("ATA/ATAPI-7 T13 1532D version 1");				break;
	case 0x001B:	minor = _T("ATA/ATAPI-6 T13 1410D version 2");				break;
	case 0x001C:	minor = _T("ATA/ATAPI-6 T13 1410D version 1");				break;
	case 0x001D:	minor = _T("ATA/ATAPI-7 published ANSI INCITS 397-2005.");	break;
	case 0x001E:	minor = _T("ATA/ATAPI-7 T13 1532D version 0");				break;
	case 0x0021:	minor = _T("ATA/ATAPI-7 T13 1532D version 4a");				break;
	case 0x0022:	minor = _T("ATA/ATAPI-6 published, ANSI INCITS 361-2002");	break;
	case 0x0027:	minor = _T("ATA8-ACS version 3c");							break;
	case 0x0028:	minor = _T("ATA8-ACS version 6");							break;
	case 0x0029:	minor = _T("ATA8-ACS version 4");							break;
	case 0x0031:	minor = _T("ACS-2 Revision 2");								break;
	case 0x0033:	minor = _T("ATA8-ACS version 3e");							break;
	case 0x0039:	minor = _T("ATA8-ACS version 4c");							break;
	case 0x0042:	minor = _T("ATA8-ACS version 3f");							break;
	case 0x0052:	minor = _T("ATA8-ACS version 3b");							break;
	case 0x0107:	minor = _T("ATA8-ACS version 2d");							break;
	case 0x0110:	minor = _T("ACS-2 Revision 3");								break;
	default:	//	minor.Format(_T("Reserved [%04Xh]"), w81);					break;
					minor.Format(_T("---- [%04Xh]"), w81);						break;
	}
}