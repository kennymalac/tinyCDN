#pragma once

#include "requestTypes.hpp"

namespace TinyCDN::Middleware::StorageCluster {

class StorageClusterRequestParser {
public:
  MaybeStorageClusterRequest request;
  //! Parses Packet type and deduces which Command needs to execute
  MaybeStorageClusterRequest parse(std::string contents) {
    return MaybeStorageClusterRequest{};
  }

  StorageClusterRequestParser() = default;
};

std::string getRequestPacket(StorageClusterRequest request) {
  return "";
}

}
