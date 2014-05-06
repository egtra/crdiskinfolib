/*---------------------------------------------------------------------------*/
//       Author : hiyohiyo
//         Mail : hiyohiyo@crystalmark.info
//          Web : http://crystalmark.info/
//      License : Simplified BSD license
/*---------------------------------------------------------------------------*/

#include "stdafx.h"
#include "AtaSmart.h"
#include "DiskInfoLib.h"

namespace
{

void InitAta(CAtaSmart& ata, BOOL useWmi, BOOL advancedDiskSearch, PBOOL flagChangeDisk, BOOL workaroundHD204UI, BOOL workaroundAdataSsd, BOOL flagHideNoSmartDisk)
{
	ata.Init(useWmi, advancedDiskSearch, flagChangeDisk, workaroundHD204UI, workaroundAdataSsd, flagHideNoSmartDisk);

	DWORD errorCount = 0;
	for (int i = 0; i < ata.vars.GetCount(); i++)
	{
		//ata.vars[i].AlarmTemperature = GetPrivateProfileInt(_T("AlarmTemperature"), m_Ata.vars[i].ModelSerial, 50, m_Ini);
		//ata.vars[i].AlarmHealthStatus = GetPrivateProfileInt(_T("AlarmHealthStatus"), m_Ata.vars[i].ModelSerial, 1, m_Ini);

		ata.vars[i].Threshold05 = 1;
		ata.vars[i].ThresholdC5 = 1;
		ata.vars[i].ThresholdC6 = 1;
		ata.vars[i].ThresholdFF = 0;

		ata.vars[i].DiskStatus = ata.CheckDiskStatus(i);
	}
}

} // namespace

