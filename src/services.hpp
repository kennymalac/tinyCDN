#pragma once

#ifdef __cplusplus
#include <experimental/filesystem>
#include <memory>
namespace fs = std::experimental::filesystem;

#include "middlewares/file.hpp"

namespace TinyCDN {
using namespace Middleware::File;

struct FileUploadingServiceSingleton {
  static FileUploadingService* getInstance(std::shared_ptr<FileBucketRegistry> registry);
};

struct FileHostingServiceSingleton {
  static FileHostingService* getInstance(std::shared_ptr<FileBucketRegistry> registry);
};
}
#else
typedef struct FileUploadingServiceSingleton FileUploadingServiceSingleton;
typedef struct FileHostingServiceSingleton FileHostingServiceSingleton;
#endif
