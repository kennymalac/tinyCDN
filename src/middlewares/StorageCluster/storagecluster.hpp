#pragma once

#include <memory>
#include <mutex>
#include <exception>
#include <experimental/filesystem>

#include "../Volume/volume.hpp"
#include "../Volume/marshaller.hpp"
#include "../../hashing.hpp"

#include "../Master/requestTypes.hpp"
#include "../Master/responseTypes.hpp"

#include "requestTypes.hpp"
#include "responseTypes.hpp"
#include "marshaller.hpp"
#include "services.hpp"


namespace fs = std::experimental::filesystem;

namespace TinyCDN::Middleware::StorageCluster {

using namespace TinyCDN::Utility::Hashing;
using namespace TinyCDN::Middleware::Volume;

struct StorageClusterParams {
  UUID4 id;
  std::string name;
  fs::path location;
//  VirtualVolumeParams virtualVolume;
};

/*!
 * \brief A single "core" allocated with available storage.
 * Hosts a virtual volume and receives commands from Master node to store/retrieve files to/from filebuckets, or replicate filebuckets, etc.
Gets a authentication key from the Master node that gives permission to store/retrieve a file.
 */
class StorageClusterNode {
public:
  UUID4 id;
  std::string name;
  fs::path location;
  //! If this StorageClusterNode was opened from a pre-existing config file that was created before running the current program instance
  bool existing;

  std::unique_ptr<VirtualVolume> virtualVolume;

  std::unique_ptr<StorageFileHostingService> getHostingService();
  std::unique_ptr<StorageFileUploadingService> getUploadingService();

  // std::map<SessionId, StorageFileHostingSession>

  // No thanks - RAII ?
  // void configure(StorageClusterParams params);

  StorageClusterNode(UUID4 id, std::string name, fs::path location);

private:
  // TODO: object pool of services
  /*
    This is where we can create the most threads.
    Each service will exist in its own thread.
    The service shares access to the virtual volume's volumemanager and access to the lookup of storagevolumes
  */
  std::unique_ptr<StorageFileUploadingService> uploadingService;
  std::unique_ptr<StorageFileHostingService> hostingService;

  // TODO pool of services... see above
  std::mutex uploadingServiceMutex;
  std::mutex hostingServiceMutex;
  std::mutex virtualVolumeMutex;

  VirtualVolumeJsonMarshaller volumeMarshaller;

  // TODO: networking
  // masterSocket
};

struct StorageClusterNodeSingleton {
  StorageClusterNode* instance;
  static StorageClusterNode* initInstance(StorageClusterNodeSingleton singleton, UUID4 id, std::string name, fs::path location);
};

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