std::vector<DiskInfo> CrDiskInfoLib::GetDiskInfo()
{
	BOOL useWmi = TRUE;
	BOOL advancedDiskSearch = FALSE;
	BOOL flagChangeDisk = FALSE;
	BOOL workaroundHD204UI = FALSE;
	BOOL workaroundAdataSsd = TRUE;
	BOOL flagHideNoSmartDisk = FALSE;

	std::vector<DiskInfo> result;

	CAtaSmart ata;
	InitAta(ata, useWmi, advancedDiskSearch, &flagChangeDisk, workaroundHD204UI, workaroundAdataSsd, flagHideNoSmartDisk);

	result.reserve(ata.vars.GetCount());
	for (int i = 0; i < ata.vars.GetCount(); i++)
	{
		DiskInfo diskInfo = {};

		diskInfo.Attribute.reserve(ata.vars[i].AttributeCount);
		if (ata.vars[i].DiskVendorId == ata.SSD_VENDOR_SANDFORCE)
		{
			diskInfo.IsCurrentCorrect = true;
			diskInfo.IsWorstCorrect = true;
			for (DWORD j = 0; j < ata.vars[i].AttributeCount; j++)
			{
				BYTE id = ata.vars[i].Attribute[j].Id;
				DiskInfo::SmartAttribute attribute = {};
				attribute.Id = id;
				attribute.CurrentValue = ata.vars[i].Attribute[j].CurrentValue;
				attribute.WorstValue = ata.vars[i].Attribute[j].WorstValue;
				attribute.ThresholdValue = ata.vars[i].Threshold[j].ThresholdValue;
				BYTE* rawValueBytes = reinterpret_cast<BYTE*>(&attribute.RawValue);
				rawValueBytes[0] = ata.vars[i].Attribute[j].RawValue[0];
				rawValueBytes[1] = ata.vars[i].Attribute[j].RawValue[1];
				rawValueBytes[2] = ata.vars[i].Attribute[j].RawValue[2];
				rawValueBytes[3] = ata.vars[i].Attribute[j].RawValue[3];
				rawValueBytes[4] = ata.vars[i].Attribute[j].RawValue[4];
				rawValueBytes[5] = ata.vars[i].Attribute[j].RawValue[5];
				rawValueBytes[6] = ata.vars[i].Attribute[j].Reserved;
				diskInfo.Attribute.push_back(attribute);
			}
		}
		else if (ata.vars[i].DiskVendorId == ata.SSD_VENDOR_JMICRON && ata.vars[i].IsRawValues8)
		{
			diskInfo.IsCurrentCorrect = false;
			diskInfo.IsWorstCorrect = false;
			for (DWORD j = 0; j < ata.vars[i].AttributeCount; j++)
			{
				DiskInfo::SmartAttribute attribute = {};
				attribute.Id = ata.vars[i].Attribute[j].Id;
				attribute.CurrentValue = ata.vars[i].Attribute[j].CurrentValue;
				BYTE* rawValueBytes = reinterpret_cast<BYTE*>(&attribute.RawValue);
				rawValueBytes[0] = ata.vars[i].Attribute[j].WorstValue;
				rawValueBytes[1] = ata.vars[i].Attribute[j].RawValue[0];
				rawValueBytes[2] = ata.vars[i].Attribute[j].RawValue[1];
				rawValueBytes[3] = ata.vars[i].Attribute[j].RawValue[2];
				rawValueBytes[4] = ata.vars[i].Attribute[j].RawValue[3];
				rawValueBytes[5] = ata.vars[i].Attribute[j].RawValue[4];
				rawValueBytes[6] = ata.vars[i].Attribute[j].RawValue[5];
				rawValueBytes[7] = ata.vars[i].Attribute[j].Reserved;
				diskInfo.Attribute.push_back(attribute);
			}
		}
		else if (ata.vars[i].DiskVendorId == ata.SSD_VENDOR_INDILINX)
		{
			diskInfo.IsCurrentCorrect = false;
			diskInfo.IsWorstCorrect = false;
			for (DWORD j = 0; j < ata.vars[i].AttributeCount; j++)
			{
				DiskInfo::SmartAttribute attribute = {};
				attribute.Id = ata.vars[i].Attribute[j].Id;
				BYTE* rawValueBytes = reinterpret_cast<BYTE*>(&attribute.RawValue);
				rawValueBytes[0] = ata.vars[i].Attribute[j].CurrentValue;
				rawValueBytes[1] = ata.vars[i].Attribute[j].WorstValue;
				rawValueBytes[2] = ata.vars[i].Attribute[j].RawValue[0];
				rawValueBytes[3] = ata.vars[i].Attribute[j].RawValue[1];
				rawValueBytes[4] = ata.vars[i].Attribute[j].RawValue[2];
				rawValueBytes[5] = ata.vars[i].Attribute[j].RawValue[3];
				rawValueBytes[6] = ata.vars[i].Attribute[j].RawValue[4];
				rawValueBytes[7] = ata.vars[i].Attribute[j].RawValue[5];
				diskInfo.Attribute.push_back(attribute);
			}
		}
		else
		{
			diskInfo.IsCurrentCorrect = true;
			diskInfo.IsWorstCorrect = true;
			for (DWORD j = 0; j < ata.vars[i].AttributeCount; j++)
			{
				DiskInfo::SmartAttribute attribute = {};
				attribute.Id = ata.vars[i].Attribute[j].Id;
				attribute.CurrentValue = ata.vars[i].Attribute[j].CurrentValue;
				attribute.WorstValue = ata.vars[i].Attribute[j].WorstValue;
				attribute.ThresholdValue = ata.vars[i].Threshold[j].ThresholdValue;
				BYTE* rawValueBytes = reinterpret_cast<BYTE*>(&attribute.RawValue);
				rawValueBytes[0] = ata.vars[i].Attribute[j].RawValue[0];
				rawValueBytes[1] = ata.vars[i].Attribute[j].RawValue[1];
				rawValueBytes[2] = ata.vars[i].Attribute[j].RawValue[2];
				rawValueBytes[3] = ata.vars[i].Attribute[j].RawValue[3];
				rawValueBytes[4] = ata.vars[i].Attribute[j].RawValue[4];
				rawValueBytes[5] = ata.vars[i].Attribute[j].RawValue[5];
				diskInfo.Attribute.push_back(attribute);
			}
		}
		std::copy(ata.vars[i].SmartReadData, ata.vars[i].SmartReadData + ARRAYSIZE(ata.vars[i].SmartReadData), diskInfo.SmartReadData);
		std::copy(ata.vars[i].SmartReadThreshold, ata.vars[i].SmartReadThreshold + ARRAYSIZE(ata.vars[i].SmartReadThreshold), diskInfo.SmartReadThreshold);

		diskInfo.IsSmartSupported = !!ata.vars[i].IsSmartSupported;
		diskInfo.IsThresholdCorrect = !!ata.vars[i].IsThresholdCorrect;
		diskInfo.IsRawValues8 = !!ata.vars[i].IsRawValues8;
		diskInfo.IsRawValues7 = !!ata.vars[i].IsRawValues7;

		diskInfo.IsLba48Supported = !!ata.vars[i].IsLba48Supported;
		diskInfo.IsApmSupported = !!ata.vars[i].IsApmSupported;
		diskInfo.IsAamSupported = !!ata.vars[i].IsAamSupported;
		diskInfo.IsNcqSupported = !!ata.vars[i].IsNcqSupported;
		diskInfo.IsDeviceSleepSupported = !!ata.vars[i].IsDeviceSleepSupported;
		diskInfo.IsMaxtorMinute = !!ata.vars[i].IsMaxtorMinute;
		diskInfo.IsSsd = !!ata.vars[i].IsSsd;
		diskInfo.IsTrimSupported = !!ata.vars[i].IsTrimSupported;

		diskInfo.PhysicalDriveId = ata.vars[i].PhysicalDriveId;
		diskInfo.ScsiPort = ata.vars[i].ScsiPort;
		diskInfo.ScsiTargetId = ata.vars[i].ScsiTargetId;
		diskInfo.ScsiBus = ata.vars[i].ScsiBus;
		diskInfo.SiliconImageType = ata.vars[i].SiliconImageType;

		diskInfo.TotalDiskSize = ata.vars[i].TotalDiskSize;
		diskInfo.Cylinder = ata.vars[i].Cylinder;
		diskInfo.Head = ata.vars[i].Head;
		diskInfo.Sector = ata.vars[i].Sector;
		diskInfo.Sector28 = ata.vars[i].Sector28;
		diskInfo.Sector48 = ata.vars[i].Sector48;
		diskInfo.NumberOfSectors = ata.vars[i].NumberOfSectors;
		diskInfo.DiskSizeChs = ata.vars[i].DiskSizeChs;
		diskInfo.DiskSizeLba28 = ata.vars[i].DiskSizeLba28;
		diskInfo.DiskSizeLba48 = ata.vars[i].DiskSizeLba48;
		diskInfo.DiskSizeWmi = ata.vars[i].DiskSizeWmi;
		diskInfo.BufferSize = ata.vars[i].BufferSize;
		diskInfo.NvCacheSize = ata.vars[i].NvCacheSize;
		diskInfo.TransferModeType = static_cast<TRANSFER_MODE>(ata.vars[i].TransferModeType);
		diskInfo.DetectedPowerOnHours = ata.vars[i].DetectedPowerOnHours;
		diskInfo.PowerOnCount = ata.vars[i].PowerOnCount;
		diskInfo.HostWrites = ata.vars[i].HostWrites;
		diskInfo.HostReads = ata.vars[i].HostReads;
		diskInfo.GBytesErased = ata.vars[i].GBytesErased;
		diskInfo.NandWrites = ata.vars[i].NandWrites;
		diskInfo.WearLevelingCount = ata.vars[i].WearLevelingCount;
		diskInfo.Life = ata.vars[i].Life;

		diskInfo.Major = ata.vars[i].Major;
		diskInfo.Minor = ata.vars[i].Minor;

		diskInfo.InterfaceType = ata.vars[i].InterfaceType;
		diskInfo.CommandType = ata.vars[i].CommandType;

		diskInfo.DiskVendorId = ata.vars[i].DiskVendorId;
		diskInfo.UsbVendorId = ata.vars[i].UsbVendorId;
		diskInfo.UsbProductId = ata.vars[i].UsbProductId;
		diskInfo.Target = ata.vars[i].Target;

		diskInfo.SasPhyEntity = ata.vars[i].sasPhyEntity;

		diskInfo.SerialNumber = ata.vars[i].SerialNumber;
		diskInfo.FirmwareRev = ata.vars[i].FirmwareRev;
		diskInfo.Model = ata.vars[i].Model;
		diskInfo.ModelWmi = ata.vars[i].ModelWmi;
		diskInfo.ModelSerial = ata.vars[i].ModelSerial;
		diskInfo.MaxTransferMode = ata.vars[i].MaxTransferMode;
		diskInfo.CurrentTransferMode = ata.vars[i].CurrentTransferMode;
		diskInfo.MajorVersion = ata.vars[i].MajorVersion;
		diskInfo.MinorVersion = ata.vars[i].MinorVersion;
		diskInfo.Interface = ata.vars[i].Interface;
		diskInfo.Enclosure = ata.vars[i].Enclosure;
		diskInfo.SsdVendorString = ata.vars[i].SsdVendorString;
		diskInfo.DeviceNominalFormFactor = ata.vars[i].DeviceNominalFormFactor;
		diskInfo.PnpDeviceId = ata.vars[i].PnpDeviceId;

		diskInfo.Temperature = ata.vars[i].Temperature;
		diskInfo.IdentifyDevice = ata.vars[i].IdentifyDevice;
		diskInfo.NominalMediaRotationRate = ata.vars[i].NominalMediaRotationRate;

		result.push_back(diskInfo);
	}

	return result;
}
