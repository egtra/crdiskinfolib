/*---------------------------------------------------------------------------*/
//       Author : hiyohiyo
//         Mail : hiyohiyo@crystalmark.info
//          Web : http://crystalmark.info/
//      License : The modified BSD license
//
//                                Copyright 2008 hiyohiyo. All rights reserved.
/*---------------------------------------------------------------------------*/

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols

#define THEME_DIR					_T("CdiResource\\theme\\")
#define LANGUAGE_DIR				_T("CdiResource\\language\\")
#define DIALOG_DIR					_T("CdiResource\\dialog\\")
#define SMART_DIR					_T("Smart\\")
#define SMART_INI					_T("Smart.ini")
#define EXCHANGE_INI				_T("Exchange.ini")
#define ALERT_MAIL_PATH				_T("CdiResource\\AlertMail.exe")
#define ALERT_MAIL_4_PATH			_T("CdiResource\\AlertMail4.exe")


#define MENU_THEME_INDEX			3
#define MENU_LANG_INDEX				6
#define MENU_DRIVE_INDEX			4

#define MAIN_DIALOG					_T("Main.html")
//#define CLASSIC_DIALOG			_T("Classic.html")
#define ABOUT_DIALOG				_T("About.html")
#define SETTING_DIALOG				_T("Setting.html")
#define HEALTH_DIALOG				_T("Health.html")
#define GRAPH_DIALOG				_T("Graph.html")
#define OPTION_DIALOG				_T("Option.html")

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
	CString m_SmartDir;
	CString m_ExeDir;
	CString m_AlertMailPath;
	CString m_Ini;

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