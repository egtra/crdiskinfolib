/*---------------------------------------------------------------------------*/
//       Author : hiyohiyo
//         Mail : hiyohiyo@crystalmark.info
//          Web : http://crystalmark.info/
//      License : Simplified BSD license
/*---------------------------------------------------------------------------*/
// Reference : http://www.usefullcode.net/2007/02/hddsmart.html (ja)

#pragma once

#include "winioctl.h"
#include "SPTIUtil.h"
#include "DiskInfoLibCommon.h"

typedef struct _VOLUME_DISK_EXTENTS_LX {
    DWORD       NumberOfDiskExtents;
    DISK_EXTENT Extents[4];
} VOLUME_DISK_EXTENTS_LX, *PVOLUME_DISK_EXTENTS_LX;

class CAtaSmart
{

public:
	static const int MAX_DISK = 48;// FIX
	static const int MAX_ATTRIBUTE = 30; // FIX
	static const int MAX_SEARCH_PHYSICAL_DRIVE = 56;
	static const int MAX_SEARCH_SCSI_PORT = 16;
	static const int MAX_SEARCH_SCSI_TARGET_ID = 8;

	static const int SCSI_MINIPORT_BUFFER_SIZE = 512;

public:
	CAtaSmart();
	virtual ~CAtaSmart();

	enum SMART_STATUS
	{
		SMART_STATUS_NO_CHANGE = 0,
		SMART_STATUS_MINOR_CHANGE,
		SMART_STATUS_MAJOR_CHANGE
	};

	enum DISK_STATUS
	{
		DISK_STATUS_UNKNOWN = 0,
		DISK_STATUS_GOOD,
		DISK_STATUS_CAUTION,
		DISK_STATUS_BAD
	};

	enum HOST_READS_WRITES_UNIT
	{
		HOST_READS_WRITES_UNKNOWN = 0,
		HOST_READS_WRITES_512B,
		HOST_READS_WRITES_32MB,
		HOST_READS_WRITES_GB,
	};

	enum CSMI_TYPE
	{
		CSMI_TYPE_DISABLE = 0,
		CSMI_TYPE_ENABLE_AUTO,
		CSMI_TYPE_ENABLE_RAID,
		CSMI_TYPE_ENABLE_ALL,
	};

	enum SMART_WMI_TYPE
	{
		WMI_SMART_DATA = 0,
		WMI_SMART_THRESHOLD
	};

	enum VENDOR_ID
	{
		HDD_GENERAL           = 0,
		SSD_GENERAL           = 1,
		SSD_VENDOR_MTRON      = 2,
		SSD_VENDOR_INDILINX   = 3,
		SSD_VENDOR_JMICRON    = 4,
		SSD_VENDOR_INTEL      = 5,
		SSD_VENDOR_SAMSUNG    = 6,
		SSD_VENDOR_SANDFORCE  = 7,
		SSD_VENDOR_MICRON     = 8,
		SSD_VENDOR_OCZ        = 9,
		HDD_SSD_VENDOR_SEAGATE= 10,
		HDD_VENDOR_WESTERN_DIGITAL=11,
		SSD_VENDOR_PLEXTOR    = 12, 
		SSD_VENDOR_SANDISK	  = 13,
		SSD_VENDOR_OCZ_VECTOR = 14,
		HDD_SSD_VENDOR_TOSHIBA= 15,
		SSD_VENDOR_MAX        = 99,

		VENDOR_UNKNOWN      = 0x0000,
		USB_VENDOR_BUFFALO  = 0x0411,
		USB_VENDOR_IO_DATA  = 0x04BB,
		USB_VENDOR_LOGITEC  = 0x0789,
		USB_VENDOR_INITIO   = 0x13FD,
		USB_VENDOR_SUNPLUS  = 0x04FC,
		USB_VENDOR_JMICRON  = 0x152D,
		USB_VENDOR_CYPRESS  = 0x04B4,
		USB_VENDOR_OXFORD   = 0x0928,
		USB_VENDOR_PROLIFIC = 0x067B,
		USB_VENDOR_ALL      = 0xFFFF,
	};

protected:
	enum IO_CONTROL_CODE
	{
		DFP_SEND_DRIVE_COMMAND	= 0x0007C084,
		DFP_RECEIVE_DRIVE_DATA	= 0x0007C088,
		IOCTL_SCSI_MINIPORT     = 0x0004D008,
		IOCTL_IDE_PASS_THROUGH  = 0x0004D028, // 2000 or later
		IOCTL_ATA_PASS_THROUGH  = 0x0004D02C, // XP SP2 and 2003 or later
	};

#pragma pack(push,1)

