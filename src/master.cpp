#include <fstream>
#include <memory>
#include <experimental/filesystem>


namespace fs = std::experimental::filesystem;
#include "master.hpp"

namespace TinyCDN {
void CDNMaster::spawnCDN() {
  // return statusfield

  // initialize a first-time registry
  session->registry = std::make_unique<Middleware::File::FileBucketRegistry>(
        fs::current_path(), "REGISTRY");

  if (!this->existing) {
    std::ofstream registryFile("REGISTRY");
    registryFile << "";
  }
  else {
    session->registry->loadRegistry();
  }
};
}
