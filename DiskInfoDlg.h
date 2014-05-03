/*---------------------------------------------------------------------------*/
//       Author : hiyohiyo
//         Mail : hiyohiyo@crystalmark.info
//          Web : http://crystalmark.info/
//      License : The modified BSD license
//
//                           Copyright 2008-2009 hiyohiyo. All rights reserved.
/*---------------------------------------------------------------------------*/

#pragma once

#include "AtaSmart.h"
#include "AboutDlg.h"
#include "SettingDlg.h"
#include "HealthDlg.h"
#include "OptionDlg.h"

#include "ListCtrlEx.h"
#include "GetOsInfo.h"
#include "EventLog.h"

#include <Dbt.h>

#define REGISTRY_PATH _T("software\\Crystal Dew World\\CrystalDiskInfo")
// CDiskInfoDlg dialog
class CDiskInfoDlg : public CDHtmlMainDialog
{
// Construction
public:
	CDiskInfoDlg(CWnd* pParent /*=NULL*/, BOOL flagStarupExit);
	virtual ~CDiskInfoDlg();

	CAtaSmart m_Ata;

// Dialog Data
	enum { IDD = IDD_DISKINFO_DIALOG, IDH = IDR_HTML_DUMMY };

	static const int SIZE_X = 640;
	static const int SIZE_SMART_X = 640;

#ifdef BENCHMARK
	static const int SIZE_Y = 275;
	static const int SIZE_SMART_Y = 520;
	static const int MAX_METER_LENGTH = 516;
#else
	static const int SIZE_Y = 235;
	static const int SIZE_SMART_Y = 480;
#endif

	// Timer
	static const int TIMER_SET_POWER_ON_UNIT = 0x1001;
	static const int TIMER_AUTO_REFRESH      = 0x1002;
	static const int TIMER_FORCE_REFRESH     = 0x1003;
	static const int TIMER_AUTO_DETECT       = 0x1004;
	static const int TIMER_UPDATE_TRAY_ICON  = 0x1005;

	// Setting
	int SAVE_SMART_PERIOD;			// sec
	int ALARM_TEMPERATURE_PERIOD;	// sec

	// Task Tray
	enum
	{
		MY_EXIT = (WM_APP + 0x1100),
		MY_SHOW_MAIN_DIALOG,
		MY_SHOW_TEMPERATURE_ICON_ONLY,
	};

	HMENU m_hMenu;
	void CreateMainMenu(DWORD index);

	DWORD GetSelectDisk();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

// Implementation
protected:
	HICON m_hIcon;
	HICON m_hIconMini;
	HICON m_hTempIcon[2][100];
	UINT m_TempIconIndex[CAtaSmart::MAX_DISK];

	UINT m_MainIconId;

	CAboutDlg*		m_AboutDlg;
	CSettingDlg*	m_SettingDlg;
	CHealthDlg*		m_HealthDlg;
	COptionDlg*		m_OptionDlg;
	CListCtrlEx		m_List;
	CImageList		m_ImageList;

	HDEVNOTIFY m_hDevNotify;

	CString		m_SmartDir;
	CString		m_ExeDir;

	DWORD m_SelectDisk;
	DWORD m_DriveMenuPage;
	DWORD m_AutoRefreshStatus;
	DWORD m_WaitTimeStatus;
	DWORD m_AutoDetectionStatus;
	DWORD m_RawValues;
	BOOL m_NowDetectingUnitPowerOnHours;

	CArray<DWORD, DWORD> m_GraphProcessId;

	int m_SizeX;
	int m_SizeY;

	int m_PreTemp[CAtaSmart::MAX_DISK];
	BOOL m_FlagTrayTemperatureIcon[CAtaSmart::MAX_DISK];
	BOOL m_FlagAutoRefreshTarget[CAtaSmart::MAX_DISK];
	BOOL m_FlagTrayMainIcon;

	// Options
	BOOL m_FlagStartupExit;
	BOOL m_FlagHideSmartInfo;
	BOOL m_FlagHideSerialNumber;
	BOOL m_FlagAdvancedDiskSearch;
	BOOL m_FlagEventLog;
	BOOL m_FlagAtaPassThroughSmart;
//	BOOL m_FlagUseEventCreate;		// Use eventcreate.exe (XP Pro or later)
	BOOL m_FlagFahrenheit;
	BOOL m_FlagAutoAamApm;
	BOOL m_FlagDumpIdentifyDevice;
	BOOL m_FlagDumpSmartReadData;
	BOOL m_FlagDumpSmartReadThreshold;
	BOOL m_FlagShowTemperatureIconOnly;
	BOOL m_FlagAsciiView;
	BOOL m_FlagSidebar;

	BOOL AddTemperatureIcon(DWORD index);
	BOOL RemoveTemperatureIcon(DWORD index);
	BOOL ModifyTemperatureIcon(DWORD index);