	typedef	struct _IDENTIFY_DEVICE_OUTDATA
	{
		SENDCMDOUTPARAMS	SendCmdOutParam;
		BYTE				Data[IDENTIFY_BUFFER_SIZE - 1];
	} IDENTIFY_DEVICE_OUTDATA, *PIDENTIFY_DEVICE_OUTDATA;

	typedef	struct _SMART_READ_DATA_OUTDATA
	{
		SENDCMDOUTPARAMS	SendCmdOutParam;
		BYTE				Data[READ_ATTRIBUTE_BUFFER_SIZE - 1];
	} SMART_READ_DATA_OUTDATA, *PSMART_READ_DATA_OUTDATA;

	typedef struct _CMD_IDE_PATH_THROUGH
	{
		IDEREGS	reg;
		DWORD   length;
		BYTE    buffer[1];
	} CMD_IDE_PATH_THROUGH, *PCMD_IDE_PATH_THROUGH;

	static const int ATA_FLAGS_DRDY_REQUIRED = 0x01;
	static const int ATA_FLAGS_DATA_IN       = 0x02;
	static const int ATA_FLAGS_DATA_OUT      = 0x04;
	static const int ATA_FLAGS_48BIT_COMMAND = 0x08;

	typedef struct _ATA_PASS_THROUGH_EX
	{
		WORD    Length;
		WORD    AtaFlags;
		BYTE    PathId;
		BYTE    TargetId;
		BYTE    Lun;
		BYTE    ReservedAsUchar;
		DWORD   DataTransferLength;
		DWORD   TimeOutValue;
		DWORD   ReservedAsUlong;
	//	DWORD   DataBufferOffset;
#ifdef _WIN64
		DWORD	padding;
#endif
		DWORD_PTR   DataBufferOffset;
		IDEREGS PreviousTaskFile;
		IDEREGS CurrentTaskFile;
	} ATA_PASS_THROUGH_EX, *PCMD_ATA_PASS_THROUGH_EX;

	typedef struct
	{
		ATA_PASS_THROUGH_EX Apt;
		DWORD Filer;
		BYTE  Buf[512];
	} ATA_PASS_THROUGH_EX_WITH_BUFFERS;

	typedef	struct SMART_ATTRIBUTE
	{
		BYTE	Id;
		WORD	StatusFlags;
		BYTE	CurrentValue;
		BYTE	WorstValue;
		BYTE	RawValue[6];
		BYTE	Reserved;
	};

	typedef	struct SMART_THRESHOLD
	{
		BYTE	Id;
		BYTE	ThresholdValue;
		BYTE	Reserved[10];
	};

	typedef struct SRB_IO_COMMAND
	{
		SRB_IO_CONTROL	Cntrol;
		IDEREGS			IdeRegs;
		BYTE			Data[512];
	};

	typedef struct {
		SRB_IO_CONTROL sic ;
		USHORT port ;
		USHORT maybe_always1 ;
		ULONG unknown[5] ;
		//IDENTIFY_DEVICE id_data ;
		WORD id_data[256] ;
	} SilIdentDev ;

#pragma	pack(pop)

public:
	DWORD UpdateSmartInfo(DWORD index);
	BOOL UpdateIdInfo(DWORD index);
	BYTE GetAamValue(DWORD index);
	BYTE GetApmValue(DWORD index);
	BOOL EnableAam(DWORD index, BYTE param);
	BOOL EnableApm(DWORD index, BYTE param);
	BOOL DisableAam(DWORD index);
	BOOL DisableApm(DWORD index);
	BYTE GetRecommendAamValue(DWORD index);
	BYTE GetRecommendApmValue(DWORD index);

	VOID Init(BOOL useWmi, BOOL advancedDiskSearch, PBOOL flagChangeDisk, BOOL workaroundHD204UI, BOOL workaroundAdataSsd, BOOL flagHideNoSmartDisk);
	BOOL MeasuredTimeUnit();
	DWORD GetPowerOnHours(DWORD rawValue, DWORD timeUnitType);
	DWORD GetPowerOnHoursEx(DWORD index, DWORD timeUnitType);

	VOID SetAtaPassThroughSmart(BOOL flag);

	struct DISK_POSITION
	{
		INT					PhysicalDriveId;
		INT					ScsiPort;
		INT					ScsiTargetId;
		CSMI_SAS_PHY_ENTITY sasPhyEntity;
	};

