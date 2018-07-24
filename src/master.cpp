#include <fstream>
#include <memory>
#include <experimental/filesystem>


namespace fs = std::experimental::filesystem;
#include "master.hpp"

namespace TinyCDN {
void CDNMaster::spawnCDN() {
  // return statusfield
  // test
  std::cout << "spawning master, existing: " << this->existing << " " << "\n";
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

CDNMaster* CDNMasterSingleton::getInstance(bool existing) {
  static CDNMaster instance(existing);
  return &instance;
}

}
