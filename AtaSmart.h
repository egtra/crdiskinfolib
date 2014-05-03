/*---------------------------------------------------------------------------*/
//       Author : hiyohiyo
//         Mail : hiyohiyo@crystalmark.info
//          Web : http://crystalmark.info/
//      License : The modified BSD license
//
//                           Copyright 2008-2009 hiyohiyo. All rights reserved.
/*---------------------------------------------------------------------------*/
// Reference : http://www.usefullcode.net/2007/02/hddsmart.html (ja)

#pragma once

#include "winioctl.h"
#include "SPTIUtil.h"

class CAtaSmart
{
public:
	static const int MAX_DISK = 32;// FIX
	static const int MAX_ATTRIBUTE = 30; // FIX
	static const int MAX_SEARCH_PHYSICAL_DRIVE = 32;
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

	enum TRANSFER_MODE
	{
		TRANSFER_MODE_UNKNOWN = 0,
		TRANSFER_MODE_PIO,
		TRANSFER_MODE_PIO_DMA,
		TRANSFER_MODE_ULTRA_DMA_16,
		TRANSFER_MODE_ULTRA_DMA_25,
		TRANSFER_MODE_ULTRA_DMA_33,
		TRANSFER_MODE_ULTRA_DMA_44,
		TRANSFER_MODE_ULTRA_DMA_66,
		TRANSFER_MODE_ULTRA_DMA_100,
		TRANSFER_MODE_ULTRA_DMA_133,
		TRANSFER_MODE_SATA_150,
		TRANSFER_MODE_SATA_300,
		TRANSFER_MODE_SATA_600
	};

	enum DISK_STATUS
	{
		DISK_STATUS_UNKNOWN = 0,
		DISK_STATUS_GOOD,
		DISK_STATUS_CAUTION,
		DISK_STATUS_BAD
	};

	enum POWER_ON_HOURS_UNIT
	{
		POWER_ON_UNKNOWN = 0,
		POWER_ON_HOURS,
		POWER_ON_MINUTES,
		POWER_ON_HALF_MINUTES,
		POWER_ON_SECONDS,
	};

	enum COMMAND_TYPE
	{
		CMD_TYPE_PHYSICAL_DRIVE = 0,
		CMD_TYPE_SCSI_MINIPORT,
		CMD_TYPE_SAT,				// SAT = SCSI_ATA_TRANSLATION
		CMD_TYPE_SUNPLUS,
		CMD_TYPE_IO_DATA,
		CMD_TYPE_LOGITEC,
		CMD_TYPE_JMICRON,
		CMD_TYPE_CYPRESS,
		CMD_TYPE_PROLIFIC,
		CMD_TYPE_DEBUG
	};

	enum VENDOR_ID
	{
		HDD_GENERAL         = 0,
		SSD_GENERAL         = 1,
		SSD_VENDOR_MTRON    = 2,
		SSD_VENDOR_INDILINX = 3,
		SSD_VENDOR_JMICRON  = 4,
		SSD_VENDOR_INTEL    = 5,
		SSD_VENDOR_SAMSUNG  = 6,

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

	enum INTERFACE_TYPE
	{
		INTERFACE_TYPE_UNKNOWN = 0,
		INTERFACE_TYPE_PATA,
		INTERFACE_TYPE_SATA,
		INTERFACE_TYPE_USB,
		INTERFACE_TYPE_IEEE1394
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
		DWORD   DataBufferOffset;
	//	DWORD_PTR   DataBufferOffset;
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

	typedef struct SRB_IO_CONTROL
	{
	   ULONG	HeaderLength;
	   UCHAR	Signature[8];
	   ULONG	Timeout;
	   ULONG	ControlCode;
	   ULONG	ReturnCode;
	   ULONG	Length;
	};

	typedef struct SRB_IO_COMMAND
	{
		SRB_IO_CONTROL	Cntrol;
		IDEREGS			IdeRegs;
		BYTE			Data[512];
	};

