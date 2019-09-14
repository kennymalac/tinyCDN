#include <fstream>
#include <memory>
#include <experimental/filesystem>


namespace fs = std::experimental::filesystem;
#include "master.hpp"

namespace TinyCDN::Middleware::Master {

bool MasterNode::inspectFileBucket(std::unique_ptr<FileBucket>& fb, Size minimumSize, std::vector<std::string> types) {

  // If this FileBucket has enough free space for this file,
  // TODO master should update filebucket size when upload transaction is approved.
  auto fbAvailableSize = fb->allocatedSize;
  if (fb->size - fbAvailableSize >= minimumSize) {
    // supports the specified ContentTypes,
    auto const notSupportsCtypes = std::any_of(types.cbegin(), types.cend(), [&fb](auto const contentType) {
      return std::find(fb->types.cbegin(), fb->types.cend(), contentType) == fb->types.end();
    });
    //std::cout << notSupportsCtypes << "\n";
    // and supports this file's FileType.
    // && std::find(b.fileTypes.begin(), b.fileTypes.end(), fileType) != b.fileTypes.end()
    if (!notSupportsCtypes) {
      return true;
    }
  }

  return false;
}

void MasterNode::spawnCDN() {
  std::cout << "spawning master, existing: " << this->existing << " " << "\n";
  registry = std::make_unique<Middleware::File::FileBucketRegistry>(
    fs::current_path(), "REGISTRY");

  if (!this->existing || !fs::exists("REGISTRY")) {
    // initialize a first-time registry
    std::ofstream registryFile("REGISTRY");
    registryFile << "";
  }
  else {
    registry->loadRegistry();
  }
};

MasterNode* MasterNodeSingleton::getInstance(bool existing, bool spawn) {
  static MasterNode instance(existing);

  if (spawn && instance.registry == nullptr) {
    instance.spawnCDN();
  }

  return &instance;
}

}
