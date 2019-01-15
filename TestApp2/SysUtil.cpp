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