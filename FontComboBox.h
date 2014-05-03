/*---------------------------------------------------------------------------*/
//       Author : hiyohiyo
//         Mail : hiyohiyo@crystalmark.info
//          Web : http://crystalmark.info/
//      License : The modified BSD license
//
//                                Copyright 2011 hiyohiyo. All rights reserved.
/*---------------------------------------------------------------------------*/

#pragma once

// CFontComboBox

class CFontComboBox : public CComboBox
{
	DECLARE_DYNAMIC(CFontComboBox)

public:
	CFontComboBox();
	virtual ~CFontComboBox();

protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

	CString fontFace;
};


