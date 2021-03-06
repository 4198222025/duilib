﻿#include <iostream> 
#include <string>
#include <string.h>
#include <winsock2.h> // include must before window.h
#include <iphlpapi.h>
#include <windows.h>  

#include "GetWindowsInfo.h"

#pragma comment(lib, "iphlpapi.lib")

#pragma warning(disable: 4996) // avoid GetVersionEx to be warned

// ***** global macros ***** //
static const int kMaxInfoBuffer = 256;
#define  GBYTES  1073741824  
#define  MBYTES  1048576  
#define  KBYTES  1024  
#define  DKBYTES 1024.0  

//-------------------------------------------------------------------------

// 函数    : GetNtVersionNumbers

// 功能    : 调用RtlGetNtVersionNumbers获取系统版本信息

// 返回值  : BOOL

// 参数    : DWORD& dwMajorVer 主版本

// 参数    : DWORD& dwMinorVer 次版本

// 参数    : DWORD& dwBuildNumber build号

// 附注    :

//-------------------------------------------------------------------------

BOOL GetNtVersionNumbers(DWORD&dwMajorVer, DWORD& dwMinorVer, DWORD& dwBuildNumber)

{

	BOOL bRet = FALSE;

	HMODULE hModNtdll = NULL;

	if (hModNtdll = ::LoadLibraryW(L"ntdll.dll"))

	{

		typedef void (WINAPI *pfRTLGETNTVERSIONNUMBERS)(DWORD*, DWORD*, DWORD*);

		pfRTLGETNTVERSIONNUMBERS pfRtlGetNtVersionNumbers;

		pfRtlGetNtVersionNumbers = (pfRTLGETNTVERSIONNUMBERS)::GetProcAddress(hModNtdll, "RtlGetNtVersionNumbers");

		if (pfRtlGetNtVersionNumbers)

		{

			pfRtlGetNtVersionNumbers(&dwMajorVer, &dwMinorVer, &dwBuildNumber);

			dwBuildNumber &= 0x0ffff;

			bRet = TRUE;

		}




		::FreeLibrary(hModNtdll);

		hModNtdll = NULL;

	}




	return bRet;

}

/*
typedef struct _OSVERSIONINFOA {
	DWORD dwOSVersionInfoSize;
	DWORD dwMajorVersion;  // 主版本号
	DWORD dwMinorVersion;  // 副版本号
	DWORD dwBuildNumber;   // build number
	DWORD dwPlatformId;    // 平台的ID VER_PLATFORM_WIN32_NT
	CHAR   szCSDVersion[128];     // Maintenance string for PSS usage
} OSVERSIONINFOA, *POSVERSIONINFOA, *LPOSVERSIONINFOA;
*/
/*
现在为什么不行了

但是从Windows8.1出来之后，GetVersionExW这个API被微软明文给废弃了，这个坑下得可够大的（参考[1]）。也就是说从Windows8.1开始之后（包括Windows10），这个API常规情况下就是返回6.2了。

“In Windows 8.1, the GetVersion(Ex)APIs have been deprecated.That means that while you can still call the APIs, if your app does not specifically target Windows 8.1, you will getWindows 8 versioning(6.2.0.0).”
*/

// ---- get os info ---- //
void getOsInfo()
{
	// get os name according to version number
	OSVERSIONINFO osver = { sizeof(OSVERSIONINFO) };
	GetVersionEx(&osver);
	std::string os_name;
	if (osver.dwMajorVersion == 5 && osver.dwMinorVersion == 0)
		os_name = "Windows 2000";
	else if (osver.dwMajorVersion == 5 && osver.dwMinorVersion == 1)
		os_name = "Windows XP";
	else if (osver.dwMajorVersion == 6 && osver.dwMinorVersion == 0)
		os_name = "Windows 2003";
	else if (osver.dwMajorVersion == 5 && osver.dwMinorVersion == 2)
		os_name = "windows vista";
	else if (osver.dwMajorVersion == 6 && osver.dwMinorVersion == 1)
		os_name = "windows 7";
	else if (osver.dwMajorVersion == 6 && osver.dwMinorVersion == 2)
		os_name = "windows 10";

	std::cout << "os name: " << os_name << std::endl;
	std::cout << "os version: " << osver.dwMajorVersion << '.' << osver.dwMinorVersion << std::endl;

}

