#pragma once

#ifdef __cplusplus
#include <string>
#include <fstream>
#include <memory>
#include <tuple>
#include <experimental/filesystem>
#include <future>
#include <optional>
namespace fs = std::experimental::filesystem;

#include "../file.hpp"

namespace TinyCDN {
using namespace Middleware::File;

struct FileUploadingServiceSingleton {
  static FileUploadingService* getInstance();
};

struct FileHostingServiceSingleton {
  static FileHostingService* getInstance();
};
}

class FileUploadingService {
public:
  // TODO make this return an authentication response from Master
  // Gets a Storage cluster id assigned
  std::future<std::unique_ptr<FileBucket>> requestFileBucket(std::unique_ptr<FileStorage::StoredFile>& tmpFile, FileBucketId id, std::string contentType, std::string fileType, std::vector<std::string> tags);

  // Send over binary data to storage cluster after sending over authentication key for upload
  // Once storage cluster has successfully stored data, return a file id and the filename
  std::future<std::tuple<Storage::fileId, std::string>> uploadFile (std::unique_ptr<FileStorage::StoredFile> tmpFile, FileBucketId id, std::string contentType, std::string fileType, std::vector<std::string> tag);
};

class FileHostingService {
public:
  // Tries to obtain a FileBucket given an id
  std::future<std::optional<std::unique_ptr<FileBucket>>> obtainFileBucket(FileBucketId fbId);

  //! Tries to obtain a StoredFile from a bucket given an id
  // Returns a tuple of the StoredFile and a boolean value that denotes if the file exists
  std::future<std::tuple<std::optional<std::unique_ptr<FileStorage::StoredFile>>, bool>> obtainnStoredFile(std::unique_ptr<FileBucket>& bucket, Storage::fileId cId, std::string fileName);

  //! Safely obtains a stream to the files contents and destroys the StoredFile instance, readds the bucket to the registry, returns the bucket id
  int hostFile(std::ifstream& stream, std::unique_ptr<FileStorage::StoredFile> file, std::unique_ptr<FileBucket> bucket);
};

#else
typedef struct FileUploadingServiceSingleton FileUploadingServiceSingleton;
typedef struct FileHostingServiceSingleton FileHostingServiceSingleton;
#endif