	struct IDENTIFY_DEVICE
	{
		WORD		GeneralConfiguration;					//0
		WORD		LogicalCylinders;						//1	Obsolete
		WORD		SpecificConfiguration;					//2
		WORD		LogicalHeads;							//3 Obsolete
		WORD		Retired1[2];							//4-5
		WORD		LogicalSectors;							//6 Obsolete
		DWORD		ReservedForCompactFlash;				//7-8
		WORD		Retired2;								//9
		CHAR		SerialNumber[20];						//10-19
		WORD		Retired3;								//20
		WORD		BufferSize;								//21 Obsolete
		WORD		Obsolute4;								//22
		CHAR		FirmwareRev[8];							//23-26
		CHAR		Model[40];								//27-46
		WORD		MaxNumPerInterupt;						//47
		WORD		Reserved1;								//48
		WORD		Capabilities1;							//49
		WORD		Capabilities2;							//50
		DWORD		Obsolute5;								//51-52
		WORD		Field88and7064;							//53
		WORD		Obsolute6[5];							//54-58
		WORD		MultSectorStuff;						//59
		DWORD		TotalAddressableSectors;				//60-61
		WORD		Obsolute7;								//62
		WORD		MultiWordDma;							//63
		WORD		PioMode;								//64
		WORD		MinMultiwordDmaCycleTime;				//65
		WORD		RecommendedMultiwordDmaCycleTime;		//66
		WORD		MinPioCycleTimewoFlowCtrl;				//67
		WORD		MinPioCycleTimeWithFlowCtrl;			//68
		WORD		Reserved2[6];							//69-74
		WORD		QueueDepth;								//75
		WORD		SerialAtaCapabilities;					//76
		WORD		ReservedForFutureSerialAta;				//77
		WORD		SerialAtaFeaturesSupported;				//78
		WORD		SerialAtaFeaturesEnabled;				//79
		WORD		MajorVersion;							//80
		WORD		MinorVersion;							//81
		WORD		CommandSetSupported1;					//82
		WORD		CommandSetSupported2;					//83
		WORD		CommandSetSupported3;					//84
		WORD		CommandSetEnabled1;						//85
		WORD		CommandSetEnabled2;						//86
		WORD		CommandSetDefault;						//87
		WORD		UltraDmaMode;							//88
		WORD		TimeReqForSecurityErase;				//89
		WORD		TimeReqForEnhancedSecure;				//90
		WORD		CurrentPowerManagement;					//91
		WORD		MasterPasswordRevision;					//92
		WORD		HardwareResetResult;					//93
		WORD		AcoustricManagement;					//94
		WORD		StreamMinRequestSize;					//95
		WORD		StreamingTimeDma;						//96
		WORD		StreamingAccessLatency;					//97
		DWORD		StreamingPerformance;					//98-99
		ULONGLONG	MaxUserLba;								//100-103
		WORD		StremingTimePio;						//104
		WORD		Reserved3;								//105
		WORD		SectorSize;								//106
		WORD		InterSeekDelay;							//107
		WORD		IeeeOui;								//108
		WORD		UniqueId3;								//109
		WORD		UniqueId2;								//110
		WORD		UniqueId1;								//111
		WORD		Reserved4[4];							//112-115
		WORD		Reserved5;								//116
		DWORD		WordsPerLogicalSector;					//117-118
		WORD		Reserved6[8];							//119-126
		WORD		RemovableMediaStatus;					//127
		WORD		SecurityStatus;							//128
		WORD		VendorSpecific[31];						//129-159
		WORD		CfaPowerMode1;							//160
		WORD		ReservedForCompactFlashAssociation[7];	//161-167
		WORD		DeviceNominalFormFactor;				//168
		WORD		DataSetManagement;						//169
		WORD		AdditionalProductIdentifier[4];			//170-173
		WORD		Reserved7[2];							//174-175
		CHAR		CurrentMediaSerialNo[60];				//176-205
		WORD		SctCommandTransport;					//206
		WORD		ReservedForCeAta1[2];					//207-208
		WORD		AlignmentOfLogicalBlocks;				//209
		DWORD		WriteReadVerifySectorCountMode3;		//210-211
		DWORD		WriteReadVerifySectorCountMode2;		//212-213
		WORD		NvCacheCapabilities;					//214
		DWORD		NvCacheSizeLogicalBlocks;				//215-216
		WORD		NominalMediaRotationRate;				//217
		WORD		Reserved8;								//218
		WORD		NvCacheOptions1;						//219
		WORD		NvCacheOptions2;						//220
		WORD		Reserved9;								//221
		WORD		TransportMajorVersionNumber;			//222
		WORD		TransportMinorVersionNumber;			//223
		WORD		ReservedForCeAta2[10];					//224-233
		WORD		MinimumBlocksPerDownloadMicrocode;		//234
		WORD		MaximumBlocksPerDownloadMicrocode;		//235
		WORD		Reserved10[19];							//236-254
		WORD		IntegrityWord;							//255
	};
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

	BOOL Init(BOOL useWmi, BOOL advancedDiskSearch, PBOOL flagChangeDisk);
	BOOL MeasuredTimeUnit();
	DWORD GetPowerOnHours(DWORD rawValue, DWORD timeUnitType);
	DWORD GetPowerOnHoursEx(DWORD index, DWORD timeUnitType);

	struct DISK_POSITION
	{
		INT					PhysicalDriveId;
		INT					ScsiPort;
		INT					ScsiTargetId;
	};

	struct ATA_SMART_INFO
	{
		IDENTIFY_DEVICE		IdentifyDevice;
		WORD				SmartReadData[256];
		WORD				SmartReadThreshold[256];
		SMART_ATTRIBUTE		Attribute[MAX_ATTRIBUTE];
		SMART_THRESHOLD		Threshold[MAX_ATTRIBUTE];

