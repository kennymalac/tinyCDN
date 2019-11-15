#include "session.hpp"

namespace TinyCDN::Middleware::StorageCluster {

using namespace TinyCDN::Middleware::Master;

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

  return marshaller.deserialize(config);
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
  singleton = marshaller.getSingleton(config);
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
