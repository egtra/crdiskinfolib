#pragma once

#define _SECURE_ATL 1

#define WINVER 0x0501
#define _WIN32_WINNT 0x0501

#define WIN32_LEAN_AND_MEAN

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS

#include <windows.h>
#include <shellapi.h>

#include <comdef.h>

#include <atlstr.h>
#include <atlcoll.h>

#pragma warning(disable : 4996)
