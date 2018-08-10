#include "middlewares/file.hpp"
#include "services.hpp"
namespace TinyCDN {
using namespace Middleware::File;

FileUploadingService* FileUploadingServiceSingleton::getInstance(std::shared_ptr<Middleware::File::FileBucketRegistry> registry) {
  static FileUploadingService instance(registry);
  return &instance;
}

FileHostingService* FileHostingServiceSingleton::getInstance(std::shared_ptr<FileBucketRegistry> registry) {
  static FileHostingService instance(registry);
  return &instance;
}

}