		BOOL				IsSmartEnabled;
		BOOL				IsIdInfoIncorrect;
		BOOL				IsSmartCorrect;
		BOOL				IsCheckSumError;
		BOOL				IsWord88;
		BOOL				IsWord64_76;
		BOOL				IsRawValues8;

		BOOL				IsSmartSupported;
		BOOL				IsLba48Supported;
		BOOL				IsAamSupported;
		BOOL				IsApmSupported;
		BOOL				IsAamEnabled;
		BOOL				IsApmEnabled;
		BOOL				IsNcqSupported;
		BOOL				IsNvCacheSupported;
		BOOL				IsMaxtorMinute;
		BOOL				IsSsd;
		BOOL				IsTrimSupported;

		INT					PhysicalDriveId;
		INT					ScsiPort;
		INT					ScsiTargetId;
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
		DWORD				NominalMediaRotationRate;
//		double				Speed;
		ULONGLONG			HostWrites;

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

		DWORD				VendorId;
		DWORD				ProductId;
		BYTE				Target;

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

		CString				SmartKeyName;
	};

	struct EXTERNAL_DISK_INFO
	{
		CString Enclosure;
		DWORD	VendorId;
		DWORD	ProductId;
	};

	CArray<ATA_SMART_INFO, ATA_SMART_INFO> vars;
	CArray<EXTERNAL_DISK_INFO, EXTERNAL_DISK_INFO> externals;

	CStringArray m_IdeController;
	CStringArray m_ScsiController;
	CStringArray m_UsbController;
	CString m_ControllerMap;

	BOOL IsAdvancedDiskSearch;
	BOOL IsEnabledWmi;
	DWORD MeasuredGetTickCount;

	BOOL FlagUsbSat;
	BOOL FlagUsbSunplus;
	BOOL FlagUsbIodata;
	BOOL FlagUsbLogitec;
	BOOL FlagUsbJmicron;
	BOOL FlagUsbCypress;

	WORD Threshold05;
	WORD ThresholdC5;
	WORD ThresholdC6;

protected:
	OSVERSIONINFOEX m_Os;
	CString m_SerialNumberA_Z[26];
	BOOL m_FlagAtaPassThrough;

	BOOL GetDiskInfo(INT physicalDriveId, INT scsiPort, INT scsiTargetId, INTERFACE_TYPE interfaceType, VENDOR_ID vendorId, DWORD productId = 0);
	BOOL AddDisk(INT PhysicalDriveId, INT ScsiPort, INT scsiTargetId, BYTE target, COMMAND_TYPE commandType, IDENTIFY_DEVICE* identify);
	DWORD CheckSmartAttributeUpdate(DWORD index, SMART_ATTRIBUTE* pre, SMART_ATTRIBUTE* cur);

	BOOL CheckSmartAttributeCorrect(ATA_SMART_INFO* asi1, ATA_SMART_INFO* asi2);

	VOID WakeUp(INT physicalDriveId);
	VOID InitAtaInfo();
	VOID InitAtaInfoByWmi();
	VOID InitStruct();
	VOID ChangeByteOrder(PCHAR str, DWORD length);
	BOOL CheckAsciiStringError(PCHAR str, DWORD length);
	HANDLE GetIoCtrlHandle(BYTE index);
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

	DWORD CheckDiskStatus(SMART_ATTRIBUTE* attribute, SMART_THRESHOLD* threshold, DWORD attributeCount, DWORD vendorId, BOOL isSmartCorrect, BOOL isSsd);

	DWORD GetTransferMode(WORD w63, WORD w76, WORD w88, CString &currentTransferMode, CString &maxTransferMode, CString &Interface, INTERFACE_TYPE *interfaceType);
	DWORD GetTimeUnitType(CString model, CString firmware, DWORD major, DWORD transferMode);
	DWORD GetAtaMajorVersion(WORD w80, CString &majorVersion);
	VOID  GetAtaMinorVersion(WORD w81, CString &minor);
//	DWORD GetMaxtorPowerOnHours(DWORD currentValue, DWORD rawValue);

	void CheckSsdSupport(ATA_SMART_INFO &asi);
	BOOL IsSsdOld(ATA_SMART_INFO &asi);
	BOOL IsSsdMtron(ATA_SMART_INFO &asi);
	BOOL IsSsdIndlinx(ATA_SMART_INFO &asi);
	BOOL IsSsdJMicron(ATA_SMART_INFO &asi);
	BOOL IsSsdIntel(ATA_SMART_INFO &asi);
	BOOL IsSsdSamsung(ATA_SMART_INFO &asi);

	static int Compare(const void *p1, const void *p2);
};
