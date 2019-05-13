#include "stdafx.h"
#include <direct.h> //_mkdir������ͷ�ļ�
#include <io.h>     //_access������ͷ�ļ�
#include <vector>
#include <string>
#include <algorithm>    // transform

#include <codecvt>

#include "SysUtil.h"

#include "VendorInfo.h"
#include "UploadFileInfo.h"

std::vector<std::string> GetSystemDrives(){

	TCHAR  drives[128];				//�洢��������������
	char* pDrive;				//������ָ��
	std::vector<std::string> strArray;

	//ȡ��ϵͳ�ĵ�һ���߼�������
	if (!GetLogicalDriveStrings(sizeof(drives), drives))
	{
		printf("��ȡ������ʧ��\r\n");
		return strArray;
	}
	pDrive=drives; //ָ���һ���߼�������
	//���������ַ������б����
	while (*pDrive)
	{
		//�����������Ƽ����б���
		strArray.push_back(pDrive);

		//ָ����һ����������ʶ��
		pDrive += strlen(pDrive) + 1;
	}

	for (int i=0; i < strArray.size(); ++i)
	{
		printf("%ls\r\n", strArray[i].c_str());
	}

	return strArray;
}

std::string GetVolumeName(std::string driveName)
{
	CHAR szVolumeName[128]={ '\0' };

	RtlZeroMemory(szVolumeName, 128);
	QueryDosDevice(driveName.c_str(), szVolumeName, 128);

	std::string strVolumeName(szVolumeName);
	return strVolumeName;
}
std::string GetUserId()
{
	char userName[260]="";
	char sid[260]="";
	DWORD nameSize=sizeof(userName);
	GetUserName(userName, &nameSize);


	char userSID[260]="";
	char userDomain[260]="";
	DWORD sidSize=sizeof(userSID);
	DWORD domainSize=sizeof(userDomain);


	SID_NAME_USE snu;
	LookupAccountName(NULL,
		userName,
		(PSID)userSID,
		&sidSize,
		userDomain,
		&domainSize,
		&snu);


	PSID_IDENTIFIER_AUTHORITY psia=GetSidIdentifierAuthority(userSID);
	sidSize=sprintf(sid, "S-%lu-", SID_REVISION);
	sidSize += sprintf(sid + strlen(sid), "%-lu", psia->Value[5]);


	int subAuthorities=*GetSidSubAuthorityCount(userSID);


	for (int i=0; i < subAuthorities; i++)
	{
		sidSize += sprintf(sid + sidSize, "-%lu", *GetSidSubAuthority(userSID, i));
	}

	sprintf(userSID, "\\REGISTRY\\USER\\%s", sid);

	std::string strUserId(userSID);
	return strUserId;
}

void CreateDir(const char *dir)
{
	int m = 0, n;
	string str1, str2;

	str1 = dir;	
	str2 = str1.substr(0, 2);
	str1 = str1.substr(3, str1.size());
	
	while (m >= 0)
	{
		m = str1.find('\\');		
		str2 += '\\' + str1.substr(0, m);	
		n = _access(str2.c_str(), 0); //�жϸ�Ŀ¼�Ƿ����
		
		if (n == -1)		
		{		
			_mkdir(str2.c_str());     //����Ŀ¼		
		}
		str1 = str1.substr(m + 1, str1.size());
	
	}
}

bool DirIsValid(string dir)
{
	return ::PathFileExists(dir.c_str());
}



#include <winioctl.h>
#include <Aclapi.h>
#include <tlhelp32.h>


#define EFS_DEVICE_NAME						L"\\Device\\SecMFDock"
#define EFS_LINK_NAME						L"\\DosDevices\\Global\\SecMFDock"


#define EFS_DEVICE							FILE_DEVICE_DISK_FILE_SYSTEM
#define EFS_IO_BASE							0x0800
#define EFS_IOCTL(code,read,write)			\
	CTL_CODE(EFS_DEVICE, \
	(EFS_IO_BASE + code), \
	(METHOD_BUFFERED), \
	(read ? FILE_READ_ACCESS : 0) | \
	(write ? FILE_WRITE_ACCESS : 0))


#define	EFS_SET_MODE					EFS_IOCTL(0x01,TRUE,TRUE)
#define EFS_SET_TRUSTPATH				EFS_IOCTL(0x02,TRUE,TRUE)
#define EFS_CLR_TRUSTPATH				EFS_IOCTL(0x03,TRUE,TRUE)
#define EFS_SET_DOCKNAME				EFS_IOCTL(0x04,TRUE,TRUE)





#define KEY_DEVICE_NAME						L"\\Device\\SecMKDock"
#define KEY_LINK_NAME						L"\\DosDevices\\Global\\SecMKDock"


