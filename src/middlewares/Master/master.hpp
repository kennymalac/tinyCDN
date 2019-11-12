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

/*!
 * \brief Communicates with Clients and facilitates host and upload transactions on the storage clusters.
 */
class MasterNode {
public:
  UUID4 id;

  std::unique_ptr<Middleware::File::FileBucketRegistry> registry;

  //! If this MasterNode was opened from a pre-existing REGISTRY file that was created before running the current program instance
  bool existing;

  //! Tests if the FileBucket in question can store a file of a specific size and various types
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
  static MasterNode* initInstance(MasterNodeSingleton singleton, fs::path location, bool existing = false);
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
  void spawn(MasterParams params, bool existing);
  //! Starts the event loop that waits for Requests. Run this in a different thread.
  void startEventLoop();

  //! Returns a raw pointer to the master node along with a lock that should be unlocked once the master node is no longer needed
  inline std::tuple<std::unique_lock<std::shared_mutex>, MasterNode*> getNode() {
    if (!started) {
      throw new std::logic_error("MasterNode is not initialized because the session has not started.");
    }
    // acquire master unique_lock
    // timeout for mutex?
    std::unique_lock<std::shared_mutex> lock(masterNodeMutex);

    return { std::move(lock), singleton.instance };
  }

  // HTTP frontend: implement the following
  // parseMasterStorageClusterRequest(std::string message); -> MasterRequest
  // parseMasterClientRequest(std::string message); -> MasterRequest

  //! Receives a request coming from the event loop and acts on the request
  MasterResponse receiveRequest(MaybeMasterRequest request);
  // TODO Consider using std::future<StorageClusterResponse>
  //! Sends a request to a StorageCluster
  void sendStorageClusterRequest(UUID4 id, StorageClusterRequest request);

private:
  MasterNodeSingleton singleton;
  bool started = false;
};


}
