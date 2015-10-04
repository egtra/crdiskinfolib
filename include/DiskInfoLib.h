// CrDiskInfoLib: include/DiskInfoLib.h
// Copyright (c) 2014, Egtra
//
// This code is released under the MIT License.
// See LICENSE.txt

#pragma once

#include "DiskInfoLibCommon.h"

#include <vector>
#include <string>

struct DiskInfo
{
	struct SmartAttribute
	{
		BYTE Id;
		BYTE CurrentValue;
		BYTE WorstValue;
		BYTE ThresholdValue;
		UINT64 RawValue;
	};

	BYTE SmartReadData[512];
	BYTE SmartReadThreshold[512];

	std::vector<SmartAttribute> Attribute;

	bool IsSmartSupported;
	bool IsCurrentCorrect; ///< SmartAttribute::CurentValue が利用可能であること。
	bool IsWorstCorrect; ///< SmartAttribute::WorstValue が利用可能であること。
	bool IsThresholdCorrect; ///< SmartAttribute::ThresholdValue が利用可能であること。
	bool IsRawValues8; ///< Smartの生の値が8バイトで表現される特殊な機器。
	bool IsRawValues7; ///< Smartの生の値が7バイトで表現される特殊な機器。

	bool IsLba48Supported;
	bool IsApmSupported;
	bool IsAamSupported;
	bool IsNcqSupported;
	bool IsDeviceSleepSupported;
	bool IsMaxtorMinute; ///< 使用時間が不正確な可能性があるMaxtor製機器。
	bool IsSsd;
	bool IsTrimSupported;

	int PhysicalDriveId; ///< OSの物理ドライブID。"\\\\.\\PhysicalDrive${N}"で参照できる。非該当または不明な場合は-1。
	int ScsiPort; ///< SCSIポート番号。"\\\\.\\scsi${N}:"で参照できる。非該当または不明な場合は-1。
	int ScsiTargetId; ///< SCSIターゲットID。非該当または不明な場合は-1。
	int ScsiBus; ///< SCSIバス番号。非該当または不明な場合は-1。
	int SiliconImageType; ///< Silicon Image製コントローラの型番の一部。非該当の場合は0。
	DWORD TotalDiskSize;
	DWORD Cylinder;
	DWORD Head;
	DWORD Sector;
	DWORD Sector28;
	ULONGLONG Sector48;
	ULONGLONG NumberOfSectors;
	DWORD DiskSizeChs;
	DWORD DiskSizeLba28;
	DWORD DiskSizeLba48;
	DWORD DiskSizeWmi;
	DWORD BufferSize;
	ULONGLONG NvCacheSize;
	TRANSFER_MODE TransferModeType;
	int DetectedPowerOnHours; ///< 使用時間。不明な場合は-1。
	//int MeasuredPowerOnHours;
	DWORD PowerOnCount;
	int HostWrites; ///< 総書込量 (GiB)。不明な場合は-1。
	int HostReads; ///< 総読込量 (GiB)。不明な場合は-1。
	int GBytesErased; ///< 総消去量 (GiB)。不明な場合は-1。
	int NandWrites; ///< NANDレベルでの総書込量 (GiB)。不明な場合は-1。
	int WearLevelingCount; ///< ウェアレベルカウント。不明な場合は-1。
	int Life; ///< 寿命。0～100。不明な場合は-1。

	DWORD Major;
	DWORD Minor;

	INTERFACE_TYPE InterfaceType;
	COMMAND_TYPE CommandType;

	DWORD DiskVendorId;
	DWORD UsbVendorId;
	DWORD UsbProductId;
	BYTE Target;

	CSMI_SAS_PHY_ENTITY SasPhyEntity;

	std::wstring SerialNumber;
	std::wstring FirmwareRev;
	std::wstring Model;
	std::wstring ModelWmi;
	std::wstring ModelSerial;
	std::wstring MaxTransferMode;
	std::wstring CurrentTransferMode;
	std::wstring MajorVersion;
	std::wstring MinorVersion;
	std::wstring Interface;
	std::wstring Enclosure;
	std::wstring SsdVendorString;
	std::wstring DeviceNominalFormFactor;
	std::wstring PnpDeviceId; ///< デバイスインスタンスパス（Windows XPまではデバイスインスタンスIDと表記されていた）。

	DWORD Temperature; /// 温度 (℃)。不明な場合は0。
	IDENTIFY_DEVICE IdentifyDevice;
	DWORD NominalMediaRotationRate; ///< 回転数。不明な場合は0。ATA8-ACS対応SSDの場合は1。
};

namespace CrDiskInfoLib
{
	std::vector<DiskInfo> GetDiskInfo();

#if CR_DISK_INFO_LIB_CUSTOM_DEBUG_PRINT
	void CrDiskInfoLib_DebugPrint(PCWSTR s); // User-defined
#else
	inline void DebugPrint(PCWSTR s)
	{
		OutputDebugStringW(s);
	}
#endif
}
