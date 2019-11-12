#include "storagecluster.hpp"

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

StorageClusterNode* StorageClusterNodeSingleton::initInstance(StorageClusterNodeSingleton singleton, UUID4 id, std::string name, fs::path location) {
  static StorageClusterNode newInstance(id, name, location);
  singleton.instance = &newInstance;
  return singleton.instance;
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
  std::ifstream configFile(location);

  if (!configFile.is_open() || configFile.bad()) {
    // TODO Notify Master of failure to load
    throw std::logic_error("Config file for StorageCluster could not be loaded!");
  }

  std::string config((std::istreambuf_iterator<char>(configFile)),
		     std::istreambuf_iterator<char>());
  configFile.close();

  // Marshal config into StorageClusterParams
  StorageClusterNodeJsonMarshaller marshaller;

  configFileLocation = location;

  // TODO returns params, or returns cluster node?
  return StorageClusterParams{};
}

void StorageClusterSession::spawn(StorageClusterParams config, bool existing) {
  if (started) {
    throw std::logic_error("StorageClusterSession already started!");
  }

  StorageClusterNodeJsonMarshaller marshaller;

  // Marshal the configuration into a object
  // try {
  //   // singleton.instance = marshaller.getInstance(config);
  //   started = true;

  singleton.initInstance(singleton, UUID4{std::string{""}}, "master-test", fs::current_path());
  // }
  // catch (MarshallerException e) {
  //   std::cerr << e.what();
  // }

  // TODO: Networking - initialize TCP server

  // TODO: Networking - send initial packet to Master

  // TODO: Networking - Wait forever for initialization acknowledgment response packet from Master
  // wait ()

  // TODO: Networking - initialize server

  // TODO: Networking - send Master success or failure, ready to parse
  // TODO: Networking - Wait forever for Master response to Success/Failure packet

  // Until this point is reached, no storage cluster request is processed
  // startEventLoop();

  started = true;
}

}
