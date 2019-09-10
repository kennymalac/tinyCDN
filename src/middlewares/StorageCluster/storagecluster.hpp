#pragma once

#include <memory>
#include <mutex>

#include "../Volume/volume.hpp"
#include "request.hpp"
#include "response.hpp"

struct StorageFileHostingService;
struct StorageFileUploadingService;

namespace TinyCDN::Middleware::StorageCluster {
using namespace TinyCDN::Middleware::Volume;

struct MasterRequest;
struct MasterResponse;
class StorageClusterRequest;
class StorageClusterResponse;

/*!
 * \brief A single "core" allocated with available storage.
 * Hosts a virtual volume and receives commands from Master node to store/retrieve files to/from filebuckets, or replicate filebuckets, etc.
Gets a authentication key from the Master node that gives permission to store/retrieve a file.
 */
class StorageClusterNode {
public:
  std::unique_ptr<VirtualVolume> virtualVolume;

  std::unique_ptr<StorageFileHostingService> getHostingService();
  std::unique_ptr<StorageFileUploadingService> getUploadingService();

  // std::map<SessionId, StorageFileHostingSession>

  // HTTP frontend: implement the following
  // parseStorageClusterMasterRequest(std::string message); -> StorageClusterRequest
  // parseStorageClusterClientRequest(std::string message); -> StorageClusterRequest

  // TODO: Use Command pattern
  StorageClusterResponse receiveMasterCommand(StorageClusterRequest request);
  void sendMasterCommand(MasterRequest request);
  // NOTE: The client does not need TCP wrappers, client communication going from HTTP -> CFFI
  // StorageClusterResponse receiveClientCommand(StorageClusterRequest request);
  // StorageClusterResponse sendClientCommand(StorageClusterRequest request);

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
  std::mutex uploadServiceMutex;
  std::mutex hostingServiceMutex;
  std::mutex virtualVolumeMutex;

  // TODO: networking
  // masterSocket
};
}