	struct ATA_SMART_INFO
	{
		IDENTIFY_DEVICE		IdentifyDevice;
		BYTE				SmartReadData[512];
		BYTE				SmartReadThreshold[512];
		SMART_ATTRIBUTE		Attribute[MAX_ATTRIBUTE];
		SMART_THRESHOLD		Threshold[MAX_ATTRIBUTE];

		BOOL				IsSmartEnabled;
		BOOL				IsIdInfoIncorrect;
		BOOL				IsSmartCorrect;
		BOOL				IsThresholdCorrect;
		BOOL				IsCheckSumError;
		BOOL				IsWord88;
		BOOL				IsWord64_76;
		BOOL				IsRawValues8;
		BOOL				IsRawValues7;
		BOOL				Is9126MB;
		BOOL				IsThresholdBug;

		BOOL				IsSmartSupported;
		BOOL				IsLba48Supported;
		BOOL				IsAamSupported;
		BOOL				IsApmSupported;
		BOOL				IsAamEnabled;
		BOOL				IsApmEnabled;
		BOOL				IsNcqSupported;
		BOOL				IsNvCacheSupported;
		BOOL				IsDeviceSleepSupported;
		BOOL				IsMaxtorMinute;
		BOOL				IsSsd;
		BOOL				IsTrimSupported;

		INT					PhysicalDriveId;
		INT					ScsiPort;
		INT					ScsiTargetId;
		INT					ScsiBus;
		INT					SiliconImageType;
//		INT					AccessType;

		DWORD				TotalDiskSize;
		DWORD				Cylinder;
		DWORD				Head;
		DWORD				Sector;
		DWORD				Sector28;
		ULONGLONG			Sector48;
		ULONGLONG			NumberOfSectors;
		DWORD				DiskSizeChs;
		DWORD				DiskSizeLba28;
		DWORD				DiskSizeLba48;
		DWORD				DiskSizeWmi;
		DWORD				BufferSize;
		ULONGLONG			NvCacheSize;
		DWORD				TransferModeType;
		DWORD				DetectedTimeUnitType;
		DWORD				MeasuredTimeUnitType;
		DWORD				AttributeCount;
		INT					DetectedPowerOnHours;
		INT					MeasuredPowerOnHours;
		INT					PowerOnRawValue;
		INT					PowerOnStartRawValue;
		DWORD				PowerOnCount;
		DWORD				Temperature;
		double				TemperatureMultiplier;
		DWORD				NominalMediaRotationRate;
//		double				Speed;
		INT					HostWrites;
		INT					HostReads;
		INT					GBytesErased;
		INT					NandWrites;
		INT					WearLevelingCount;

		INT					PlextorNandWritesUnit;

		INT					Life;

		DWORD				Major;
		DWORD				Minor;

		DWORD				DiskStatus;
		DWORD				DriveLetterMap;
		// 
		DWORD				AlarmTemperature;
		BOOL				AlarmHealthStatus;

		INTERFACE_TYPE		InterfaceType;
		COMMAND_TYPE		CommandType;
		HOST_READS_WRITES_UNIT HostReadsWritesUnit;

		DWORD				DiskVendorId;
		DWORD				UsbVendorId;
		DWORD				UsbProductId;
		BYTE				Target;

		WORD				Threshold05;
		WORD				ThresholdC5;
		WORD				ThresholdC6;
		WORD				ThresholdFF;

		CSMI_SAS_PHY_ENTITY sasPhyEntity;

		CString				SerialNumber;
		CString				SerialNumberReverse;
		CString				FirmwareRev;
		CString				FirmwareRevReverse;
		CString				Model;
		CString				ModelReverse;
		CString				ModelWmi;
		CString				ModelSerial;
		CString				DriveMap;
		CString				MaxTransferMode;
		CString				CurrentTransferMode;
		CString				MajorVersion;
		CString				MinorVersion;
		CString				Interface;
		CString				Enclosure;
		CString				CommandTypeString;
		CString				SsdVendorString;
		CString				DeviceNominalFormFactor;
		CString				PnpDeviceId;

		CString				SmartKeyName;
	};

	struct EXTERNAL_DISK_INFO
	{
		CString Enclosure;
		DWORD	UsbVendorId;
		DWORD	UsbProductId;
	};

	CAtlArray<ATA_SMART_INFO> vars;
	CAtlArray<EXTERNAL_DISK_INFO> externals;

