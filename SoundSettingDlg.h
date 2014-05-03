/*---------------------------------------------------------------------------*/
//       Author : hiyohiyo
//         Mail : hiyohiyo@crystalmark.info
//          Web : http://crystalmark.info/
//      License : Simplified BSD license
//
//                                Copyright 212 hiyohiyo. All rights reserved.
/*---------------------------------------------------------------------------*/


#pragma once

class CSoundSettingDlg : public CDHtmlDialogEx
{
	DECLARE_DYNCREATE(CSoundSettingDlg)

	static const int SIZE_X = 480;
	static const int SIZE_Y = 48;

public:
	CSoundSettingDlg(CWnd* pParent = NULL);
	virtual ~CSoundSettingDlg();

	enum { IDD = IDD_SOUND_SETTINGS, IDH = IDR_HTML_DUMMY};

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	virtual void OnDocumentComplete(LPDISPATCH pDisp, LPCTSTR szUrl);

	CString m_SoundFile;

	HRESULT OnSelectSoundFile(IHTMLElement *pElement);
	HRESULT OnNoSoundFile(IHTMLElement *pElement);
	HRESULT OnPlaySoundFile(IHTMLElement *pElement);

	DECLARE_MESSAGE_MAP()
	DECLARE_DHTML_EVENT_MAP()
};
