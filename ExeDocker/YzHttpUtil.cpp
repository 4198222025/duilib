#include "stdafx.h"
#include "YzHttpUtil.h"

#include "UploadFileInfo.h"

string PrasePackageId(string response)
{
	std::string asciiStr = Utf8ToAscii(response);

	// char * jsonStr = "{\"semantic\":{\"slots\":{\"name\":\"张三\"}}, \"rc\":0, \"operation\":\"CALL\", \"service\":\"telephone\", \"text\":\"打电话给张三\"}";
	const char * jsonStr = asciiStr.c_str();
	cJSON * root = NULL;
	cJSON * item = NULL;//cjson对象

	root = cJSON_Parse(jsonStr);
	if (!root)
	{
		printf("Error before: [%s]\n", cJSON_GetErrorPtr());
		return "";
	}
	else
	{
		cJSON * item = cJSON_GetObjectItem(root, "packageid");//
		if (item)
		{
			return item->valuestring;
		}		
	}

	return "";
}

std::vector<UploadFileInfo> PrasePackageFileInfo(string packagefileinfostr)
{

	vector<UploadFileInfo>  result;

	std::string asciiStr = packagefileinfostr;// Utf8ToAscii(packagefileinfostr);

	// char * jsonStr = "{\"semantic\":{\"slots\":{\"name\":\"张三\"}}, \"rc\":0, \"operation\":\"CALL\", \"service\":\"telephone\", \"text\":\"打电话给张三\"}";
	const char * jsonStr = asciiStr.c_str();
	cJSON * root = NULL;
	cJSON * item = NULL;//cjson对象

	root = cJSON_Parse(jsonStr);
	if (!root)
	{
		printf("Error before: [%s]\n", cJSON_GetErrorPtr());
		return result;
	}
	else
	{
		cJSON * arr = cJSON_GetObjectItem(root, "filelist");//
		int count = cJSON_GetArraySize(arr);
		for (int i = 0; i < count; i++)
		{
			cJSON * s = cJSON_GetArrayItem(arr, i);

			cJSON * nameProp = cJSON_GetObjectItem(s, "filename");
			cJSON * dirProp = cJSON_GetObjectItem(s, "filedir");
			cJSON * noProp = cJSON_GetObjectItem(s, "fileno");

			UploadFileInfo f;
			f.uuid = noProp->valuestring;
			f.dir = dirProp->valuestring;
			f.name = nameProp->valuestring;

			result.push_back(f);

		}
	}

	return result;
}
