#include <fstream>
#include <memory>

#include "master.hpp"

void CDNMaster::spawnCDN() {
  // return statusfield

  // initialize a first-time registry
  session->registry = std::make_shared<FileBucketRegistry>(
    fs::current_path(), "REGISTRY");

  if (!this->existing) {
    std::ofstream registryFile("REGISTRY");
    registryFile << "";
  }
};
