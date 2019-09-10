#pragma once

#ifdef __cplusplus
#include <experimental/filesystem>
#include <memory>
namespace fs = std::experimental::filesystem;

#include "../file.hpp"
#endif

#ifdef __cplusplus
namespace TinyCDN::Middleware::Master {

//! Currently only holds a pointer to the FileBucketRegistry which contains information about all initiated FileBuckets.
struct MasterSession {
  std::shared_ptr<Middleware::File::FileBucketRegistry> registry;

  //SessionProvisioner session;

  // uploadSessions;
  // hostingSessions;
};
//extern "C" {
//  struct SessionProvisioner {
//    CDNMasterSession* master;
//    Middleware::File::FileUploadingSession* getUploadingSession() {
//      return new Middleware::File::FileUploadingSession;
//    };


//  };
//}
class MasterNode {
// TODO private
public:

  //! The spawned session store which stores current sessions
  std::unique_ptr<MasterSession> session;

  /*
    spawn CDN from new registry
    spawn CDN from existing registry
    contact name servers
   */

  //! If this MasterNode was opened from a pre-existing REGISTRY file that was created before running the current program instance
  bool existing;

  //! Initializes the session's registry, starts the MasterSession process (TODO)
  void spawnCDN();

  MasterNode(bool existing) : existing(existing) {
    session = std::make_unique<MasterSession>();
  }
};

struct MasterNodeSingleton {
  static MasterNode* getInstance(bool existing = false, bool spawn = false);
};

}
#else
typedef struct MasterNode MasterNode;
typedef struct MasterNodeSingleton MasterNodeSingleton;
typedef struct StoredFile StoredFile;
typedef struct FileBucket FileBucket;
#endif
