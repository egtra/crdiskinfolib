/*---------------------------------------------------------------------------*/
//       Author : hiyohiyo
//         Mail : hiyohiyo@crystalmark.info
//          Web : http://crystalmark.info/
//      License : Simplified BSD license
/*---------------------------------------------------------------------------*/

#pragma once

class CComboBoxCx : public CComboBox
{
	DECLARE_DYNAMIC(CComboBoxCx)

public:
	CComboBoxCx();
	virtual ~CComboBoxCx();

	void SetFontEx(CString face, int size, double zoomRatio);
	BOOL IsHighContrast();

protected:
	CFont    m_Font;
	DECLARE_MESSAGE_MAP()
};