	CString m_LiDisk[8];

	CString m_Model;
	CString m_Firmware;
	CString m_SerialNumber;
	CString m_Capacity;
	CString m_Temperature;
	CString m_PowerOnHours;
	CString m_PowerOnCount;
	CString m_Feature;
	CString m_BufferSize;
	CString m_NvCacheSize;
	CString m_RotationRate;
	CString m_LbaSize;
	CString m_DriveMap;
	CString m_Interface;
	CString m_TransferMode;
	CString m_AtaAtapi;
	CString m_DiskStatus;
	CString m_SmartStatus;

	CString m_LabelFirmware;
	CString m_LabelSerialNumber;
	CString m_LabelCapacity;
	CString m_LabelTemperature;
	CString m_LabelPowerOnHours;
	CString m_LabelPowerOnCount;
	CString m_LabelFeature;
	CString m_LabelBufferSize;
	CString m_LabelNvCacheSize;
	CString m_LabelRotationRate;
	CString m_LabelLbaSize;
	CString m_LabelDriveMap;
	CString m_LabelInterface;
	CString m_LabelTransferMode;
	CString m_LabelAtaAtapi;
	CString m_LabelDiskStatus;
	CString m_LabelSmartStatus;
#ifdef BENCHMARK
	CString m_BenchmarkMeter;
#endif

	CString m_StatusTip;
	CString m_PowerOnHoursClass;

	BOOL ChangeDisk(DWORD index);
	BOOL UpdateListCtrl(DWORD index);
	void SelectDrive(DWORD index);
	void InitDriveList();
	void InitListCtrl();
	BOOL InitAta(BOOL useWmi, BOOL advancedDiskSearch, PBOOL flagChangeDisk);

	void ChangeLang(CString LangName);
	void UpdateDialogSize();
	void CheckHideSerialNumber();
	void CheckResident();
	void CheckStartup();
	void AutoAamApmAdaption();
	void ShowTemperatureIconOnly();

#ifdef GADGET_SUPPORT
	void UpdateShareInfo(); // For Sidebar Gadget Support
	void DeleteShareInfo();
#endif

	BOOL AddTrayMainIcon();
	BOOL RemoveTrayMainIcon();

	CString GetDiskStatus(DWORD statusCode);
	CString GetDiskStatusClass(DWORD statusCode);
	CString CDiskInfoDlg::GetDiskStatusReason(DWORD index);
	CString GetTemperatureClass(DWORD temperature);
	CString GetLogicalDriveInfo(DWORD index);

	CString Encode10X(DWORD value);
	DWORD Decode10X(CString cstr);

	void CheckRadioAutoRefresh(int id, int value);
	void CheckRadioWaitTime(int id, int value);
	BOOL CheckRadioZoomType(int id, int value);
	void CheckRadioRawValues(int id, int value);
	void CheckRadioAutoDetection(int id, int value);
	void CheckRadioAutoRefresh();
	void CheckRadioWaitTime();
	void CheckRadioZoomType();
	void CheckRadioRawValues();
	void CheckRadioAutoDetection();
	void ReExecute();
	void AlarmOverheat();
	void AlarmHealthStatus(DWORD i, CString dir, CString disk);
	void CheckPage();
	void CheckTrayTemperatureIcon();
	void UpdateTrayTemperatureIcon(BOOL flagForce);
	void UpdateToolTip();
	BOOL IsTemperatureIconExist();

	void TaskTrayRightMenu(DWORD index);
	void SaveSmartInfo(DWORD index);
	void ShowGraphDlg(int index);
	void CreateExchangeInfo();
	void KillGraphDlg();

	BOOL RegisterStartup();
	BOOL UnregisterStartup();

	CString __Number(DWORD value);
	CHAR AsciiFilter(BYTE c);

	// Generated message map functions
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual void OnDocumentComplete(LPDISPATCH pDisp, LPCTSTR szUrl);

	HRESULT OnDisk0(IHTMLElement *pElement);
	HRESULT OnDisk1(IHTMLElement *pElement);
	HRESULT OnDisk2(IHTMLElement *pElement);
	HRESULT OnDisk3(IHTMLElement *pElement);
	HRESULT OnDisk4(IHTMLElement *pElement);
	HRESULT OnDisk5(IHTMLElement *pElement);
	HRESULT OnDisk6(IHTMLElement *pElement);
	HRESULT OnDisk7(IHTMLElement *pElement);
	HRESULT OnPreDisk(IHTMLElement *pElement);
	HRESULT OnNextDisk(IHTMLElement *pElement);
	HRESULT OnDiskStatus(IHTMLElement *pElement);
#ifdef BENCHMARK
	HRESULT OnBenchmark(IHTMLElement *pElement);
	void SetMeter(double score);
#endif

