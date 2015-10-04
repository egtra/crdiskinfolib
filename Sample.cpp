// CrDiskInfoLib: Sample.cpp
// Copyright (c) 2014, Egtra
//
// This code is released under the MIT license.
// See LICENSE.txt

#include "stdafx.h"
#include <iostream>
#include <locale>
#include <algorithm>
#include <tchar.h>
#include "DiskInfoLib.h"

void PrintDiskInfo(const DiskInfo& diskInfo)
{
	std::wcout << "----------------\n";
	std::wcout << L"SerialNumber: " << diskInfo.SerialNumber << L'\n';
	std::wcout << L"FirmwareRev: " << diskInfo.FirmwareRev << L'\n';
	std::wcout << L"Model: " << diskInfo.Model << L'\n';
	std::wcout << L"ModelWmi: " << diskInfo.ModelWmi << L'\n';
	std::wcout << L"ModelSerial: " << diskInfo.ModelSerial << L'\n';
	std::wcout << L"MaxTransferMode: " << diskInfo.MaxTransferMode << L'\n';
	std::wcout << L"CurrentTransferMode: " << diskInfo.CurrentTransferMode << L'\n';
	std::wcout << L"MajorVersion: " << diskInfo.MajorVersion << L'\n';
	std::wcout << L"MinorVersion: " << diskInfo.MinorVersion << L'\n';
	std::wcout << L"Interface: " << diskInfo.Interface << L'\n';
	std::wcout << L"Enclosure: " << diskInfo.Enclosure << L'\n';
	std::wcout << L"SsdVendorString: " << diskInfo.SsdVendorString << L'\n';
	std::wcout << L"DeviceNominalFormFactor: " << diskInfo.DeviceNominalFormFactor << L'\n';
	std::wcout << L"PnpDeviceId: " << diskInfo.PnpDeviceId << L'\n';
}

int _tmain()
{
	std::wcout.imbue(std::locale(""));

	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	if (FAILED(hr))
	{
		std::cerr << "CoInitializeEx failed: " << std::hex << hr << std::endl;
		return 1;
	}

	hr = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL);
	if (FAILED(hr))
	{
		std::cerr << "CoInitializeSecurity failed: " << std::hex << hr << std::endl;
		return 1;
	}

	// for WMI error
	SetErrorMode(SEM_FAILCRITICALERRORS);

	try
	{
		std::vector<DiskInfo> diskInfo = CrDiskInfoLib::GetDiskInfo();
		std::for_each(diskInfo.begin(), diskInfo.end(), PrintDiskInfo);
	}
	catch (const std::exception& e)
	{
		std::cerr << "std::exception: " << e.what() << std::endl;
		return 1;
	}
	catch (...)
	{
		std::cerr << "Unknown exception" << std::endl;
		return 1;
	}

	CoUninitialize();
}
