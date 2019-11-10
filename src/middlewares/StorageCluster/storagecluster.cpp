#include "storagecluster.hpp"
#include "request.hpp"
#include "../Master/master.hpp"

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

StorageClusterNode::StorageClusterNode(UUID4 id, std::string name, fs::path location) :
  id(id), name(name), location(location) {
  // TODO - this has to either create a new virtual volume, or parse a file pertaining to an existing virtual volume, and create that volume
  hostingService = std::make_unique<StorageFileHostingService>();
  uploadingService = std::make_unique<StorageFileUploadingService>();
}

StorageClusterResponse StorageClusterSession::receiveMasterCommand(TinyCDN::Middleware::StorageCluster::StorageClusterRequest request) {
  // TODO parse request
  // TODO handle response

  return StorageClusterResponse{};
}

void StorageClusterSession::sendMasterCommand(MasterRequest request) {
  // Send TCP packet to Master

  // Poll for Master response

  // When Master response is acknowledged, OK

  // TODO: what if Master goes down? timeout?
}

StorageClusterParams StorageClusterSession::loadConfig(fs::path location) {
  // Take file
  std::ifstream configFile(this->configFileLocation);

  if (!configFile.is_open() || configFile.bad()) {
    // TODO Notify Master of failure to load

    throw std::logic_error("Config file for StorageCluster could not be loaded!");
  }

  std::string config((std::istreambuf_iterator<char>(configFile)),
		     std::istreambuf_iterator<char>());
  configFile.close();

  // Marshal config into StorageClusterParams
  StorageClusterNodeJsonMarshaller marshaller;

  // try {
  //   // singleton.instance = marshaller.deserialize(config);
  //   started = true;
  // }
  // catch (MarshallerException e) {
  //   std::cerr << e.what();
  // }

  // TODO returns params, or returns cluster node?
  return StorageClusterParams{};
}

void StorageClusterSession::spawn() {
  if (started) {
    throw std::logic_error("StorageClusterSession already started!");
  }

  // TODO: Networking - initialize TCP server

  // TODO: Networking - Wait forever for initialization packet from Master
  // wait ()

  // TODO: Networking - send acknowledgement packet to Master

  // TODO: Networking - initialize server

  // TODO: Networking - send Master success or failure, ready to parse

  // TODO: Networking - receive final Master acknowledgment packet

  // Until this point is reached, no storage cluster request is allowed
}



}