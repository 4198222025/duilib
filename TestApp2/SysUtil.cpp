#include "stdafx.h"
#include <vector>
#include <string>

#include "SysUtil.h"

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
	pDrive = drives; //ָ���һ���߼�������
	//���������ַ������б����
	while (*pDrive)
	{
		//�����������Ƽ����б���
		strArray.push_back(pDrive);

		//ָ����һ����������ʶ��
		pDrive += strlen(pDrive) + 1;
	}

	for (int i = 0; i < strArray.size(); ++i)
	{
		printf("%ls\r\n", strArray[i].c_str());
	}

	return strArray;
}

std::string GetVolumeName(std::string driveName)
{
	CHAR szVolumeName[128] = { '\0' };

	RtlZeroMemory(szVolumeName, 128);
	QueryDosDevice(driveName.c_str(), szVolumeName, 128);

	std::string strVolumeName(szVolumeName);
	return strVolumeName;
}
std::string GetUserId()
{
	char userName[260] = "";
	char sid[260] = "";
	DWORD nameSize = sizeof(userName);
	GetUserName(userName, &nameSize);


	char userSID[260] = "";
	char userDomain[260] = "";
	DWORD sidSize = sizeof(userSID);
	DWORD domainSize = sizeof(userDomain);


	SID_NAME_USE snu;
	LookupAccountName(NULL,
		userName,
		(PSID)userSID,
		&sidSize,
		userDomain,
		&domainSize,
		&snu);


	PSID_IDENTIFIER_AUTHORITY psia = GetSidIdentifierAuthority(userSID);
	sidSize = sprintf(sid, "S-%lu-", SID_REVISION);
	sidSize += sprintf(sid + strlen(sid), "%-lu", psia->Value[5]);


	int subAuthorities = *GetSidSubAuthorityCount(userSID);


	for (int i = 0; i < subAuthorities; i++)
	{
		sidSize += sprintf(sid + sidSize, "-%lu", *GetSidSubAuthority(userSID, i));
	}

	sprintf(userSID, "\\REGISTRY\\USER\\%s", sid);

	std::string strUserId(userSID);
	return strUserId;
}