#define KEY_DEVICE							FILE_DEVICE_UNKNOWN
#define KEY_IO_BASE							0x0800
#define KEY_IOCTL(code,read,write)			\
	CTL_CODE(KEY_DEVICE, \
	(KEY_IO_BASE + code), \
	(METHOD_BUFFERED), \
	(read ? FILE_READ_ACCESS : 0) | \
	(write ? FILE_WRITE_ACCESS : 0))


#define	KEY_SET_MODE					KEY_IOCTL(0x01,TRUE,TRUE)
#define KEY_SET_SID						KEY_IOCTL(0x02,TRUE,TRUE)


HANDLE ghEfsDevice=INVALID_HANDLE_VALUE;
HANDLE ghKeyDevice=INVALID_HANDLE_VALUE;

static LONG CreateRegKeyCustom(HKEY hkey, LPCSTR lpcstr, PHKEY pSubKey)
{

	DWORD dwRes;
	PSID pEveryoneSID=NULL, pAdminSID=NULL;
	PACL pACL=NULL;
	PSECURITY_DESCRIPTOR pSD=NULL;
	EXPLICIT_ACCESS ea[2];
	SID_IDENTIFIER_AUTHORITY SIDAuthWorld =
		SECURITY_WORLD_SID_AUTHORITY;
	SID_IDENTIFIER_AUTHORITY SIDAuthNT=SECURITY_NT_AUTHORITY;
	SECURITY_ATTRIBUTES sa;
	LONG lRes;
	HKEY hkSub=NULL;

	// Create a well-known SID for the Everyone group.
	if (!AllocateAndInitializeSid(&SIDAuthWorld, 1,
		SECURITY_WORLD_RID,
		0, 0, 0, 0, 0, 0, 0,
		&pEveryoneSID))
	{
		_tprintf(_T("AllocateAndInitializeSid Error %u\n"), GetLastError());
		lRes=GetLastError();
		goto Cleanup;
	}

	// Initialize an EXPLICIT_ACCESS structure for an ACE.
	// The ACE will allow Everyone read access to the key.
	ZeroMemory(&ea, 2 * sizeof(EXPLICIT_ACCESS));
	ea[0].grfAccessPermissions=KEY_ALL_ACCESS;
	ea[0].grfAccessMode=SET_ACCESS;
	ea[0].grfInheritance=SUB_CONTAINERS_AND_OBJECTS_INHERIT;
	ea[0].Trustee.TrusteeForm=TRUSTEE_IS_SID;
	ea[0].Trustee.TrusteeType=TRUSTEE_IS_WELL_KNOWN_GROUP;
	ea[0].Trustee.ptstrName=(LPTSTR)pEveryoneSID;

	// Create a SID for the BUILTIN\Administrators group.
	if (!AllocateAndInitializeSid(&SIDAuthNT, 2,
		SECURITY_BUILTIN_DOMAIN_RID,
		DOMAIN_ALIAS_RID_ADMINS,
		0, 0, 0, 0, 0, 0,
		&pAdminSID))
	{
		_tprintf(_T("AllocateAndInitializeSid Error %u\n"), GetLastError());
		lRes=GetLastError();
		goto Cleanup;
	}

	// Initialize an EXPLICIT_ACCESS structure for an ACE.
	// The ACE will allow the Administrators group full access to
	// the key.
	ea[1].grfAccessPermissions=KEY_ALL_ACCESS;
	ea[1].grfAccessMode=SET_ACCESS;
	ea[1].grfInheritance=SUB_CONTAINERS_AND_OBJECTS_INHERIT;
	ea[1].Trustee.TrusteeForm=TRUSTEE_IS_SID;
	ea[1].Trustee.TrusteeType=TRUSTEE_IS_GROUP;
	ea[1].Trustee.ptstrName=(LPTSTR)pAdminSID;

	// Create a new ACL that contains the new ACEs.
	dwRes=SetEntriesInAcl(2, ea, NULL, &pACL);
	if (ERROR_SUCCESS != dwRes)
	{
		_tprintf(_T("SetEntriesInAcl Error %u\n"), GetLastError());
		lRes=GetLastError();
		goto Cleanup;
	}

	// Initialize a security descriptor.  
	pSD=(PSECURITY_DESCRIPTOR)LocalAlloc(LPTR,
		SECURITY_DESCRIPTOR_MIN_LENGTH);
	if (NULL == pSD)
	{
		_tprintf(_T("LocalAlloc Error %u\n"), GetLastError());
		lRes=GetLastError();
		goto Cleanup;
	}

	if (!InitializeSecurityDescriptor(pSD,
		SECURITY_DESCRIPTOR_REVISION))
	{
		_tprintf(_T("InitializeSecurityDescriptor Error %u\n"),
			GetLastError());
		lRes=GetLastError();
		goto Cleanup;
	}

	// Add the ACL to the security descriptor. 
	if (!SetSecurityDescriptorDacl(pSD,
		TRUE,     // bDaclPresent flag   
		pACL,
		FALSE))   // not a default DACL 
	{
		_tprintf(_T("SetSecurityDescriptorDacl Error %u\n"),
			GetLastError());
		lRes=GetLastError();
		goto Cleanup;
	}

	// Initialize a security attributes structure.
	sa.nLength=sizeof (SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor=pSD;
	sa.bInheritHandle=FALSE;

	// Use the security attributes to set the security descriptor 
	// when you create a key.





	if (RegOpenKey(hkey, lpcstr, pSubKey) == ERROR_SUCCESS)
	{
		RegSetKeySecurity(*pSubKey, DACL_SECURITY_INFORMATION, sa.lpSecurityDescriptor);

	}
	else
	{
		MessageBox(NULL, "ERROR\n", "", MB_OK);
	}


Cleanup:

	if (pEveryoneSID)
		FreeSid(pEveryoneSID);
	if (pAdminSID)
		FreeSid(pAdminSID);
	if (pACL)
		LocalFree(pACL);
	if (pSD)
		LocalFree(pSD);
	if (hkSub)
		RegCloseKey(hkSub);

	return lRes;

}

//kill����from����
static BOOL KillProcessFromName(std::string strProcessName)
{
	//�������̿���(TH32CS_SNAPPROCESS��ʾ�������н��̵Ŀ���)
	HANDLE hSnapShot=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	//PROCESSENTRY32���̿��յĽṹ��
	PROCESSENTRY32 pe;

	//ʵ������ʹ��Process32First��ȡ��һ�����յĽ���ǰ�����ĳ�ʼ������
	pe.dwSize=sizeof(PROCESSENTRY32);


	//�����IFЧ��ͬ:
	//if(hProcessSnap == INVALID_HANDLE_VALUE)   ��Ч�ľ��
	if (!Process32First(hSnapShot, &pe))
	{
		return FALSE;
	}

	//���ַ���ת��ΪСд
	//strProcessName.MakeLower();

	std::transform(strProcessName.begin(), strProcessName.end(), strProcessName.begin(), ::tolower);

	//��������Ч  ��һֱ��ȡ��һ�����ѭ����ȥ
	while (Process32Next(hSnapShot, &pe))
	{

		//pe.szExeFile��ȡ��ǰ���̵Ŀ�ִ���ļ�����
		std::string scTmp=pe.szExeFile;


		//����ִ���ļ���������Ӣ����ĸ�޸�ΪСд
		//scTmp.MakeLower();
		std::transform(scTmp.begin(), scTmp.end(), scTmp.begin(), ::tolower);

		//�Ƚϵ�ǰ���̵Ŀ�ִ���ļ����ƺʹ��ݽ������ļ������Ƿ���ͬ
		//��ͬ�Ļ�Compare����0
		if (!scTmp.compare(strProcessName))
		{

			//�ӿ��ս����л�ȡ�ý��̵�PID(������������е�PID)
			DWORD dwProcessID=pe.th32ProcessID;
			HANDLE hProcess=::OpenProcess(PROCESS_TERMINATE, FALSE, dwProcessID);
			::TerminateProcess(hProcess, 0);
			CloseHandle(hProcess);
			return TRUE;
		}
		//scTmp.ReleaseBuffer();
	}
	//strProcessName.ReleaseBuffer();
	return FALSE;
}

/************************************************************************/
/* ��װ����
����һ�������ļ�·��
*/
/************************************************************************/
bool SecLoad(std::string strDrvPath)
{	
		SC_HANDLE hDriver;
		SC_HANDLE hSCM;
		DWORD dwError = 0;
		string strLog;

		//strLog.Format(_T("���������ļ���%s.\r\n"), strDrvPath);
		//CFunc::SendLog(strLog);
		//CString strDrvTitle = strDrvPath.Right(strDrvPath.GetLength() - strDrvPath.ReverseFind('\\') - 1);
		string strDrvTitle = strDrvPath.substr(strDrvPath.rfind('\\')+1);
		strDrvTitle = strDrvTitle.substr(0,strDrvTitle.find('.'));
		//�򿪷��������
		hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
		if (hSCM == NULL)
		{
			//strLog.Format(_T("�򿪷��������ʧ�ܣ����������ļ���%sʧ��.\r\n"), strDrvPath);
			//CFunc::SendLog(strLog);
			return false;
		}
		//Ϊ������������
		hDriver = CreateService(hSCM, strDrvTitle.c_str(), strDrvTitle.c_str(), SERVICE_ALL_ACCESS, SERVICE_KERNEL_DRIVER,
			SERVICE_DEMAND_START, SERVICE_ERROR_IGNORE, strDrvPath.c_str(), NULL, NULL, NULL, NULL, NULL);
		if (hDriver == NULL)
		{
			dwError = GetLastError();
			if (dwError != ERROR_SERVICE_EXISTS)
			{
				CloseServiceHandle(hSCM);
				//strLog.Format(_T("���������ļ���%sʧ�ܣ�������:%u.\r\n"), strDrvPath, dwError);
				//CFunc::SendLog(strLog);
				return false;
			}
			//��������Ѿ����ڣ��ʹ�֮
			hDriver = OpenService(hSCM, strDrvTitle.c_str(), SERVICE_ALL_ACCESS);
			if (hDriver == NULL)
			{
				CloseServiceHandle(hSCM);
				//strLog.Format(_T("���������ļ���%sʧ�ܣ������Ѵ��ڣ�������ʧ�ܣ�������:%u.\r\n"), strDrvPath, dwError);
				//CFunc::SendLog(strLog);
				return false;
			}
			else
			{
				//CFunc::SendLog(_T("���������Ѿ����ڣ������Ѿ���װ�������������´򿪳ɹ�.\r\n"));
			}
		}

		dwError = StartService(hDriver, 0, NULL);
		if (!dwError)
		{
			dwError = GetLastError();
			if (dwError != ERROR_IO_PENDING && dwError != ERROR_SERVICE_ALREADY_RUNNING)
			{
				CloseServiceHandle(hDriver);
				CloseServiceHandle(hSCM);
				//strLog.Format(_T("���������ļ���%sʧ�ܣ���������ʧ�ܣ�������:%u.\r\n"), strDrvPath, dwError);
				//CFunc::SendLog(strLog);
				return false;
			}
			else
			{
				//CFunc::SendLog(_T("���������Ѿ������������ٴ�����.\r\n"));
			}
		}
		return true;
}

void SecInit(std::string strDriveName, std::string strVolumeName, std::string strSidName)
{
	BOOLEAN bRet=FALSE;
	DWORD dwRet=0;
	WCHAR szDockName[260];
	ULONG nLen;

	ghEfsDevice=CreateFile("\\\\.\\SecMFDock",
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);


	RtlZeroMemory(szDockName, sizeof(szDockName));

	//MultiByteToWideChar(CP_ACP, 0, m_DockName.GetBuffer(260), 260, szDockName, 260);
	MultiByteToWideChar(CP_ACP, 0, strVolumeName.c_str(), strVolumeName.size(), szDockName, 260);

	nLen=wcslen(szDockName) * sizeof(WCHAR);



	bRet=DeviceIoControl(ghEfsDevice,
		EFS_SET_DOCKNAME,
		szDockName,
		nLen,
		NULL,
		0,
		&dwRet,
		NULL);





	WCHAR szSidName[260];


	ghKeyDevice=CreateFile("\\\\.\\SecMKDock",
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	RtlZeroMemory(szSidName, sizeof(szSidName));
	//MultiByteToWideChar(CP_ACP, 0, m_SidName.GetBuffer(260), 260, szSidName, 260);
	MultiByteToWideChar(CP_ACP, 0, strSidName.c_str(), strSidName.size(), szSidName, 260);
	nLen=wcslen(szSidName) * sizeof(WCHAR);




	bRet=DeviceIoControl(ghKeyDevice,
		KEY_SET_SID,
		szSidName,
		nLen,
		NULL,
		0,
		&dwRet,
		NULL);





	HANDLE	hToken;
	HKEY hKey=NULL;
	HKEY hSubKey=NULL;


	if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES,
		&hToken))
	{
		TOKEN_PRIVILEGES tp;
		tp.PrivilegeCount=1;
		LookupPrivilegeValue(NULL, "SeRestorePrivilege", &tp.Privileges[0].Luid);
		tp.Privileges[0].Attributes=SE_PRIVILEGE_ENABLED;
		AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), NULL, NULL);
		bRet=(GetLastError() == ERROR_SUCCESS);
		CloseHandle(hToken);
	}


	//int nSel;
	//CString strLetter;


	//nSel=m_DockLetter.GetCurSel();
	//m_DockLetter.GetLBText(nSel, strLetter);


	//	����ע����Ӽ�
	CHAR szPath[260];
	sprintf(szPath, "%s\\DockBox\\FB1028", strDriveName.c_str());
	MessageBox(NULL, szPath, "", MB_OK);
	RegLoadKey(HKEY_USERS, "FB1028", szPath);


	CreateRegKeyCustom(HKEY_USERS, "FB1028", &hKey);
	RegCreateKey(hKey, "USER", &hSubKey);
	CloseHandle(hKey);
	hKey=hSubKey;
	RegCreateKey(hKey, "current", &hSubKey);
	CloseHandle(hSubKey);

	RegCreateKey(hKey, "current\\software", &hSubKey);
	CloseHandle(hSubKey);
	RegCreateKey(hKey, "classes", &hSubKey);
	CloseHandle(hSubKey);
	CloseHandle(hKey);


	CreateRegKeyCustom(HKEY_USERS, "FB1028", &hKey);
	RegCreateKey(hKey, "machine", &hSubKey);
	CloseHandle(hKey);
	hKey=hSubKey;
	RegCreateKey(hKey, "system", &hSubKey);
	CloseHandle(hSubKey);

	RegCreateKey(hKey, "system\\ControlSet001", &hSubKey);
	CloseHandle(hSubKey);

	CloseHandle(hKey);

	hKey=hSubKey;
	CloseHandle(hKey);
}
void SecFsdModule(std::string strVolumeName)
{
	BOOLEAN bRet=FALSE;
	DWORD dwRet=0;
	WCHAR szDockName[260];
	ULONG nLen;


	ghEfsDevice=CreateFile("\\\\.\\SecMFDock",
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);


	RtlZeroMemory(szDockName, sizeof(szDockName));


	//UpdateData(TRUE);


	//MultiByteToWideChar(CP_ACP, 0, m_DockName.GetBuffer(0), 256, szDockName, 256);
	MultiByteToWideChar(CP_ACP, 0, strVolumeName.c_str(), strVolumeName.size(), szDockName, 260);
	nLen=wcslen(szDockName) * sizeof(WCHAR);



	bRet=DeviceIoControl(ghEfsDevice,
		EFS_SET_DOCKNAME,
		szDockName,
		nLen,
		NULL,
		0,
		&dwRet,
		NULL);


	if (bRet == FALSE)
	{
		MessageBox(NULL, "Error\n", "��ʾ", MB_OK);
		return;
	}
}
void SecRegkeyModule(std::string strDriveName, std::string strSidName)
{
	BOOLEAN bRet=FALSE;
	DWORD dwRet=0;
	WCHAR szSidName[260];
	ULONG nLen;


	ghKeyDevice=CreateFile("\\\\.\\SecMKDock",
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);



	RtlZeroMemory(szSidName, sizeof(szSidName));


	//UpdateData(TRUE);


	//MultiByteToWideChar(CP_ACP, 0, m_SidName.GetBuffer(0), 256, szSidName, 256);
	MultiByteToWideChar(CP_ACP, 0, strSidName.c_str(), strSidName.size(), szSidName, 260);

	nLen=wcslen(szSidName) * sizeof(WCHAR);



	bRet=DeviceIoControl(ghKeyDevice,
		KEY_SET_SID,
		szSidName,
		nLen,
		NULL,
		0,
		&dwRet,
		NULL);

	if (bRet == FALSE)
	{
		//AfxMessageBox("Error\n");
		//return;
	}



	HANDLE	hToken;
	HKEY hKey=NULL;
	HKEY hSubKey=NULL;


	if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES,
		&hToken))
	{
		TOKEN_PRIVILEGES tp;
		tp.PrivilegeCount=1;
		LookupPrivilegeValue(NULL, "SeRestorePrivilege", &tp.Privileges[0].Luid);
		tp.Privileges[0].Attributes=SE_PRIVILEGE_ENABLED;
		AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), NULL, NULL);
		bRet=(GetLastError() == ERROR_SUCCESS);
		CloseHandle(hToken);
	}


	//int nSel;
	//CString strLetter;


	//nSel=m_DockLetter.GetCurSel();
	//m_DockLetter.GetLBText(nSel, strLetter);

	//	����ע����Ӽ�
	CHAR szPath[260];
	//sprintf(szPath, "%s\\DockBox\\FB1028", strLetter);
	sprintf(szPath, "%s\\DockBox\\FB1028", strDriveName.c_str());
	//	AfxMessageBox(szPath);
	RegLoadKey(HKEY_USERS, "FB1028", szPath);


	CreateRegKeyCustom(HKEY_USERS, "FB1028", &hKey);
	RegCreateKey(hKey, "USER", &hSubKey);
	CloseHandle(hKey);
	hKey=hSubKey;
	RegCreateKey(hKey, "current", &hSubKey);
	CloseHandle(hSubKey);

	RegCreateKey(hKey, "current\\software", &hSubKey);
	CloseHandle(hSubKey);
	RegCreateKey(hKey, "classes", &hSubKey);
	CloseHandle(hSubKey);
	CloseHandle(hKey);


	CreateRegKeyCustom(HKEY_USERS, "FB1028", &hKey);
	RegCreateKey(hKey, "machine", &hSubKey);
	CloseHandle(hKey);
	hKey=hSubKey;
	RegCreateKey(hKey, "system", &hSubKey);
	CloseHandle(hSubKey);

	RegCreateKey(hKey, "system\\ControlSet001", &hSubKey);
	CloseHandle(hSubKey);

	CloseHandle(hKey);

	hKey=hSubKey;
	CloseHandle(hKey);
}

