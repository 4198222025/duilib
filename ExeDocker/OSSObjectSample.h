#include <alibabacloud/oss/OssClient.h>

class ObjectSample
{
public:
    ObjectSample(const std::string &bucket);
    ~ObjectSample();
 
    void DoesObjectExist();
    void PutFolder();
    void PutObjectFromBuffer();
	void PutObjectFromBuffer(std::string key, std::string content);
    void PutObjectFromFile();
	void PutObjectFromFile(std::string key, std::string filepath);
    void GetObjectToBuffer();
	bool GetObjectToBuffer(std::string key, std::string& content);
    void GetObjectToFile();
	bool GetObjectToFile(std::string key, std::string filepath);
    void DeleteObject();
    void DeleteObjects();
    void HeadObject();
    void GetObjectMeta();
    void AppendObject();
    void MultipartUploadObject();
    void ResumableObject();

    void PutObjectProgress();
    void GetObjectProgress();

    void PutObjectCallable();
    void GetObjectCallable();

    void UploadObjectProgress();
    void MultiCopyObjectProcess();
    void DownloadObjectProcess();

    void CopyObject();
    void RestoreArchiveObject(const std::string bucket, const std::string key, int maxWaitTimeInSeconds);

    void PutObjectCallback();

private:
    void PrintError(const std::string &funcName, const AlibabaCloud::OSS::OssError &error);
    AlibabaCloud::OSS::OssClient *client;
    std::string bucket_;
};
