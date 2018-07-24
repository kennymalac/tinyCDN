#pragma once

#ifdef __cplusplus
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

#include "middlewares/file.hpp"
#endif

#ifdef __cplusplus
namespace TinyCDN {

//! Currently only holds a pointer to the FileBucketRegistry which contains information about all initiated FileBuckets.
struct CDNMasterSession {
  std::unique_ptr<Middleware::File::FileBucketRegistry> registry;

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
class CDNMaster {
// TODO private
public:

  //! The spawned session store which stores current sessions
  std::unique_ptr<CDNMasterSession> session;

  /*
    spawn CDN from new registry
    spawn CDN from existing registry
    contact name servers
   */

  //! If this CDNMaster was opened from a pre-existing REGISTRY file that was created before running the current program instance
  const bool existing;

  //! Initializes the session's registry, starts the CDNMasterSession process (TODO)
  void spawnCDN();

  CDNMaster(bool existing) : existing(existing) {
    session = std::make_unique<CDNMasterSession>();
  }
};

struct CDNMasterSingleton {
  static CDNMaster* getInstance(bool existing = false);
};

}
#else
typedef struct CDNMaster CDNMaster;
typedef struct CDNMasterSingleton CDNMasterSingleton;
typedef struct StoredFile StoredFile;
typedef struct FileBucket FileBucket;
#endif