void SecDesktopMode()
{
	ULONG nMode=1;
	DWORD dwRet=0;


	DeviceIoControl(ghEfsDevice,
		EFS_SET_MODE,
		&nMode,
		4,
		NULL,
		0,
		&dwRet,
		NULL);


	DeviceIoControl(ghKeyDevice,
		KEY_SET_MODE,
		&nMode,
		4,
		NULL,
		0,
		&dwRet,
		NULL);


	KillProcessFromName("explorer.exe");
	KillProcessFromName("TrustedInstaller.exe");
	KillProcessFromName("msiexec.exe");
}
void SecUseMode(std::string strDriveName, std::string strVolumeName, std::string strSidName)
{
	BOOLEAN bRet=FALSE;
	DWORD dwRet=0;
	WCHAR szDockName[260];
	ULONG nLen;






	ghEfsDevice=CreateFile("\\\\.\\SecMFDock",
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);


	RtlZeroMemory(szDockName, sizeof(szDockName));


	//UpdateData(TRUE);



	//MultiByteToWideChar(CP_ACP, 0, m_DockName.GetBuffer(260), 260, szDockName, 260);
	MultiByteToWideChar(CP_ACP, 0, strVolumeName.c_str(), strVolumeName.size(), szDockName, 260);

	nLen=wcslen(szDockName) * sizeof(WCHAR);



	bRet=DeviceIoControl(ghEfsDevice,
		EFS_SET_DOCKNAME,
		szDockName,
		nLen,
		NULL,
		0,
		&dwRet,
		NULL);





	WCHAR szSidName[260];


	ghKeyDevice=CreateFile("\\\\.\\SecMKDock",
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);









	RtlZeroMemory(szSidName, sizeof(szSidName));
	//MultiByteToWideChar(CP_ACP, 0, m_SidName.GetBuffer(260), 260, szSidName, 260);
	MultiByteToWideChar(CP_ACP, 0, strSidName.c_str(), strSidName.size(), szSidName, 260);
	nLen=wcslen(szSidName) * sizeof(WCHAR);




	bRet=DeviceIoControl(ghKeyDevice,
		KEY_SET_SID,
		szSidName,
		nLen,
		NULL,
		0,
		&dwRet,
		NULL);


	HANDLE	hToken;
	HKEY hKey=NULL;
	HKEY hSubKey=NULL;


	if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES,
		&hToken))
	{
		TOKEN_PRIVILEGES tp;
		tp.PrivilegeCount=1;
		LookupPrivilegeValue(NULL, "SeRestorePrivilege", &tp.Privileges[0].Luid);
		tp.Privileges[0].Attributes=SE_PRIVILEGE_ENABLED;
		AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), NULL, NULL);
		bRet=(GetLastError() == ERROR_SUCCESS);
		CloseHandle(hToken);
	}


	//int nSel;
	//CString strLetter;


	//nSel=m_DockLetter.GetCurSel();
	//m_DockLetter.GetLBText(nSel, strLetter);


	//	����ע����Ӽ�
	char szPath[260];
	//sprintf(szPath, "%s\\DockBox\\FB1028", strLetter);
	sprintf(szPath, _T("%s\\DockBox\\FB1028"), strDriveName.c_str());
	RegLoadKey(HKEY_USERS, _T("FB1028"), szPath);
}
void SecShutdownMode()
{
	ULONG nMode=1;
	DWORD dwRet=0;


	DeviceIoControl(ghEfsDevice,
		EFS_SET_MODE,
		&nMode,
		4,
		NULL,
		0,
		&dwRet,
		NULL);


	DeviceIoControl(ghKeyDevice,
		KEY_SET_MODE,
		&nMode,
		4,
		NULL,
		0,
		&dwRet,
		NULL);


	KillProcessFromName("explorer.exe");
	KillProcessFromName("TrustedInstaller.exe");
	KillProcessFromName("msiexec.exe");


	ExitWindowsEx(EWX_LOGOFF, 0);
	return;
}



