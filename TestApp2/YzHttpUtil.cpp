#include "stdafx.h"
#include "YzHttpUtil.h"

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