// ---- get cpu info ---- //

#ifdef _WIN64

// method 2: usde winapi, works for x86 and x64
#include <intrin.h>
void getCpuInfo()
{
	int cpuInfo[4] = { -1 };
	char cpu_manufacture[32] = { 0 };
	char cpu_type[32] = { 0 };
	char cpu_freq[32] = { 0 };

	__cpuid(cpuInfo, 0x80000002);
	memcpy(cpu_manufacture, cpuInfo, sizeof(cpuInfo));

	__cpuid(cpuInfo, 0x80000003);
	memcpy(cpu_type, cpuInfo, sizeof(cpuInfo));

	__cpuid(cpuInfo, 0x80000004);
	memcpy(cpu_freq, cpuInfo, sizeof(cpuInfo));

	std::cout << "CPU manufacture: " << cpu_manufacture << std::endl;
	std::cout << "CPU type: " << cpu_type << std::endl;
	std::cout << "CPU main frequency: " << cpu_freq << std::endl;
}

#else

// mothed 1: this kind asm embedded in code only works in x86 build
// save 4 register variables
DWORD deax;
DWORD debx;
DWORD decx;
DWORD dedx;

// init cpu in assembly language
void initCpu(DWORD veax)
{
	__asm
	{
		mov eax, veax
			cpuid
			mov deax, eax
			mov debx, ebx
			mov decx, ecx
			mov dedx, edx
	}
}

long getCpuFreq()
{
	int start, over;
	_asm
	{
		RDTSC
			mov start, eax
	}
	Sleep(50);
	_asm
	{
		RDTSC
			mov over, eax
	}
	return (over - start) / 50000;
}

std::string getManufactureID()
{
	char manuID[25];
	memset(manuID, 0, sizeof(manuID));

	initCpu(0);
	memcpy(manuID + 0, &debx, 4); // copy to array
	memcpy(manuID + 4, &dedx, 4);
	memcpy(manuID + 8, &decx, 4);

	return manuID;
}

std::string getCpuType()
{
	const DWORD id = 0x80000002; // start 0x80000002 end to 0x80000004
	char cpuType[49];
	memset(cpuType, 0, sizeof(cpuType));

	for (DWORD t = 0; t < 3; t++)
	{
		initCpu(id + t);

		memcpy(cpuType + 16 * t + 0, &deax, 4);
		memcpy(cpuType + 16 * t + 4, &debx, 4);
		memcpy(cpuType + 16 * t + 8, &decx, 4);
		memcpy(cpuType + 16 * t + 12, &dedx, 4);
	}

	return cpuType;
}

void getCpuInfo()
{
	std::cout << "CPU manufacture: " << getManufactureID() << std::endl;
	std::cout << "CPU type: " << getCpuType() << std::endl;
	std::cout << "CPU main frequency: " << getCpuFreq() << "MHz" << std::endl;
}

#endif

// ---- get memory info ---- //
void getMemoryInfo()
{
	std::string memory_info;
	MEMORYSTATUSEX statusex;
	statusex.dwLength = sizeof(statusex);
	if (GlobalMemoryStatusEx(&statusex))
	{
		unsigned long long total = 0, remain_total = 0, avl = 0, remain_avl = 0;
		double decimal_total = 0, decimal_avl = 0;
		remain_total = statusex.ullTotalPhys % GBYTES;
		total = statusex.ullTotalPhys / GBYTES;
		avl = statusex.ullAvailPhys / GBYTES;
		remain_avl = statusex.ullAvailPhys % GBYTES;
		if (remain_total > 0)
			decimal_total = (remain_total / MBYTES) / DKBYTES;
		if (remain_avl > 0)
			decimal_avl = (remain_avl / MBYTES) / DKBYTES;

		decimal_total += (double)total;
		decimal_avl += (double)avl;
		char  buffer[kMaxInfoBuffer];
		sprintf_s(buffer, kMaxInfoBuffer, "total %.2f GB (%.2f GB available)", decimal_total, decimal_avl);
		memory_info.append(buffer);
	}
	std::cout << memory_info << std::endl;
}

