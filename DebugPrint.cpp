/*---------------------------------------------------------------------------*/
//       Author : hiyohiyo
//         Mail : hiyohiyo@crystalmark.info
//          Web : http://crystalmark.info/
//      License : The modified BSD license
//
//                                Copyright 2008 hiyohiyo. All rights reserved.
/*---------------------------------------------------------------------------*/

#include "stdafx.h"
#include "DebugPrint.h"

void DebugPrint(CString cstr)
{
	static int flag = TRUE;
	static TCHAR file[MAX_PATH];

	if(flag)
	{
		TCHAR* ptrEnd;
		::GetModuleFileName(NULL, file, MAX_PATH);
		if((ptrEnd = _tcsrchr(file, '.')) != NULL )
		{
			*ptrEnd = '\0';
			_tcscat_s(file, MAX_PATH, _T(".log"));
		}
		DeleteFile(file);
		flag = FALSE;
	}

	FILE *fp;
	_tfopen_s(&fp, file, _T("a"));
	_ftprintf(fp, _T("%s\n"), cstr);
	fclose(fp);
}