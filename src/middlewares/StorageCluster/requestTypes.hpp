#pragma once

#include "../../hashing.hpp"

namespace TinyCDN::Middleware::StorageCluster {

using namespace TinyCDN::Utility::Hashing;

// TODO namespace separate for uploading service requests?
struct StorageClusterMasterFileUploadInitializationRequest {
  // Id<64> bucketId;
  // uintmax_t size
};

using StorageClusterRequest = std::variant<StorageClusterMasterFileUploadInitializationRequest>;
using MaybeStorageClusterRequest = std::variant<std::monostate, StorageClusterMasterFileUploadInitializationRequest>;

}