	CAtlArray<CString> m_IdeController;
	CAtlArray<CString> m_ScsiController;
	CAtlArray<CString> m_UsbController;
	CString m_ControllerMap;
	CAtlArray<CString> m_BlackIdeController;
	CAtlArray<CString> m_BlackScsiController;
	CAtlArray<CString> m_SiliconImageController;
	CAtlArray<CString> m_UASPController;
	CAtlArray<DWORD> m_SiliconImageControllerType;
	CAtlArray<INT> m_BlackPhysicalDrive;

	BOOL IsAdvancedDiskSearch;
	BOOL IsEnabledWmi;

	BOOL IsWorkaroundHD204UI;
	BOOL IsWorkaroundAdataSsd;

	DWORD MeasuredGetTickCount;

	BOOL FlagNvidiaController;
	BOOL FlagMarvellController;
	BOOL FlagUsbSat;
	BOOL FlagUsbSunplus;
	BOOL FlagUsbIodata;
	BOOL FlagUsbLogitec;
	BOOL FlagUsbJmicron;
	BOOL FlagUsbCypress;
	BOOL FlagUsbMemory;
	DWORD CsmiType;

	DWORD CheckDiskStatus(DWORD index);

protected:
	OSVERSIONINFOEX m_Os;
	CString m_SerialNumberA_Z[26];
	BOOL m_FlagAtaPassThrough;
	BOOL m_FlagAtaPassThroughSmart;

	BOOL GetDiskInfo(INT physicalDriveId, INT scsiPort, INT scsiTargetId, INTERFACE_TYPE interfaceType, VENDOR_ID vendorId, DWORD productId = 0, INT scsiBus = -1, DWORD siliconImageId = 0, BOOL FlagNvidiaController = 0, BOOL FlagMarvellController = 0, CString pnpDeviceId = _T(""));
	BOOL AddDisk(INT PhysicalDriveId, INT ScsiPort, INT scsiTargetId, INT scsiBus, BYTE target, COMMAND_TYPE commandType, IDENTIFY_DEVICE* identify, INT siliconImageType = -1, PCSMI_SAS_PHY_ENTITY sasPhyEntity = NULL, CString pnpDeviceId = _T(""));
	DWORD CheckSmartAttributeUpdate(DWORD index, SMART_ATTRIBUTE* pre, SMART_ATTRIBUTE* cur);

	BOOL CheckSmartAttributeCorrect(ATA_SMART_INFO* asi1, ATA_SMART_INFO* asi2);

	VOID WakeUp(INT physicalDriveId);
	VOID InitAtaInfo();
	VOID InitAtaInfoByWmi();
	VOID InitStruct();
	VOID ChangeByteOrder(PCHAR str, DWORD length);
	BOOL CheckAsciiStringError(PCHAR str, DWORD length);
	HANDLE GetIoCtrlHandle(BYTE index);
	HANDLE GetIoCtrlHandle(INT scsiPort, DWORD siliconImageType);
	HANDLE GetIoCtrlHandleCsmi(INT scsiPort);
	BOOL SendAtaCommand(DWORD i, BYTE main, BYTE sub, BYTE param);

	BOOL DoIdentifyDevicePd(INT physicalDriveId, BYTE target, IDENTIFY_DEVICE* identify);
	BOOL GetSmartAttributePd(INT physicalDriveId, BYTE target, ATA_SMART_INFO* asi);
	BOOL GetSmartThresholdPd(INT physicalDriveId, BYTE target, ATA_SMART_INFO* asi);
	BOOL ControlSmartStatusPd(INT physicalDriveId, BYTE target, BYTE command);
	BOOL SendAtaCommandPd(INT physicalDriveId, BYTE target, BYTE main, BYTE sub, BYTE param, PBYTE data, DWORD dataSize);

	BOOL DoIdentifyDeviceScsi(INT scsiPort, INT scsiTargetId, IDENTIFY_DEVICE* identify);
	BOOL GetSmartAttributeScsi(INT scsiPort, INT scsiTargetId, ATA_SMART_INFO* asi);
	BOOL GetSmartThresholdScsi(INT scsiPort, INT scsiTargetId, ATA_SMART_INFO* asi);
	BOOL ControlSmartStatusScsi(INT scsiPort, INT scsiTargetId, BYTE command);
	BOOL SendAtaCommandScsi(INT scsiPort, INT scsiTargetId, BYTE main, BYTE sub, BYTE param);

