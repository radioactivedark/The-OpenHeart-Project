#pragma once

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <stdio.h>
#include <tchar.h>
#include <Windows.h>
#include <Iphlpapi.h>
#include <string.h>

// Fetches the MAC address and prints it
const string getMACaddress()
{
    char output[32] = "";
	IP_ADAPTER_INFO AdapterInfo[16];			// Allocate information for up to 16 NICs
	DWORD dwBufLen = sizeof(AdapterInfo);		// Save the memory size of buffer

	DWORD dwStatus = GetAdaptersInfo(			// Call GetAdapterInfo
		AdapterInfo,							// [out] buffer to receive data
		&dwBufLen);								// [in] size of receive data buffer
	if(dwStatus == ERROR_SUCCESS)               // Verify return value is valid, no buffer overflow
	{
        PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo;// Contains pointer to current adapter info
        while(pAdapterInfo)
        {
            unsigned char *MACData = pAdapterInfo->Address;
            if(!MACData[0]&&!MACData[1]&&!MACData[2]&&!MACData[3]&&!MACData[4]&&!MACData[5])
            {
                pAdapterInfo = pAdapterInfo->Next;
                continue;
            }
            sprintf(output,"%02X-%02X-%02X-%02X-%02X-%02X",
                MACData[0], MACData[1], MACData[2], MACData[3], MACData[4], MACData[5]);
            break;
        }
	}
	return string(output);
}
