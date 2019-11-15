#pragma once

#include <memory>
#include <mutex>
#include <exception>
#include <experimental/filesystem>

#include "../Volume/volume.hpp"
#include "../../hashing.hpp"

#include "../Master/requestTypes.hpp"
#include "../Master/responseTypes.hpp"

#include "requestTypes.hpp"
#include "responseTypes.hpp"
#include "services.hpp"


namespace fs = std::experimental::filesystem;

namespace TinyCDN::Middleware::StorageCluster {

using namespace TinyCDN::Utility::Hashing;
using namespace TinyCDN::Middleware::Volume;

/*!
 * \brief A single "core" allocated with available storage.
 * Hosts a virtual volume and receives commands from Master node to store/retrieve files to/from FileBucket instances, or replicate FileBucket instances, etc.
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

  StorageClusterNode(UUID4 id, std::string name, fs::path location, bool existing, std::unique_ptr<VirtualVolume> virtualVolume);

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

  // TODO: networking
  // masterSocket
};

struct StorageClusterNodeSingleton {
  StorageClusterNode* instance;
  static StorageClusterNode* initInstance(StorageClusterNodeSingleton singleton, UUID4 id, std::string name, fs::path location, bool existing, std::unique_ptr<VirtualVolume> virtualVolume);
};

}
