#include "storagecluster.hpp"

#include "../Master/master.hpp"
#include "../Volume/marshaller.hpp"

namespace TinyCDN::Middleware::StorageCluster {

using namespace TinyCDN::Middleware::Master;

std::unique_ptr<StorageFileHostingService> StorageClusterNode::getHostingService() {
  hostingServiceMutex.lock();
  return std::move(hostingService);
}

std::unique_ptr<StorageFileUploadingService> StorageClusterNode::getUploadingService() {
  uploadingServiceMutex.lock();
  return std::move(uploadingService);
}

StorageClusterNode::StorageClusterNode(UUID4 id, std::string name, fs::path location, bool existing, std::unique_ptr<VirtualVolume> virtualVolume) :
  id(id), name(name), location(location), existing(existing), virtualVolume(std::move(virtualVolume)) {
  hostingService = std::make_unique<StorageFileHostingService>();
  uploadingService = std::make_unique<StorageFileUploadingService>();
}

StorageClusterNode* StorageClusterNodeSingleton::initInstance(StorageClusterNodeSingleton singleton, UUID4 id, std::string name, fs::path location, bool existing, std::unique_ptr<VirtualVolume> virtualVolume) {
  static StorageClusterNode newInstance(id, name, location, existing, std::move(virtualVolume));
  singleton.instance = &newInstance;
  return singleton.instance;
}

}