// ---- get harddisk info ---- //
std::string execCmd(const char *cmd)
{
	char buffer[128] = { 0 };
	std::string result;
	FILE *pipe = _popen(cmd, "r");
	if (!pipe) throw std::runtime_error("_popen() failed!");
	while (!feof(pipe))
	{
		if (fgets(buffer, 128, pipe) != NULL)
			result += buffer;
	}
	_pclose(pipe);

	return result;
}

void getHardDiskInfo()
{
	std::string hd_seiral = execCmd("wmic path win32_physicalmedia get SerialNumber");
	std::cout << "HardDisk Serial Number: " << hd_seiral << std::endl;
}

// ---- get network info ---- //
void getNetworkInfo()
{
	// PIP_ADAPTER_INFO struct contains network information
	PIP_ADAPTER_INFO pIpAdapterInfo = new IP_ADAPTER_INFO();
	unsigned long adapter_size = sizeof(IP_ADAPTER_INFO);
	int ret = GetAdaptersInfo(pIpAdapterInfo, &adapter_size);

	if (ret == ERROR_BUFFER_OVERFLOW)
	{
		// overflow, use the output size to recreate the handler
		delete pIpAdapterInfo;
		pIpAdapterInfo = (PIP_ADAPTER_INFO)new BYTE[adapter_size];
		ret = GetAdaptersInfo(pIpAdapterInfo, &adapter_size);
	}

	if (ret == ERROR_SUCCESS)
	{
		int card_index = 0;

		// may have many cards, it saved in linklist
		while (pIpAdapterInfo)
		{
			std::cout << "---- " << "NetworkCard " << ++card_index << " ----" << std::endl;

			std::cout << "Network Card Name: " << pIpAdapterInfo->AdapterName << std::endl;
			std::cout << "Network Card Description: " << pIpAdapterInfo->Description << std::endl;

			// get IP, one card may have many IPs
			PIP_ADDR_STRING pIpAddr = &(pIpAdapterInfo->IpAddressList);
			while (pIpAddr)
			{
				char local_ip[128] = { 0 };
				strcpy(local_ip, pIpAddr->IpAddress.String);
				std::cout << "Local IP: " << local_ip << std::endl;

				pIpAddr = pIpAddr->Next;
			}

			char local_mac[128] = { 0 };
			int char_index = 0;
			for (int i = 0; i < pIpAdapterInfo->AddressLength; i++)
			{
				char temp_str[10] = { 0 };
				sprintf(temp_str, "%02X-", pIpAdapterInfo->Address[i]); // X for uppercase, x for lowercase
				strcpy(local_mac + char_index, temp_str);
				char_index += 3;
			}
			local_mac[17] = '\0'; // remove tail '-'

			std::cout << "Local Mac: " << local_mac << std::endl;

			// here just need the first card info
			break;
			// iterate next
			//pIpAdapterInfo = pIpAdapterInfo->Next;
		}
	}

	if (pIpAdapterInfo)
		delete pIpAdapterInfo;
}
/*
int main(int argc, char *argv[])
{
	std::cout << "=== os information ===" << std::endl;
	getOsInfo();

	std::cout << "=== cpu infomation ===" << std::endl;
	getCpuInfo();

	std::cout << "=== memory information ===" << std::endl;
	getMemoryInfo();

	std::cout << "=== harddisk information ===" << std::endl;
	getHardDiskInfo();

	std::cout << "=== network information ===" << std::endl;
	getNetworkInfo();

	system("pause");
	return 0;
}
*/

typedef void (WINAPI *PGNSI)(LPSYSTEM_INFO);
typedef void (WINAPI *PGPI)(DWORD, DWORD, DWORD, DWORD, DWORD*);