	BOOL DoIdentifyDeviceSat(INT physicalDriveId, BYTE target, IDENTIFY_DEVICE* identify, COMMAND_TYPE commandType);
	BOOL GetSmartAttributeSat(INT physicalDriveId, BYTE target, ATA_SMART_INFO* asi);
	BOOL GetSmartThresholdSat(INT physicalDriveId, BYTE target, ATA_SMART_INFO* asi);
	BOOL ControlSmartStatusSat(INT physicalDriveId, BYTE target, BYTE command, COMMAND_TYPE commandType);
	BOOL SendAtaCommandSat(INT physicalDriveId, BYTE target, BYTE main, BYTE sub, BYTE param, COMMAND_TYPE commandType);

	BOOL DoIdentifyDeviceSi(INT physicalDriveId, INT scsiPort, INT scsiBus, DWORD siliconImageId, IDENTIFY_DEVICE* identify);
	BOOL GetSmartAttributeSi(INT physicalDriveId, ATA_SMART_INFO* asi);
	BOOL GetSmartThresholdSi(INT physicalDriveId, ATA_SMART_INFO* asi);

	BOOL GetSmartAttributeWmi(ATA_SMART_INFO* asi);
	BOOL GetSmartThresholdWmi(ATA_SMART_INFO* asi);
	BOOL GetSmartInfoWmi(DWORD type, ATA_SMART_INFO* asi);

	BOOL AddDiskCsmi(INT scsiPort);
//	BOOL GetPhyInfo(INT scsiPort, CSMI_SAS_PHY_INFO & phyInfo);
	BOOL CsmiIoctl(HANDLE hHandle, UINT code, SRB_IO_CONTROL *csmiBuf, UINT csmiBufSize);
	BOOL DoIdentifyDeviceCsmi(INT scsiPort, PCSMI_SAS_PHY_ENTITY sasPhyEntity, IDENTIFY_DEVICE* identify);
	BOOL GetSmartAttributeCsmi(INT scsiPort, PCSMI_SAS_PHY_ENTITY sasPhyEntity, ATA_SMART_INFO* asi);
	BOOL GetSmartThresholdCsmi(INT scsiPort, PCSMI_SAS_PHY_ENTITY sasPhyEntity, ATA_SMART_INFO* asi);
	BOOL ControlSmartStatusCsmi(INT scsiPort, PCSMI_SAS_PHY_ENTITY sasPhyEntity, BYTE command);
	BOOL SendAtaCommandCsmi(INT scsiPort, PCSMI_SAS_PHY_ENTITY sasPhyEntity, BYTE main, BYTE sub, BYTE param, PBYTE data, DWORD dataSize);

	DWORD GetTransferMode(WORD w63, WORD w76, WORD w77, WORD w88, CString &currentTransferMode, CString &maxTransferMode, CString &Interface, INTERFACE_TYPE *interfaceType);
	DWORD GetTimeUnitType(CString model, CString firmware, DWORD major, DWORD transferMode);
	DWORD GetAtaMajorVersion(WORD w80, CString &majorVersion);
	VOID  GetAtaMinorVersion(WORD w81, CString &minor);
//	DWORD GetMaxtorPowerOnHours(DWORD currentValue, DWORD rawValue);

	BOOL FillSmartData(ATA_SMART_INFO* asi);
	BOOL FillSmartThreshold(ATA_SMART_INFO* asi);
	
	VOID CheckSsdSupport(ATA_SMART_INFO &asi);
	BOOL IsSsdOld(ATA_SMART_INFO &asi);
	BOOL IsSsdMtron(ATA_SMART_INFO &asi);
	BOOL IsSsdIndlinx(ATA_SMART_INFO &asi);
	BOOL IsSsdJMicron60x(ATA_SMART_INFO &asi);
	BOOL IsSsdJMicron61x(ATA_SMART_INFO &asi);
	BOOL IsSsdIntel(ATA_SMART_INFO &asi);
	BOOL IsSsdSamsung(ATA_SMART_INFO &asi);
	BOOL IsSsdSandForce(ATA_SMART_INFO &asi);
	BOOL IsSsdMicron(ATA_SMART_INFO &asi);
	BOOL IsSsdOcz(ATA_SMART_INFO &asi);
	BOOL IsSsdPlextor(ATA_SMART_INFO &asi);
	BOOL IsSsdSanDisk(ATA_SMART_INFO &asi);
	BOOL IsSsdOczVector(ATA_SMART_INFO &asi);
	INT CheckPlextorNandWritesUnit(ATA_SMART_INFO &asi);

	static int Compare(const void *p1, const void *p2);

	CString GetModelSerial(CString &model, CString &serialNumber);
};
