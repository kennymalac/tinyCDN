#pragma once

#include <memory>
#include <string>

// #include "storagecluster.hpp"

namespace TinyCDN::Middleware::StorageCluster {

struct StorageClusterNode;
struct StorageClusterNodeJsonMarshaller {
  std::unique_ptr<StorageClusterNode> deserialize(std::string input);
};
}