	LRESULT OnPowerBroadcast(WPARAM wParam, LPARAM lParam);
	LRESULT OnDeviceChange(WPARAM wParam, LPARAM lParam);

	void Refresh(DWORD flagForceUpdate);
	BOOL AppendLog(CString dir, CString disk, CString file, CTime time, int value, BOOL firstTime = FALSE, int threshold = 0);
	BOOL AddEventLog(DWORD eventId, WORD eventType, CString message);

	void RebuildListHeader(DWORD index, BOOL forceUpdate = FALSE);

	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
	DECLARE_DHTML_EVENT_MAP()
public:
	afx_msg void OnExit();
	afx_msg void OnAbout();
	afx_msg void OnHideSmartInfo();
	afx_msg void OnHideSerialNumber();
	afx_msg void OnEditCopy();
	afx_msg void OnCrystalDewWorld();
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnRefresh();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnHelpAboutSmart();
	afx_msg void OnAutoRefreshDisable();
	afx_msg void OnAutoRefresh01Min();
	afx_msg void OnAutoRefresh03Min();
	afx_msg void OnAutoRefresh05Min();
	afx_msg void OnAutoRefresh10Min();
	afx_msg void OnAutoRefresh30Min();
	afx_msg void OnAutoRefresh60Min();
	afx_msg void OnOpenDiskManagement();
	afx_msg void OnOpenDeviceManager();
	afx_msg void OnAdvancedDiskSearch();
	afx_msg void OnResident();

	// Task Tray
	afx_msg LRESULT OnRegMessage(WPARAM, LPARAM);
	afx_msg LRESULT OnTaskbarCreated(WPARAM, LPARAM);
	afx_msg LRESULT OnTempIcon0(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnTempIcon1(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnTempIcon2(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnTempIcon3(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnTempIcon4(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnTempIcon5(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnTempIcon6(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnTempIcon7(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnTempIcon8(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnTempIcon9(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnTempIcon10(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnTempIcon11(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnTempIcon12(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnTempIcon13(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnTempIcon14(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnTempIcon15(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnTempIcon16(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnTempIcon17(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnTempIcon18(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnTempIcon19(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnTempIcon20(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnTempIcon21(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnTempIcon22(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnTempIcon23(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnTempIcon24(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnTempIcon25(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnTempIcon26(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnTempIcon27(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnTempIcon28(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnTempIcon29(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnTempIcon30(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnTempIcon31(WPARAM wParam,LPARAM lParam);
	afx_msg void OnGraph();
	afx_msg void OnHelp();
	afx_msg void OnCustomize();
	afx_msg void OnStartup();
	afx_msg void OnWait0Sec();
	afx_msg void OnWait5Sec();
	afx_msg void OnWait10Sec();
	afx_msg void OnWait15Sec();
	afx_msg void OnWait20Sec();
	afx_msg void OnWait30Sec();
	afx_msg void OnWait40Sec();
	afx_msg void OnWait50Sec();
	afx_msg void OnWait60Sec();
	afx_msg void OnWait90Sec();
	afx_msg void OnWait120Sec();
	afx_msg void OnWait150Sec();
	afx_msg void OnWait180Sec();
	afx_msg void OnWait210Sec();
	afx_msg void OnWait240Sec();
	afx_msg void OnAutoDetection05Sec();
	afx_msg void OnAutoDetection10Sec();
	afx_msg void OnAutoDetection20Sec();
	afx_msg void OnAutoDetection30Sec();
	afx_msg void OnAutoDetectionDisable();
	afx_msg void OnEventLog();
	afx_msg void OnAtaPassThroughSmart();
	afx_msg void OnCelsius();
	afx_msg void OnFahrenheit();
	afx_msg void OnAamApm();
	afx_msg void OnAutoAamApm();
	afx_msg void OnRescan();
	afx_msg void OnUsbSat();
	afx_msg void OnUsbIodata();
	afx_msg void OnUsbSunplus();
	afx_msg void OnUsbLogitec();
	afx_msg void OnUsbJmicron();
	afx_msg void OnUsbCypress();
	afx_msg void OnUsbEnableAll();
	afx_msg void OnUsbDisableAll();
	afx_msg void OnHealthStatus();
	afx_msg void OnDumpIdentifyDevice();
	afx_msg void OnDumpSmartReadData();
	afx_msg void OnDumpSmartReadThreshold();
	afx_msg void OnResidentMinimize();
	afx_msg void OnResidentHide();
	afx_msg void OnZoom100();
	afx_msg void OnZoom125();
	afx_msg void OnZoom150();
	afx_msg void OnZoom200();
	afx_msg void OnZoomAuto();
	afx_msg void OnRawValues16();
	afx_msg void OnRawValues10All();
	afx_msg void OnRawValues2byte();
	afx_msg void OnRawValues1byte();
//	afx_msg void OnAutoDetection();
	afx_msg void OnAsciiView();
};	
