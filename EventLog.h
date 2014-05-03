/*---------------------------------------------------------------------------*/
//       Author : hiyohiyo
//         Mail : hiyohiyo@crystalmark.info
//          Web : http://crystalmark.info/
//      License : Simplified BSD license
/*---------------------------------------------------------------------------*/

#pragma once

BOOL InstallEventSource();
BOOL WriteEventLog(DWORD eventId, WORD eventType, PTCHAR source, CString message);
BOOL UninstallEventSource();