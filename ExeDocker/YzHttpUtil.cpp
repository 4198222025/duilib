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

static string server = "http://39.96.6.55:9999";

// reply of the requery  
static size_t req_reply(void *ptr, size_t size, size_t nmemb, void *stream)
{
	cout << "----->reply" << endl;
	string *str = (string*)stream;
	cout << *str << endl;
	(*str).append((char*)ptr, size*nmemb);
	return size * nmemb;
}

static vector<SoftwareInfo> PraseXXX(string response)
{

	vector<SoftwareInfo>  result;

	std::string asciiStr = Utf8ToAscii(response);

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
		cJSON * totalCountPorp = cJSON_GetObjectItem(root, "count");//
		int totalCount = totalCountPorp->valueint;

		cJSON * arr = cJSON_GetObjectItem(root, "data");//
		int count = cJSON_GetArraySize(arr);
		for (int i = 0; i < count; i++)
		{
			cJSON * s = cJSON_GetArrayItem(arr, i);

			cJSON * idProp = cJSON_GetObjectItem(s, "id");
			cJSON * nameProp = cJSON_GetObjectItem(s, "name");
			cJSON * descProp = cJSON_GetObjectItem(s, "desc1");
			cJSON * versionProp = cJSON_GetObjectItem(s, "version");

			SoftwareInfo f;
			f.id = idProp->valuestring;
			f.name = nameProp->valuestring;
			f.desc = descProp->valuestring;
			

			result.push_back(f);

		}
	}

	return result;
}

std::vector<SoftwareInfo> CallQueryPackageService(string url, string category, int page, int pageCount)
{


	CURL *curl;
	CURLcode res;

	curl = curl_easy_init();
	if (!curl)
	{
		MessageBox(NULL, _T("curl_easy_init 失败！"), _T("提示"), MB_OK);
	}

	string fullurl = server + "/client/product/list";

	curl_easy_setopt(curl, CURLOPT_POST, 1); // post req  
	curl_easy_setopt(curl, CURLOPT_URL, fullurl.c_str());

	string fmt = "{\"keywords\":\"%s\"," \
		"\"first_level\":\"%s\"," \
		"\"page\":%d," \
		"\"limit\":%d}";

	char requestbuf[1024];
	memset(requestbuf, 0, 1024);
	sprintf(requestbuf, fmt.c_str(), "", category.c_str(), page, pageCount);


	string utf8post = AsciiToUtf8(requestbuf);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, utf8post.c_str()); // params  

	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, req_reply);
	string response;
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&response);
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);

	struct curl_slist *head = NULL;
	head = curl_slist_append(head, "Content-Type:application/json;charset=UTF-8");
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, head);

	curl_easy_setopt(curl, CURLOPT_HEADER, 0);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3);


	res = curl_easy_perform(curl);

	string packageid = "";
	if (res != CURLE_OK){
		MessageBox(NULL, curl_easy_strerror(res), _T("提示"), MB_OK);
		fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
	}
	else
	{
		std::vector<SoftwareInfo>  arrSoftware = PraseXXX(response);
		return arrSoftware;		
	}

	std::vector<SoftwareInfo>  arrSoftware;
	return arrSoftware;
}