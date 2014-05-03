/*---------------------------------------------------------------------------*/
//       Author : hiyohiyo
//         Mail : hiyohiyo@crystalmark.info
//          Web : http://crystalmark.info/
//      License : The modified BSD license
//
//                                Copyright 2008 hiyohiyo. All rights reserved.
/*---------------------------------------------------------------------------*/

#pragma once

BOOL InstallEventSource();
BOOL WriteEventLog(DWORD eventId, WORD eventType, PTCHAR source, CString message);
BOOL UninstallEventSource();