void SecFinish()
{
	ULONG nMode=0;
	DWORD dwRet=0;


	Sleep(3000);


	DeviceIoControl(ghKeyDevice,
		KEY_SET_MODE,
		&nMode,
		4,
		NULL,
		0,
		&dwRet,
		NULL);


	HANDLE hToken=NULL;
	::OpenProcessToken(::GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken);
	TOKEN_PRIVILEGES tkp;
	LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);
	tkp.PrivilegeCount=1;
	tkp.Privileges[0].Attributes=SE_PRIVILEGE_ENABLED;
	::AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0);
	int nErroCode=::GetLastError();



	KillProcessFromName("TrustedInstaller.exe");
	KillProcessFromName("msiexec.exe");
	ExitWindowsEx(EWX_REBOOT | EWX_FORCE, 0);
	ExitWindows(EWX_REBOOT, 0);
}


wstring AsciiToUnicode(const string& str) {
	// Ԥ��-�������п��ֽڵĳ���    
	int unicodeLen = MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, nullptr, 0);
	// ��ָ�򻺳�����ָ����������ڴ�    
	wchar_t *pUnicode = (wchar_t*)malloc(sizeof(wchar_t)*unicodeLen);
	// ��ʼ�򻺳���ת���ֽ�    
	MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, pUnicode, unicodeLen);
	wstring ret_str = pUnicode;
	free(pUnicode);
	return ret_str;
}
string UnicodeToAscii(const wstring& wstr) {
	// Ԥ��-�������ж��ֽڵĳ���    
	int ansiiLen = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
	// ��ָ�򻺳�����ָ����������ڴ�    
	char *pAssii = (char*)malloc(sizeof(char)*ansiiLen);
	// ��ʼ�򻺳���ת���ֽ�    
	WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, pAssii, ansiiLen, nullptr, nullptr);
	string ret_str = pAssii;
	free(pAssii);
	return ret_str;
}
wstring Utf8ToUnicode(const string& str) {
	// Ԥ��-�������п��ֽڵĳ���    
	int unicodeLen = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
	// ��ָ�򻺳�����ָ����������ڴ�    
	wchar_t *pUnicode = (wchar_t*)malloc(sizeof(wchar_t)*unicodeLen);
	// ��ʼ�򻺳���ת���ֽ�    
	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, pUnicode, unicodeLen);
	wstring ret_str = pUnicode;
	free(pUnicode);
	return ret_str;
}
string UnicodeToUtf8(const wstring& wstr) {
	// Ԥ��-�������ж��ֽڵĳ���    
	int ansiiLen = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
	// ��ָ�򻺳�����ָ����������ڴ�    
	char *pAssii = (char*)malloc(sizeof(char)*ansiiLen);
	// ��ʼ�򻺳���ת���ֽ�    
	WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, pAssii, ansiiLen, nullptr, nullptr);
	string ret_str = pAssii;
	free(pAssii);
	return ret_str;
}
string AsciiToUtf8(const string& str) {
	return UnicodeToUtf8(AsciiToUnicode(str));
}
string Utf8ToAscii(const string& str) {
	return UnicodeToAscii(Utf8ToUnicode(str));
}

