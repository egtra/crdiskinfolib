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
#include "FontComboBox.h"


// CFontComboBox

IMPLEMENT_DYNAMIC(CFontComboBox, CComboBox)

CFontComboBox::CFontComboBox()
{

}

CFontComboBox::~CFontComboBox()
{
}


BEGIN_MESSAGE_MAP(CFontComboBox, CComboBox)
END_MESSAGE_MAP()

void CFontComboBox::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
    CString cstr;
    if (lpDrawItemStruct->itemID == -1)
        return;
    GetLBText(lpDrawItemStruct->itemID, cstr);
    CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);

    CFont font;
    LOGFONT logfont;
    memset(&logfont, 0, sizeof(logfont));
    logfont.lfHeight = 18;
    logfont.lfWidth = 0;
    logfont.lfWeight = 400;
    logfont.lfCharSet = SHIFTJIS_CHARSET;
    pDC->SelectObject(&font);
    _tcscpy_s(logfont.lfFaceName, 32, (LPCTSTR)cstr);
    font.CreateFontIndirect(&logfont);
    pDC->SelectObject(&font);

    pDC->DrawText(cstr, &lpDrawItemStruct->rcItem, DT_SINGLELINE | DT_VCENTER);
}