BOOL GetOperatingSystemName(std::string& strWindowsName, DWORD& dwProcessorArchitecture)
{
	OSVERSIONINFOEX osvi;
	SYSTEM_INFO si;
	PGNSI pGNSI;
	PGPI pGPI;
	BOOL bOsVersionInfoEx;
	DWORD dwType;

	ZeroMemory(&si, sizeof(SYSTEM_INFO));
	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));

	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

	if (!(bOsVersionInfoEx = GetVersionEx((OSVERSIONINFO *)&osvi)))
		return FALSE;

	if (!GetNtVersionNumbers(osvi.dwMajorVersion, osvi.dwMinorVersion, osvi.dwBuildNumber))
		return FALSE;
	;

	// Call GetNativeSystemInfo if supported or GetSystemInfo otherwise.

	pGNSI = (PGNSI)GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "GetNativeSystemInfo");
	if (NULL != pGNSI)
	{
		pGNSI(&si);
	}
	else
	{
		GetSystemInfo(&si);
	}

	if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 || si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64 || si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_ALPHA64)
	{
		dwProcessorArchitecture = 64;
		//StringCchCat(pszOS, BUFSIZE, TEXT(", 64-bit"));
	}
	else if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL)
	{
		dwProcessorArchitecture = 32;
		//StringCchCat(pszOS, BUFSIZE, TEXT(", 32-bit"));
	}
		


	if (VER_PLATFORM_WIN32_NT == osvi.dwPlatformId && osvi.dwMajorVersion > 4)
	{
		strWindowsName += "Microsoft ";
		//StringCchCopy(pszOS, BUFSIZE, TEXT("Microsoft "));

		// Test for the specific product.

		//write log
		/*FILE *fp;
		char fname[32];

		fp = fopen("C:/cczsllog.txt", "w+");
		fprintf(fp, "%s", ";osvi.dwMajorVersion:");
		fprintf(fp, "%d\n", osvi.dwMajorVersion);
		fprintf(fp, "%s", ";osvi.dwMinorVersion:");
		fprintf(fp, "%d\n", osvi.dwMinorVersion);
		fclose(fp);*/


		if (osvi.dwMajorVersion == 6)
		{
			if (osvi.dwMinorVersion == 0)
			{
				if (osvi.wProductType == VER_NT_WORKSTATION)
					strWindowsName += "Windows Vista ";
				else strWindowsName += "Windows Server 2008 "; 
			}
			else if (osvi.dwMinorVersion == 1)
			{
				if (osvi.wProductType == VER_NT_WORKSTATION)
					strWindowsName += "Windows 7 "; //StringCchCat(pszOS, BUFSIZE, TEXT("Windows 7 "));
				else strWindowsName += "Windows Server 2008 R2 "; //StringCchCat(pszOS, BUFSIZE, TEXT("Windows Server 2008 R2 "));
			}
			else if (osvi.dwMinorVersion == 2)
			{
				if (osvi.wProductType == VER_NT_WORKSTATION)
					strWindowsName += "Windows 8 "; //StringCchCat(pszOS, BUFSIZE, TEXT("Windows 8 "));
				else strWindowsName += "Windows Server 2012 "; //StringCchCat(pszOS, BUFSIZE, TEXT("Windows Server 2012 "));
			}
			else if (osvi.dwMinorVersion == 3)
			{
				if (osvi.wProductType == VER_NT_WORKSTATION)
					strWindowsName += "Windows 8.1 "; //StringCchCat(pszOS, BUFSIZE, TEXT("Windows 8.1 "));
				else strWindowsName += "Windows Server 2012 R2 "; //StringCchCat(pszOS, BUFSIZE, TEXT("Windows Server 2012 R2 "));
			}

			pGPI = (PGPI)GetProcAddress(
				GetModuleHandle(TEXT("kernel32.dll")),
				"GetProductInfo");

			pGPI(osvi.dwMajorVersion, osvi.dwMinorVersion, 0, 0, &dwType);

			switch (dwType)
			{
			case PRODUCT_ULTIMATE:
				strWindowsName += "Ultimate"; //StringCchCat(pszOS, BUFSIZE, TEXT("Ultimate"));
				break;
				//case PRODUCT_PROFESSIONAL:
			case 0x00000030:
				strWindowsName += "Professional"; //StringCchCat(pszOS, BUFSIZE, TEXT("Professional"));
				break;
			case PRODUCT_HOME_PREMIUM:
				strWindowsName += "Home Premium"; //StringCchCat(pszOS, BUFSIZE, TEXT("Home Premium"));
				break;
			case PRODUCT_HOME_BASIC:
				strWindowsName += "Home Basic"; //StringCchCat(pszOS, BUFSIZE, TEXT("Home Basic"));
				break;
			case PRODUCT_ENTERPRISE:
				strWindowsName += "Enterprise"; //StringCchCat(pszOS, BUFSIZE, TEXT("Enterprise"));
				break;
			case PRODUCT_BUSINESS:
				strWindowsName += "Business"; //StringCchCat(pszOS, BUFSIZE, TEXT("Business"));
				break;
			case PRODUCT_STARTER:
				strWindowsName += "Starter"; //StringCchCat(pszOS, BUFSIZE, TEXT("Starter"));
				break;
			case PRODUCT_CLUSTER_SERVER:
				strWindowsName += "Cluster Server"; //StringCchCat(pszOS, BUFSIZE, TEXT("Cluster Server"));
				break;
			case PRODUCT_DATACENTER_SERVER:
				strWindowsName += "Datacenter"; //StringCchCat(pszOS, BUFSIZE, TEXT("Datacenter"));
				break;
			case PRODUCT_DATACENTER_SERVER_CORE:
				strWindowsName += "Datacenter Edition (core installation)"; //StringCchCat(pszOS, BUFSIZE, TEXT("Datacenter Edition (core installation)"));
				break;
			case PRODUCT_ENTERPRISE_SERVER:
				strWindowsName += "Enterprise"; //StringCchCat(pszOS, BUFSIZE, TEXT("Enterprise"));
				break;
			case PRODUCT_ENTERPRISE_SERVER_CORE:
				strWindowsName += "Enterprise Edition (core installation)"; //StringCchCat(pszOS, BUFSIZE, TEXT("Enterprise Edition (core installation)"));
				break;
			case PRODUCT_ENTERPRISE_SERVER_IA64:
				strWindowsName += "Enterprise Edition for Itanium-based Systems"; //StringCchCat(pszOS, BUFSIZE, TEXT("Enterprise Edition for Itanium-based Systems"));
				break;
			case PRODUCT_SMALLBUSINESS_SERVER:
				strWindowsName += "Small Business Server"; //StringCchCat(pszOS, BUFSIZE, TEXT("Small Business Server"));
				break;
			case PRODUCT_SMALLBUSINESS_SERVER_PREMIUM:
				strWindowsName += "Small Business Server Premium Edition"; //StringCchCat(pszOS, BUFSIZE, TEXT("Small Business Server Premium Edition"));
				break;
			case PRODUCT_STANDARD_SERVER:
				strWindowsName += "Standard"; //StringCchCat(pszOS, BUFSIZE, TEXT("Standard"));
				break;
			case PRODUCT_STANDARD_SERVER_CORE:
				strWindowsName += "Standard Edition (core installation)"; //StringCchCat(pszOS, BUFSIZE, TEXT("Standard Edition (core installation)"));
				break;
			case PRODUCT_WEB_SERVER:
				strWindowsName += "Web Server"; //StringCchCat(pszOS, BUFSIZE, TEXT("Web Server"));
				break;
			}
		}

		if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2)
		{
			if (GetSystemMetrics(SM_SERVERR2))
				strWindowsName += "Windows Server 2003 R2, "; //StringCchCat(pszOS, BUFSIZE, TEXT("Windows Server 2003 R2, "));
			else if (osvi.wSuiteMask & VER_SUITE_STORAGE_SERVER)
				strWindowsName += "Windows Storage Server 2003"; //StringCchCat(pszOS, BUFSIZE, TEXT("Windows Storage Server 2003"));
			//else if ( osvi.wSuiteMask & VER_SUITE_WH_SERVER )
			//   StringCchCat(pszOS, BUFSIZE, TEXT( "Windows Home Server"));
			else if (osvi.wProductType == VER_NT_WORKSTATION &&
				si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
			{
				strWindowsName += "Windows XP Professional x64 Edition"; //StringCchCat(pszOS, BUFSIZE, TEXT("Windows XP Professional x64 Edition"));
			}
			else strWindowsName += "Windows Server 2003, "; //StringCchCat(pszOS, BUFSIZE, TEXT("Windows Server 2003, "));

			// Test for the server type.
			if (osvi.wProductType != VER_NT_WORKSTATION)
			{
				if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64)
				{
					if (osvi.wSuiteMask & VER_SUITE_DATACENTER)
						strWindowsName += "Datacenter Edition for Itanium-based Systems"; //StringCchCat(pszOS, BUFSIZE, TEXT("Datacenter Edition for Itanium-based Systems"));
					else if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE)
						strWindowsName += "Enterprise Edition for Itanium-based Systems"; //StringCchCat(pszOS, BUFSIZE, TEXT("Enterprise Edition for Itanium-based Systems"));
				}

				else if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
				{
					if (osvi.wSuiteMask & VER_SUITE_DATACENTER)
						strWindowsName += "Datacenter x64 Edition"; //StringCchCat(pszOS, BUFSIZE, TEXT("Datacenter x64 Edition"));
					else if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE)
						strWindowsName += "Enterprise x64 Edition"; //StringCchCat(pszOS, BUFSIZE, TEXT("Enterprise x64 Edition"));
					else strWindowsName += "Standard x64 Edition"; //StringCchCat(pszOS, BUFSIZE, TEXT("Standard x64 Edition"));
				}

				else
				{
					if (osvi.wSuiteMask & VER_SUITE_COMPUTE_SERVER)
						strWindowsName += "Compute Cluster Edition"; //StringCchCat(pszOS, BUFSIZE, TEXT("Compute Cluster Edition"));
					else if (osvi.wSuiteMask & VER_SUITE_DATACENTER)
						strWindowsName += "Datacenter Edition"; //StringCchCat(pszOS, BUFSIZE, TEXT("Datacenter Edition"));
					else if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE)
						strWindowsName += "Enterprise Edition"; //StringCchCat(pszOS, BUFSIZE, TEXT("Enterprise Edition"));
					else if (osvi.wSuiteMask & VER_SUITE_BLADE)
						strWindowsName += "Web Edition"; //StringCchCat(pszOS, BUFSIZE, TEXT("Web Edition"));
					else strWindowsName += "Standard Edition"; //StringCchCat(pszOS, BUFSIZE, TEXT("Standard Edition"));
				}
			}
		}

		if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1)
		{
			strWindowsName += "Windows XP "; //StringCchCat(pszOS, BUFSIZE, TEXT("Windows XP "));
			if (osvi.wSuiteMask & VER_SUITE_PERSONAL)
				strWindowsName += "Home Edition"; //StringCchCat(pszOS, BUFSIZE, TEXT("Home Edition"));
			else strWindowsName += "Professional"; //StringCchCat(pszOS, BUFSIZE, TEXT("Professional"));
		}

		if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0)
		{
			strWindowsName += "Windows 2000 "; //StringCchCat(pszOS, BUFSIZE, TEXT("Windows 2000 "));

			if (osvi.wProductType == VER_NT_WORKSTATION)
			{
				strWindowsName += "Professional"; //StringCchCat(pszOS, BUFSIZE, TEXT("Professional"));
			}
			else
			{
				if (osvi.wSuiteMask & VER_SUITE_DATACENTER)
					strWindowsName += "Datacenter Server"; //StringCchCat(pszOS, BUFSIZE, TEXT("Datacenter Server"));
				else if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE)
					strWindowsName += "Advanced Server"; //StringCchCat(pszOS, BUFSIZE, TEXT("Advanced Server"));
				else strWindowsName += "Server"; //StringCchCat(pszOS, BUFSIZE, TEXT("Server"));
			}
		}

		/*if ( osvi.dwMajorVersion >= 6 )
		{
		if ( si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64 )
		StringCchCat(pszOS, BUFSIZE, TEXT( ", 64-bit" ));
		else if (si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_INTEL )
		StringCchCat(pszOS, BUFSIZE, TEXT(", 32-bit"));
		}*/

		if (osvi.dwMajorVersion == 10)
		{
			if (osvi.dwMinorVersion == 0)
			{
				if (osvi.wProductType == VER_NT_WORKSTATION)
				{
					strWindowsName += "Windows 10 "; //StringCchCat(pszOS, BUFSIZE, TEXT("Windows 10 "));

					pGPI = (PGPI)GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "GetProductInfo");

					pGPI(osvi.dwMajorVersion, osvi.dwMinorVersion, 0, 0, &dwType);

					switch (dwType)
					{
					case PRODUCT_ULTIMATE:
						strWindowsName += "Ultimate"; //StringCchCat(pszOS, BUFSIZE, TEXT("Ultimate"));
						break;
						//case PRODUCT_PROFESSIONAL:
					case 0x00000030:
						strWindowsName += "Professional"; //StringCchCat(pszOS, BUFSIZE, TEXT("Professional"));
						break;
					case PRODUCT_HOME_PREMIUM:
						strWindowsName += "Home Premium"; //StringCchCat(pszOS, BUFSIZE, TEXT("Home Premium"));
						break;
					case PRODUCT_HOME_BASIC:
						strWindowsName += "Home Basic"; //StringCchCat(pszOS, BUFSIZE, TEXT("Home Basic"));
						break;
					case PRODUCT_ENTERPRISE:
						strWindowsName += "Enterprise"; //StringCchCat(pszOS, BUFSIZE, TEXT("Enterprise"));
						break;
					case PRODUCT_BUSINESS:
						strWindowsName += "Business"; //StringCchCat(pszOS, BUFSIZE, TEXT("Business"));
						break;
					case PRODUCT_STARTER:
						strWindowsName += "Starter"; //StringCchCat(pszOS, BUFSIZE, TEXT("Starter"));
						break;
					case PRODUCT_CLUSTER_SERVER:
						strWindowsName += "Cluster Server"; //StringCchCat(pszOS, BUFSIZE, TEXT("Cluster Server"));
						break;
					case PRODUCT_DATACENTER_SERVER:
						strWindowsName += "Datacenter"; //StringCchCat(pszOS, BUFSIZE, TEXT("Datacenter"));
						break;
					case PRODUCT_DATACENTER_SERVER_CORE:
						strWindowsName += "Datacenter Edition (core installation)"; //StringCchCat(pszOS, BUFSIZE, TEXT("Datacenter Edition (core installation)"));
						break;
					case PRODUCT_ENTERPRISE_SERVER:
						strWindowsName += "Enterprise"; //StringCchCat(pszOS, BUFSIZE, TEXT("Enterprise"));
						break;
					case PRODUCT_ENTERPRISE_SERVER_CORE:
						strWindowsName += "Enterprise Edition (core installation)"; //StringCchCat(pszOS, BUFSIZE, TEXT("Enterprise Edition (core installation)"));
						break;
					case PRODUCT_ENTERPRISE_SERVER_IA64:
						strWindowsName += "Enterprise Edition for Itanium-based Systems"; //StringCchCat(pszOS, BUFSIZE, TEXT("Enterprise Edition for Itanium-based Systems"));
						break;
					case PRODUCT_SMALLBUSINESS_SERVER:
						strWindowsName += "Small Business Server"; //StringCchCat(pszOS, BUFSIZE, TEXT("Small Business Server"));
						break;
					case PRODUCT_SMALLBUSINESS_SERVER_PREMIUM:
						strWindowsName += "Small Business Server Premium Edition"; //StringCchCat(pszOS, BUFSIZE, TEXT("Small Business Server Premium Edition"));
						break;
					case PRODUCT_STANDARD_SERVER:
						strWindowsName += "Standard"; //StringCchCat(pszOS, BUFSIZE, TEXT("Standard"));
						break;
					case PRODUCT_STANDARD_SERVER_CORE:
						strWindowsName += "Standard Edition (core installation)"; //StringCchCat(pszOS, BUFSIZE, TEXT("Standard Edition (core installation)"));
						break;
					case PRODUCT_WEB_SERVER:
						strWindowsName += "Web Server"; //StringCchCat(pszOS, BUFSIZE, TEXT("Web Server"));
						break;
					}
				}
				else {
					strWindowsName += "Windows Server 2016 Technical Preview "; //StringCchCat(pszOS, BUFSIZE, TEXT("Windows Server 2016 Technical Preview "));
				}
			}
		}

		return TRUE;
	}

	else
	{
		return FALSE;
	}
}

// get the service pack name of local machine
BOOL GetOperationSystemServicePackName(LPTSTR pszSP)
{
	// Include service pack (if any) and build number.
	OSVERSIONINFOEX osvi;
	BOOL bOsVersionInfoEx;

	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));

	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

	if (!(bOsVersionInfoEx = GetVersionEx((OSVERSIONINFO *)&osvi)))
		return FALSE;
	/*
	if (_tcslen(osvi.szCSDVersion) > 0)
	{
		StringCchCat(pszSP, BUFSIZE, osvi.szCSDVersion);
		return TRUE;
	}
	else
	{
		_tcscpy(pszSP, _T("(null)"));
		return TRUE;
	}

	*/

	return FALSE;
}