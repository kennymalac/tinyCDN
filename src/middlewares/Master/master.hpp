#pragma once

#include <experimental/filesystem>
#include <memory>
namespace fs = std::experimental::filesystem;

#include "../file.hpp"
// #include ""

namespace TinyCDN::Middleware::Master {

using namespace TinyCDN::Utility::Hashing;
using namespace TinyCDN::Middleware::File;

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

struct StorageClusterRequest;

class MasterRequest {
  virtual void parse(std::string contents) = 0;
};

class MasterResponse {
  virtual void parse(std::string contents) = 0;
};

class Success {};
class Error {
  std::string reason;
};

class MasterRequestInitResponsePacket : MasterRequest {
  // static std::string name = "INIT_RESPONSE";
  std::variant<Success, Error> callerStatus;
};

class MasterNode {
public:
  UUID4 id;

  std::unique_ptr<Middleware::File::FileBucketRegistry> registry;

  // HTTP frontend: implement the following
  // parseMasterStorageClusterRequest(std::string message); -> MasterRequest
  // parseMasterClientRequest(std::string message); -> MasterRequest

  MasterResponse receiveMasterCommand(MasterRequest request);
  // Consider using std::future<StorageClusterResponse>
  void sendStorageClusterCommand(StorageClusterRequest request);

  void spawnCDN();

  /*
    spawn CDN from new registry
    spawn CDN from existing registry
    contact name servers
   */

  //! If this MasterNode was opened from a pre-existing REGISTRY file that was created before running the current program instance
  bool existing;

  bool inspectFileBucket(std::unique_ptr<FileBucket>& fb, Size minimumSize, std::vector<std::string> types);

  MasterNode(bool existing) : existing(existing) {}

private:
  std::vector<MasterStorageClusterNodeConfig> storageClusterNodeConfigs;

  // TODO: object pool of services

  /*
    This is where we can create the most threads.
    Each service will exist in its own thread.
    Facilitates communication from client to send user to storage node
  */
  // std::unique_ptr<MasterFileUploadingService> uploadingService;
  // TODO look into how to redirect from Master -> Storage node on a single HTTP request
  // std::unique_ptr<MasterFileHostingService> hostingService;

  // TODO pool of services... see above
  std::mutex uploadServiceMutex;
  std::mutex hostingServiceMutex;
};

struct MasterNodeSingleton {
  static MasterNode* getInstance(bool existing = false, bool spawn = false);
};

/*!
 *\brief Copyable object that allows protected access to the program's single MasterNode instance
*/
class MasterSession {
public:
  // Only shared because we would like to copy MasterSession. All locks will be unique_locks
  std::shared_mutex masterNodeMutex;

 //! Initializes the FileBucketRegistry and connects to Storage cluster (TODO)
  void spawn(MasterParams params);

  MasterParams loadConfig(fs::path location);

  //! Returns a raw pointer to the master node along with a lock that should be unlocked once the master node is no longer needed
  inline std::tuple<std::unique_lock<std::shared_mutex>, MasterNode*> getMasterNode() {
    // acquire master unique_lock
    // timeout for mutex?

    //    masterSession.masterNodeMutex;
    std::unique_lock<std::shared_mutex> lock(masterNodeMutex);

    return { std::move(lock), singleton.getInstance() };
  }

private:
  MasterNodeSingleton singleton;
};


}
