#pragma once

#include "../Volume/marshaller.hpp"
#include "marshaller.hpp"
#include "storagecluster.hpp"

namespace fs = std::experimental::filesystem;

namespace TinyCDN::Middleware::StorageCluster {

using namespace TinyCDN::Utility::Hashing;
using namespace TinyCDN::Middleware::Volume;

class StorageClusterSession {
public:
  // Only shared because we would like to copy StorageClusterSession. All locks will be unique_locks
  std::shared_mutex storageClusterNodeMutex;
  fs::path configFileLocation;

  StorageClusterParams loadConfig(fs::path location);
  void spawn(StorageClusterParams config, bool existing);

  //! Returns a raw pointer to the storageCluster node along with a lock that should be unlocked once the storageCluster node is no longer needed
  inline std::tuple<std::unique_lock<std::shared_mutex>, StorageClusterNode*> getNode() {
    if (!started) {
      throw new std::logic_error("StorageClusterNode is not initialized because the session has not started.");
    }
    // acquire storageCluster unique_lock
    // timeout for mutex?
    std::unique_lock<std::shared_mutex> lock(storageClusterNodeMutex);

    return { std::move(lock), singleton.instance };
  }

  // HTTP frontend: implement the following
  // parseStorageClusterMasterRequest(std::string message); -> StorageClusterRequest
  // parseStorageClusterClientRequest(std::string message); -> StorageClusterRequest

  // TODO: Use Command pattern, use variant for different types of commands rather than virtual class
  StorageClusterResponse receiveMasterCommand(StorageClusterRequest request);
  //! Sends a formatted TCP packet to the master node. TODO: Should this be a std::future<AcknowledgementResponse>. Should acknowledgement response have an ID? OR stateless? Probably stateless
  void sendMasterCommand(Master::MasterRequest request);

  // NOTE: The client does not need TCP wrappers, client communication going from HTTP -> CFFI
  // StorageClusterResponse receiveClientCommand(StorageClusterRequest request);
  // StorageClusterResponse sendClientCommand(StorageClusterRequest request);


private:
  bool started;
  StorageClusterNodeSingleton singleton;
};

}
