/*---------------------------------------------------------------------------*/
//       Author : hiyohiyo
//         Mail : hiyohiyo@crystalmark.info
//          Web : http://crystalmark.info/
//      License : Simplified BSD license
/*---------------------------------------------------------------------------*/
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols

#include "resourceS.h"

#define THEME_DIR					_T("CdiResource\\theme\\")
#define LANGUAGE_DIR				_T("CdiResource\\language\\")
#define DIALOG_DIR					_T("CdiResource\\dialog\\")
#define GADGET_DIR					_T("CdiResource\\gadget\\")
#define SMART_DIR					_T("Smart\\")
#define SMART_INI					_T("Smart.ini")
#define EXCHANGE_INI				_T("Exchange.ini")
#define ALARM_HISTORY_CSV			_T("AlarmHistory.csv")
#define ALERT_MAIL_PATH				_T("CdiResource\\AlertMail.exe")
#define ALERT_MAIL_4_PATH			_T("CdiResource\\AlertMail4.exe")
#define OPUS_DEC_PATH				_T("CdiResource\\opus\\opusdec.exe")


#define MENU_THEME_INDEX			3
#define MENU_LANG_INDEX				6
#define MENU_DRIVE_INDEX			4

#ifdef SUISHO_SHIZUKU_SUPPORT
#define MAIN_DIALOG					_T("MainS.html")
#define MAIN_DIALOG_IE8				_T("MainS8.html")
#define ABOUT_DIALOG				_T("AboutS.html")
#else
#define MAIN_DIALOG					_T("Main.html")
#define MAIN_DIALOG_IE8				_T("Main8.html")
#define ABOUT_DIALOG				_T("About.html")
#endif

//#define CLASSIC_DIALOG			_T("Classic.html")
#define SETTING_DIALOG				_T("Setting.html")
#define HEALTH_DIALOG				_T("Health.html")
#define GRAPH_DIALOG				_T("Graph.html")
#define GRAPH_DIALOG_IE8			_T("Graph8.html")
#define OPTION_DIALOG				_T("Option.html")
//#define ALARM_HISTORY_DIALOG		_T("AlarmHistory.html")
#define SOUND_SETTING_DIALOG		_T("SoundSetting.html")

#define DEFAULT_THEME				THEME_DIR _T("default\\Main.css")
#define DEFAULT_LANGUAGE			LANGUAGE_DIR _T("English.lang")


class CDiskInfoApp : public CWinApp
{
public:
	CDiskInfoApp();

	CString m_MainDlgPath;
	CString m_AboutDlgPath;
	CString m_SettingDlgPath;
	CString m_HealthDlgPath;
	CString m_GraphDlgPath;
	CString m_OptionDlgPath;
//	CString m_AlarmHistoryDlgPath;
	CString m_SoundSettingDlgPath;
	CString m_SmartDir;
	CString m_ExeDir;
	CString m_GadgetDir;
	CString m_AlertMailPath;
	CString m_OpusDecPath;
	CString m_Ini;
	CString m_SaveAsText;
	BOOL m_FlagCopyExit;

	CString m_ThemeDir;
	CString m_LangDir;
	DWORD m_ThemeIndex;
	DWORD m_LangIndex;

// Overrides
	public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CDiskInfoApp theApp;