std::string CreateInstalledItemXml(VendorInfo& vendor)
{
	bool addOne = (vendor.arrSoftware.size() % 4) > 0;
	int rows = vendor.arrSoftware.size() / 4;
	if (addOne)
	{
		rows += 1;
	}
	int height = rows * 100 + 50;

	

	char buf[128];
	_stprintf(buf, "%d", height);
	
	std::string tmp(buf);


	std::string xml;
	xml += "<?xml version=\"1.0\" encoding=\"UTF-8\" ?> ";
	xml += "<Window size=\"308, 500\" caption=\"0, 0, 0, 36\" roundcorner=\"4, 4\" >";
	xml += "<Container bkcolor=\"#FFEEEEEE\" inset=\"0, 0, 0, 0\" height=\""+ tmp +"\" >";
	xml += "<VerticalLayout bkcolor=\"#FFEEEEEE\" >";
	xml += "<VerticalLayout height=\"40\"  bkcolor=\"#FFEEEEEE\">";
	xml += "<Control />";
	xml += "<Label text=\"" + vendor.name + "\" font=\"2\" textpadding=\"10, 0, 0, 0\" />";
	xml += "<Control />";
	xml += "</VerticalLayout>";
	xml += "<TileLayout bkcolor=\"#FFEEEEEE\" height=\"" + tmp + "\"  inset=\"5, 5, 5, 5\" childpadding=\"10, 10, 10, 10\" childvpadding=\"10, 10, 10, 10\" name=\"software_list\" itemsize=\"220, 80\" columns=\"4\" vscrollbar=\"true\" hscrollbar=\"false\">";
	for (int i = 0; i < vendor.arrSoftware.size(); i++)
	{
		xml += "<Container bkcolor=\"#FFDDDDDD\" inset=\"2, 2, 2, 2\" height=\"80\" tooltip=\"������\">";
		xml += "<HorizontalLayout height=\"60\" >";
		xml += "<Container bkcolor=\"#FFDDDDDD\"  inset=\"10, 10, 10, 10\" width=\"90\"  >";
		xml += "<Icon name=\"software_icon\" float=\"0.6, 0.5, 0.6, 0.5\" pos=\" -24, -24, 24, 24\"  icon=\"" + vendor.arrSoftware[i].icon + "\" tooltip=\"����ͼ�꣬������\"/>";
		xml += "</Container>";
		xml += "<Control width=\"5\" />";
		xml += "<VerticalLayout  bkcolor=\"#FFDDDDDD\" inset=\"0, 0, 0, 0\">";
		xml += "<Text text=\"" + vendor.arrSoftware[i].name + "\" showhtml=\"true\" font=\"2\" height=\"50\" padding=\"2, 2, 0, 2\" />";
		xml += "<Text text=\"" + vendor.arrSoftware[i].desc + "\" showhtml=\"true\" font=\"3\" height=\"30\" padding=\"2, 2, 0, 2\" />";
		xml += "</VerticalLayout>";
		xml += "</HorizontalLayout>";
		xml += "</Container>";
	}
	xml += "</TileLayout>";
	xml += "</VerticalLayout>";
	xml += "</Container>";
	xml += "</Window>";

	return xml;
}


