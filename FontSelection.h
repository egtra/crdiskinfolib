/*---------------------------------------------------------------------------*/
//       Author : hiyohiyo
//         Mail : hiyohiyo@crystalmark.info
//          Web : http://crystalmark.info/
//      License : Simplified BSD license
/*---------------------------------------------------------------------------*/

#pragma once
#include "FontComboBox.h"
#include "DialogCx.h"
#include "ButtonCx.h"

class CFontSelection : public CDialogCx
{
	DECLARE_DYNAMIC(CFontSelection)

	static const int SIZE_X = 320;
	static const int SIZE_Y = 80;

public:
	CFontSelection(CWnd* pParent = NULL);
	virtual ~CFontSelection();

	CString GetFontFace();

	enum { IDD = IDD_FONT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

//	CString m_FontFace;
//	CString m_WindowTitle;

	CButtonCx m_CtrlOk;

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual void UpdateDialogSize();

	CFontComboBox m_FontComboBox;
	afx_msg void OnBnClickedOk();
};
