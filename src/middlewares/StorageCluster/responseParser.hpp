#pragma once

#include "responseTypes.hpp"

namespace TinyCDN::Middleware::StorageCluster {

class StorageClusterResponseParser {
public:
  MaybeStorageClusterResponse response;
  //! Parses Packet type and deduces which Command needs to execute
  MaybeStorageClusterResponse parse(std::string contents) {
    return MaybeStorageClusterResponse{};
  }

  StorageClusterResponseParser() = default;
};

}
