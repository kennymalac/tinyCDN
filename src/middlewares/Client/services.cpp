#include <future>
#include <tuple>

#include "middlewares/file.hpp"
#include "services.hpp"
namespace TinyCDN {
using namespace Middleware::File;

FileUploadingService* FileUploadingServiceSingleton::getInstance() {
  static FileUploadingService instance();
  return &instance;
}

FileHostingService* FileHostingServiceSingleton::getInstance() {
  static FileHostingService instance();
  return &instance;
}

std::future<std::tuple<std::optional<std::unique_ptr<FileBucket>>, std::optional<std::shared_ptr<FileBucketRegistryItem>>>> FileHostingService::obtainFileBucket(FileBucketId fbId) {
  // TODO const
  // TODO actually make this async
  std::promise<std::tuple<std::optional<std::unique_ptr<FileBucket>>, std::optional<std::shared_ptr<FileBucketRegistryItem>>>> test;
  std::optional<std::unique_ptr<FileBucket>> bucket;

  // TODO C++20: use atomic_shared_ptr
  // TODO std::scoped_lock registryLock();

  auto item = registry->getItem(fbId);
  if (item.has_value()) {
    bucket = std::move(item.value()->fileBucket.value());
  }

  test.set_value(std::make_tuple(std::move(bucket), item));
  return test.get_future();
}

std::future<std::tuple<std::optional<std::unique_ptr<FileStorage::StoredFile>>, bool>> FileHostingService::obtainStoredFile(std::unique_ptr<FileBucket>& bucket, Storage::fileId cId, std::string fileName) {
  // TODO actually make this async
  std::promise<std::tuple<std::optional<std::unique_ptr<FileStorage::StoredFile>>, bool>> test;

  std::optional<std::unique_ptr<FileStorage::StoredFile>> maybeFile;
  auto exists = false;

  // TODO ask master to contact storage cluster
  auto storedFile = bucket->storage->lookup(cId);

  exists = true;

  if (storedFile->location.filename() == fileName) {
    maybeFile = std::move(storedFile);
  }

  test.set_value(std::make_tuple(std::move(maybeFile), exists));
  return test.get_future();
}

int FileHostingService::hostFile(std::ifstream& stream, std::unique_ptr<FileStorage::StoredFile> file, std::unique_ptr<FileBucket> bucket, std::shared_ptr<FileBucketRegistryItem>& item) {
  // TODO safely obtain a lock to the file from the bucket?
  stream = file->getStream<std::ifstream>();
  auto const fbId = bucket->id;

  item->fileBucket = std::move(bucket);

  return fbId;
}


}
