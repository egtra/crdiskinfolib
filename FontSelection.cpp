/*---------------------------------------------------------------------------*/
//       Author : hiyohiyo
//         Mail : hiyohiyo@crystalmark.info
//          Web : http://crystalmark.info/
//      License : The modified BSD license
//
//                                Copyright 2011 hiyohiyo. All rights reserved.
/*---------------------------------------------------------------------------*/

#include "stdafx.h"
#include "DiskInfo.h"
#include "FontSelection.h"

int CALLBACK EnumFontFamExProc(ENUMLOGFONTEX *lpelfe, NEWTEXTMETRICEX *lpntme, int FontType, LPARAM lParam);

IMPLEMENT_DYNAMIC(CFontSelection, CDialog)

CFontSelection::CFontSelection(CWnd* pParent, CString fontFace, CString windowTitle)
	: CDialog(CFontSelection::IDD, pParent)
{
	m_FontFace = fontFace;
	m_WindowTitle = windowTitle;
}

CFontSelection::~CFontSelection()
{
}

void CFontSelection::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_FONT_COMBO, m_FontComboBox);
}


BEGIN_MESSAGE_MAP(CFontSelection, CDialog)
END_MESSAGE_MAP()


BOOL CFontSelection::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetWindowText(m_WindowTitle);

    CClientDC dc(this);
    LOGFONT logfont; 
    ZeroMemory(&logfont, sizeof(LOGFONT)); 
    logfont.lfCharSet = DEFAULT_CHARSET;
    ::EnumFontFamiliesEx(dc.m_hDC, &logfont, (FONTENUMPROC)EnumFontFamExProc, (long)&m_FontComboBox, 0);

	int no = m_FontComboBox.FindStringExact(0, m_FontFace);
	if(no >= 0)
	{
		m_FontComboBox.SetCurSel(no);
	}
	else
	{
		no = m_FontComboBox.FindStringExact(0, _T("Tahoma"));
		if(no >= 0)
		{
			m_FontComboBox.SetCurSel(no);
		}
		else
		{
			m_FontComboBox.SetCurSel(0);
		}
	}

	return TRUE;
}

CString CFontSelection::GetFontFace()
{
	return m_FontFace;
}

int CALLBACK EnumFontFamExProc(ENUMLOGFONTEX *lpelfe, NEWTEXTMETRICEX *lpntme, int FontType, LPARAM lParam)
{

   CFontComboBox* pFontComboBox = (CFontComboBox*)lParam;
	if(pFontComboBox->FindStringExact(0, (TCHAR*)lpelfe->elfFullName) == CB_ERR
	&& _tcschr((TCHAR*)lpelfe->elfFullName, _T('@')) == NULL
	)
	{
	    pFontComboBox->AddString((TCHAR*)lpelfe->elfFullName);
	}
    return TRUE;
}

void CFontSelection::OnOK()
{
	m_FontComboBox.GetLBText(m_FontComboBox.GetCurSel(), m_FontFace);

	CDialog::OnOK();
}