std::string CreateItemXml(std::string strIcon, std::string strName, std::string strOS, std::string strDesc)
{
	std::string xml;
	xml += "<?xml version=\"1.0\" encoding=\"UTF-8\" ?> ";
	xml += "<Window size=\"308, 228\" caption=\"0, 0, 0, 36\" roundcorner=\"4, 4\" >";
	xml += "<Container bkcolor=\"#FFEEEEEE\" inset=\"0, 0, 0, 0\" height=\"80\" >";
	xml += "<HorizontalLayout height=\"70\" >";
	xml += "<Container bkcolor=\"#FFEEEEEE\"  inset=\"10, 10, 10, 10\" width=\"70\"  >";
	xml += "<Icon name=\"software_icon\" float=\"0.5, 0.5, 0.5, 0.5\" pos=\" -24, -24, 24, 24\"  icon=\"" + strIcon + "\" userdata=\"this_is_can_hover\" />";
	xml += "</Container>";
	xml += "<Control width=\"5\" />";
	xml += "<VerticalLayout  bkcolor=\"#FFEEEEEE\" inset=\"10, 5, 0, 5\">";
	//xml += "<Button name=\"changeskgginbtn\" width=\"22\" height=\"22\" normalimage=\".\\skin\\YouziRes\\close_normal.png\" />";
	xml += "<Text text=\"" + strName + "\" showhtml=\"true\" font=\"2\" height=\"30\" padding=\"10, 2, 0, 2\" />";
	xml += "<Text text=\"" + strOS + "\" showhtml=\"true\" font=\"3\" height=\"10\" padding=\"10, 2, 0, 2\" />";
	xml += "<Text text=\"" + strDesc + "\" showhtml=\"true\" font=\"3\" height=\"10\" padding=\"10, 2, 0, 2\" />";
	xml += "</VerticalLayout>";
	xml += "</HorizontalLayout>";
	xml += "</Container>";
	xml += "</Window>";

	return xml;
}

