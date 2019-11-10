#pragma once

#include <experimental/filesystem>
#include <memory>
namespace fs = std::experimental::filesystem;

#include "../file.hpp"
#include "requestTypes.hpp"
#include "responseTypes.hpp"
#include "../StorageCluster/requestTypes.hpp"
// #include ""

namespace TinyCDN::Middleware::Master {

using namespace TinyCDN::Utility::Hashing;
using namespace TinyCDN::Middleware::File;
using namespace TinyCDN::Middleware::StorageCluster;

using VolumeId = Id<64>;

struct MasterStorageClusterNodeConfig {
  VolumeId id;
  VolumeId virtualVolumeId;
  std::string hostname;
  int port;
};

struct MasterParams {
  UUID4 id;
  // clusternodes
  // clientnodes
};

class MasterFileHostingService;
class MasterFileUploadingService;

class MasterNode {
public:
  UUID4 id;

  std::unique_ptr<Middleware::File::FileBucketRegistry> registry;

  //! If this MasterNode was opened from a pre-existing REGISTRY file that was created before running the current program instance
  bool existing;

  bool inspectFileBucket(std::unique_ptr<FileBucket>& fb, Size minimumSize, std::vector<std::string> types);
  std::unique_ptr<MasterFileHostingService> getHostingService();
  std::unique_ptr<MasterFileUploadingService> getUploadingService();

  MasterNode(fs::path location, bool existing);

private:
  std::vector<MasterStorageClusterNodeConfig> storageClusterNodeConfigs;

  // TODO: object pool of services

  /*
    This is where we can create the most threads.
    Each service will exist in its own thread.
    Facilitates communication from client to send user to storage node
  */
  std::unique_ptr<MasterFileUploadingService> uploadingService;
  // TODO look into how to redirect from Master -> Storage node on a single HTTP request
  std::unique_ptr<MasterFileHostingService> hostingService;

  // TODO pool of services... see above
  std::mutex uploadingServiceMutex;
  std::mutex hostingServiceMutex;
};

struct MasterNodeSingleton {
  MasterNode* instance;
  MasterNode* initInstance(fs::path location, bool existing = false);
};

/*!
 *\brief Copyable object that allows protected access to the program's single MasterNode instance and sends & receives requests across to and from other nodes
*/
class MasterSession {
public:
  // Only shared because we would like to copy MasterSession. All locks will be unique_locks
  std::shared_mutex masterNodeMutex;

  //! Parses a JSON file location into a configuration that can be spawned
  MasterParams loadConfig(fs::path location);
  //! Runs the TCP server, Initializes the FileBucketRegistry, connects to Storage cluster (TODO), and runs the event loop
  void spawn(MasterParams params);
  //! Starts the event loop that waits for Requests. Run this in a different thread.
  void startEventLoop();

  //! Returns a raw pointer to the master node along with a lock that should be unlocked once the master node is no longer needed
  inline std::tuple<std::unique_lock<std::shared_mutex>, MasterNode*> getMasterNode() {
    // acquire master unique_lock
    // timeout for mutex?

    //    masterSession.masterNodeMutex;
    std::unique_lock<std::shared_mutex> lock(masterNodeMutex);

    return { std::move(lock), singleton.instance };
  }

  // HTTP frontend: implement the following
  // parseMasterStorageClusterRequest(std::string message); -> MasterRequest
  // parseMasterClientRequest(std::string message); -> MasterRequest

  MasterResponse receiveRequest(MaybeMasterRequest request);
  // Consider using std::future<StorageClusterResponse>
  void sendStorageClusterRequest(UUID4 id, StorageClusterRequest request);

private:
  MasterNodeSingleton singleton;
  bool started = false;
};


}
