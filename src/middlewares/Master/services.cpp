#include "middlewares/file.hpp"
#include "services.hpp"
namespace TinyCDN {
using namespace Middleware::File;

// MasterFileUploadingService* MasterFileUploadingServiceSingleton::getInstance() {
//   static MasterFileUploadingService instance();
//   return &instance;
// }

// MasterFileHostingService* FileHostingServiceSingleton::getInstance() {
//   static FileHostingService instance();
//   return &instance;
// }

auto MasterFileUploadingService::requestFileBucket(
  std::unique_ptr<FileBucketRegistry>& registry,
  Size fileSize,
  FileBucketId id,
  std::string contentType,
  std::string fileType,
  std::vector<std::string> tags)
  -> std::future<std::unique_ptr<FileBucket>> {

  std::ios::sync_with_stdio();

    std::shared_lock lock(registry->mutex);
    // Check if there is a ready bucket that supports this file
    auto maybeBucket = std::find_if(registry->currentFileBuckets.begin(), registry->currentFileBuckets.end(), [contentType, fileType, fileSize](const auto& b) {
	return
	  // If this FileBucket has enough free space for this file,
	  (b->storage->getAllocatedSize() - b->size) > fileSize
	  // supports this file's ContentType,
	  && std::find(b->types.begin(), b->types.end(), contentType) != b->types.end();
	// and supports this file's FileType.
	//&& std::find(b->fileTypes.begin(), b->fileTypes.end(), fileType) != b=>fileTypes.end();
      });

    if (maybeBucket == registry->currentFileBuckets.end()) {
      std::cout << "No bucket available for content type: " << contentType << std::endl;
      return {};
    }
    else {
      std::promise<std::unique_ptr<FileBucket>> test;
      test.set_value(std::move(*maybeBucket));
      return test.get_future();
    }
  }
}

}
