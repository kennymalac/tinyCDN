#pragma once

#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

#include "middlewares/file.hpp"


struct CDNMasterSession {
  std::shared_ptr<FileBucketRegistry> registry;

  // uploadSessions;
  // hostingSessions;
};

class CDNMaster {
  std::unique_ptr<CDNMasterSession> session;

  /*
    spawn CDN from new registry
    spawn CDN from existing registry
    contact name servers
   */

public:
  const bool existing;

  void spawnCDN();

  CDNMaster(bool existing) : existing(existing) {
    session = std::make_unique<CDNMasterSession>();
  };
};
