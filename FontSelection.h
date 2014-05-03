/*---------------------------------------------------------------------------*/
//       Author : hiyohiyo
//         Mail : hiyohiyo@crystalmark.info
//          Web : http://crystalmark.info/
//      License : The modified BSD license
//
//                                Copyright 2011 hiyohiyo. All rights reserved.
/*---------------------------------------------------------------------------*/
#pragma once
#include "FontComboBox.h"

class CFontSelection : public CDialog
{
	DECLARE_DYNAMIC(CFontSelection)

public:
	CFontSelection(CWnd* pParent, CString fontFace, CString windowTitle);
	virtual ~CFontSelection();

	CString GetFontFace();

	enum { IDD = IDD_FONT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

	CString m_FontFace;
	CString m_WindowTitle;
	virtual void OnOK();
//	virtual void OnCancel();

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();

	CFontComboBox m_FontComboBox;
};