std::vector<SoftwareInfo> PraseJson(std::string strJsonFile)
{
	std::vector<SoftwareInfo>  arrSoftware;

	FILE * f = fopen(strJsonFile.c_str(), "rb");
	/* ��ȡ�ļ���С */
	fseek(f, 0, SEEK_END);
	long lSize = ftell(f);
	rewind(f);

	/* �����ڴ�洢�����ļ� */
	char * buffer = (char*)malloc(sizeof(char)*(lSize + 1));
	if (buffer == NULL)
	{
		return arrSoftware;
	}
	memset(buffer, 0, lSize + 1);

	
	/* ���ļ�������buffer�� */
	long result = fread(buffer, 1, lSize, f);
	if (result != lSize)
	{
		return arrSoftware;
	}

	fclose(f);

	std::string utf8Str(buffer);

	std::string asciiStr = Utf8ToAscii(utf8Str);
	

	// char * jsonStr = "{\"semantic\":{\"slots\":{\"name\":\"����\"}}, \"rc\":0, \"operation\":\"CALL\", \"service\":\"telephone\", \"text\":\"��绰������\"}";
	const char * jsonStr = asciiStr.c_str();
	cJSON * root = NULL;
	cJSON * item = NULL;//cjson����

	root = cJSON_Parse(jsonStr);
	if (!root)
	{
		printf("Error before: [%s]\n", cJSON_GetErrorPtr());
	}
	else
	{
		
		cJSON * arr = cJSON_GetObjectItem(root, "softwares");//
		int count = cJSON_GetArraySize(arr);
		for (int i = 0; i < count; i++)
		{
			cJSON * s = cJSON_GetArrayItem(arr, i);

			cJSON * nameProp = cJSON_GetObjectItem(s, "name");
			cJSON * iconProp = cJSON_GetObjectItem(s, "icon");
			cJSON * osProp = cJSON_GetObjectItem(s, "os");
			cJSON * descProp = cJSON_GetObjectItem(s, "desc");

			SoftwareInfo software;
			software.name = nameProp->valuestring;
			software.icon = iconProp->valuestring;
			software.os = osProp->valuestring;
			software.desc = descProp->valuestring;

			arrSoftware.push_back(software);

		}
	}

	free(buffer);

	return arrSoftware;
}



