#pragma once

#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

#include "middlewares/file.hpp"

namespace TinyCDN {

//! Currently only holds a pointer to the FileBucketRegistry which contains information about all initiated FileBuckets.
//! I'm contemplating making this a unique_ptr.
struct CDNMasterSession {
  std::shared_ptr<Middleware::File::FileBucketRegistry> registry;

  // uploadSessions;
  // hostingSessions;
};

class CDNMaster {
  //! The spawned session store which stores current sessions
  std::unique_ptr<CDNMasterSession> session;

  /*
    spawn CDN from new registry
    spawn CDN from existing registry
    contact name servers
   */

public:
  //! If this CDNMaster was opened from a pre-existing REGISTRY file that was created before running the current program instance
  const bool existing;

  //! Initializes the session's registry, starts the CDNMasterSession process (TODO)
  void spawnCDN();

  CDNMaster(bool existing) : existing(existing) {
    session = std::make_unique<CDNMasterSession>();
  }
};
}
